#pragma once
#include "Registers.h"
#include <memory>

struct GetResistorAndStepForCalibrationTest;
struct CurrentSourceTest;

namespace CLAMP {
    class Channel;
    class Board;
    class Chip;
    class SimplifiedWaveform;

    /** \brief High-level functions for configuring CLAMP chips and channels.
     *
     *  The ClampController class contains a series of members that mimics the structures on
     *  the chip.  Member functions within each of these provide ways to:
     *       - set configuration parameters
     *       - calibrate the unit
     *       - etc.
     *
     *  The configuration functions come in two flavors.  
     *    - Add the appropriate command to the given channel's command list in memory.  
     *      This is useful when building a more complex command sequence.  
     *    - Update the settings for a list of channels immediately,
     *      i.e., it sends the command sequence to the FPGA then executes it.  These contain the
     *      word Immediate in the function name.
     *
     *  Click on a section of the image below to see more detailed documentation.
     *
     \htmlonly
         <img id="Clamp4_overview" src="Clamp_4_diagram.jpg" border="0" width="738" height="769" orgwidth="738" orgheight="769" usemap="#config-map" alt="" style="height: 770px; width: 738px;">
         <map name="config-map" id="config-map">
         <area alt="" title="Off chip components" href="class_clamp4_1_1_clamp_config_1_1_off_chip_components.html" shape="rect" coords="4,4,305,213" style="outline:none;" target="_self">
         <area alt="" title="Clamp Current Generator" href="class_clamp4_1_1_clamp_config_1_1_clamp_current_generator.html" shape="rect" coords="430,6,734,175" style="outline:none;" target="_self">
         <area alt="" title="Voltage Amplifier" href="class_clamp4_1_1_clamp_config_1_1_voltage_amplifier.html" shape="rect" coords="556,201,733,360" style="outline:none;" target="_self">
         <area alt="" title="Current-to-voltage converter" href="class_clamp4_1_1_clamp_config_1_1_current_to_voltage_converter.html" shape="rect" coords="260,287,471,595" style="outline:none;" target="_self">
         <area alt="" title="Fast transient capacitive compensation" href="class_clamp4_1_1_clamp_config_1_1_fast_transient_capacitive_compensation.html" shape="rect" coords="3,379,223,638" style="outline:none;" target="_self">
         <area alt="" title="Difference Amplifier" href="class_clamp4_1_1_clamp_config_1_1_difference_amplifier.html" shape="rect" coords="476,387,734,594" style="outline:none;" target="_self">
         <area alt="" title="Clamp voltage generator" href="class_clamp4_1_1_clamp_config_1_1_clamp_voltage_generator.html" shape="rect" coords="280,617,734,751" style="outline:none;" target="_self">
         </map>
     \endhtmlonly
     */
    namespace ClampConfig {
        /** \brief Index of a chip/channel combination.
         *
         *  A CLAMP Evaluation Board can have up to 4 CLAMP chips, each of which has 4 channels.
         *  The ClampController and its member components provide members to change the settings
         *  of channels, and functions typically either operate on one chip/channel or on a list
         *  of chip/channels.
         *
         *  This struct handles the single channel case.  ChipChannelList is a list of chip/channel
         *  pairs.
         *
         *  For example, if you wanted to create a command that operated on %Chip 2, %Channel 0, you'd
         *  do something like this:
         *  \code
             ChipChannel index(2, 0);
             clampController.<something>.<method>(index, <other params>);
         *  \endcode
        */
        struct ChipChannel {
            /// %Chip [0-3]
            unsigned int chip;
            /// %Channel [0-3]
            unsigned int channel;

            /** \brief Constructor
             *  \param[in] chip_  %Chip [0-3]
             *  \param[in] channel_  %Channel [0-3]
             */
            ChipChannel(unsigned int chip_ = 0, unsigned int channel_ = 0) : chip(chip_), channel(channel_) {}
        };
        /// \cond private
        inline bool operator<(const ChipChannel& _Left, const ChipChannel& _Right) {	// test if _Left < _Right
            return (_Left.chip < _Right.chip ||
                (!(_Right.chip < _Left.chip) && _Left.channel < _Right.channel));
        }
        /// \endcond

        /** \brief A list of chip/channel pairs.
         *
         *  See ChipChannel for some background.
         *
         *  For example, if you wanted to create a command that operated on %Channel 0 of all chips, you'd
         *  do something like this:
         *  \code
             ChipChannelList list = { ChipChannel(0, 0), ChipChannel(1, 0), ChipChannel(2, 0), ChipChannel(3, 0) };
             clampController.<something>.<method>(list, <other params>);
         *  \endcode
         */
        typedef std::vector<ChipChannel> ChipChannelList;

        class ClampController;

        /** \brief Base class for many of the classes in this namespace
         *
         *  Provides common functionality that is used throughout; no public interface.
         */
        class ChipComponent {
        protected:
            ChipComponent(ClampController& c);

            /** \brief Access to the ClampController for subclasses
             *
             * Subclasses use this reference to it to access other subclasses, the Board, Channel and Chip instances, etc.
             */
            ClampController& controller;

            std::vector<WaveformControl::WaveformCommand>& commands(const ChipChannel& chipChannel) const;
            Registers::ChannelRegisters& registers(const ChipChannel& chipChannel) const;
            Registers::GlobalRegisters& chipRegisters(const ChipChannel& chipChannel) const;
        };

        /** \brief Configuration of difference amplifier.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The difference amplifier amplifies the current output of voltage clamp, prior to it
         *  being sampled by the Mux.
         *
         * <img id="DifferenceAmplifier" src="DifferenceAmplifier.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class DifferenceAmplifier : private ChipComponent {
        public:
            DifferenceAmplifier(ClampController& c);

            void setInPlus(const ChipChannel& chipChannel, bool ground);
            void setInPlusImmediate(const ChipChannelList& channelList, bool ground);

            void setInMinus(const ChipChannel& chipChannel, bool ground);
            void setInMinusImmediate(const ChipChannelList& channelList, bool ground);

            void calibrate1(const ChipChannelList& channelList);
            void calibrate2(const ChipChannelList& channelList);

        private:
            void setOffsetTrim(const ChipChannel& chipChannel, uint8_t value);
            void createOffsetTrimCommands(const ChipChannel& chipChannel);
            void getBestOffsetTrimAndResidual(const ChipChannel& chipChannel);
        };

        /** \brief Configuration of current-to-voltage converter.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The current-to-voltage converter passes an input current (coming from the electrode or other off-chip components)
         *  through a feedback resistance, thus converting the current to a voltage.  That voltage is measured via the
         *  DifferenceAmplifier.
         *
         * <img id="CurrentToVoltageConverter" src="CurrentToVoltageConverter.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class CurrentToVoltageConverter : private ChipComponent {
        public:
            CurrentToVoltageConverter(ClampController& c);

            void setVoltageClampConnect(const ChipChannel& chipChannel, bool connect);
            void setVoltageClampConnectImmediate(const ChipChannelList& channelList, bool connect);

			void switchToVoltageClampMode(const ChipChannel& chipChannel, bool connect);
			void switchToCurrentClampMode(const ChipChannel& chipChannel, bool connect);

            Registers::Register3::Resistance getFeedbackResistance(const ChipChannel& chipChannel);
            void setFeedbackResistance(const ChipChannel& chipChannel, Registers::Register3::Resistance value);
            void setFeedbackResistanceImmediate(const ChipChannelList& channelList, Registers::Register3::Resistance value);
            void setFeedbackResistanceAndCapacitance(const ChipChannel& chipChannel, Registers::Register3::Resistance value);
            void setFeedbackResistanceAndCapacitanceImmediate(const ChipChannelList& channelList, Registers::Register3::Resistance value);

            void setFeedbackCapacitance(const ChipChannel& chipChannel, double value);
            void setFeedbackCapacitanceImmediate(const ChipChannelList& channelList, double value);

            void calibrate(const ChipChannelList& channelList);

        private:
            Registers::Register8::InputSelect mostRecentCalibrationResistor;

            void calibrateOneR(const ChipChannelList& channelList, Registers::Register3::Resistance r);
            void createCommandsToCalibrateCurrentToVoltageConverter(const ChipChannel& chipChannel);
            void calculateMeasuredRValue(const ChipChannel& chipChannel, Registers::Register3::Resistance r);
        };

        /** \brief Configuration of clamp voltage generator.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The clamp voltage generator creates the voltage used in voltage clamp mode.
         *
         * <img id="ClampVoltageGenerator" src="ClampVoltageGenerator.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class ClampVoltageGenerator : private ChipComponent {
        public:
            ClampVoltageGenerator(ClampController& c);

            void setClampVoltage(const ChipChannel& chipChannel, int16_t value);
            void setClampVoltageImmediate(const ChipChannelList& channelList, int16_t value);

            void setClampStepSize(const ChipChannel& chipChannel, bool is5mV);
            void setClampStepSizeImmediate(const ChipChannelList& channelList, bool is5mV);

            void calibrate(const ChipChannelList& channelList);

            double getAppliedVoltage(bool is5mV, bool isPlus, uint8_t magnitude);

        private:
            void createCommandsToCalibrateClampVoltage(const ChipChannel& chipChannel);
            void getBestOffsetTrim(const ChipChannel& chipChannel);
            void setClampVoltageOffsetTrim(const ChipChannel& chipChannel, uint8_t value);
        };

        /** \brief Configuration of fast transient capacitive compensation.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The CLAMP chip contains circuitry for cancelling/compensating for fast capacitive transients when changing applied
         *  voltage or applied current.
         *
         * <img id="CapacitiveCompensation" src="CapacitiveCompensation.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class FastTransientCapacitiveCompensation : private ChipComponent {
        public:
            FastTransientCapacitiveCompensation(ClampController& c);

            bool getConnect(const ChipChannel& chipChannel);
            void setConnect(const ChipChannel& chipChannel, bool connect);
            void setConnectImmediate(const ChipChannelList& channelList, bool connect);

            void setInSelect(const ChipChannel& chipChannel, bool currentClamp);
            void setInSelectImmediate(const ChipChannelList& channelList, bool currentClamp);

            void setMagnitude(const ChipChannel& chipChannel, double magnitude);
            double getMagnitude(const ChipChannel& chipChannel);
            void setMagnitudeImmediate(const ChipChannelList& channelList, double value);

            void buzz(const ChipChannel& chipChannel, bool largeAmplitude, double seconds);
            void buzzImmediate(const ChipChannelList& channelList, bool largeAmplitude, double seconds);
        };

        /** \brief Configuration of voltage amplifier.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The voltage amplifier is used to measure voltages in current clamp mode.
         *
         * <img id="VoltageAmplifier" src="VoltageAmplifier.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class VoltageAmplifier : private ChipComponent {
        public:
            VoltageAmplifier(ClampController& c);

            void setPower(const ChipChannel& chipChannel, bool on);
            void setPowerImmediate(const ChipChannelList& channelList, bool on);

            void calibrate(const ChipChannelList& channelList);

        private:
            void createCommandsToCalibrateVoltageAmplifier(const ChipChannel& chipChannel);
            void getBestResidual(const ChipChannel& chipChannel);
        };

        /** \brief A pair of (coarse, fine) calibration values for calibrating the current scale.
         *
         *  See ClampCurrentGenerator for more information.
         */
        struct CurrentCalibration {
            /// Maximum coarse value
            static const uint8_t MAX_COARSE = 127;
            /// Maximum fine value
            static const uint8_t MAX_FINE = 255;

            /// Coarse calibration value [0-127]
            uint8_t coarse;
            /// Fine calibration value [0-255]
            uint8_t fine;

            /// Constructor
            CurrentCalibration() {}
            /** \brief Constructor
             *  \param[in] coarse_ Coarse calibration value [0-127]
             *  \param[in] fine_   Fine calibration value [0-255]
             */
            CurrentCalibration(uint8_t coarse_, uint8_t fine_) : coarse(coarse_), fine(fine_) {}
        };

        /// Current scales for common scenarios
        enum CurrentScale {
            I_5pA,   ///< 5 pA step size
            I_50pA,  ///< 50 pA step size
            I_500pA, ///< 500 pA step size
            I_1nA    ///< 1000 pA (1 nA) step size
        };
        uint8_t getNominalCoarse(CurrentScale scale);
        double getStepSize(CurrentScale scale);

        class ClampCurrentGenerator;
        /// \cond private
        // Class used internally in calibrating the CurrentClampGenerator
        class ClampCurrentCalibrationHelper {
        public:
            ClampCurrentCalibrationHelper(ClampController& controller_) : controller(controller_) {}

            CurrentCalibration left, right, mid, saved;
            double stepSize;
            bool positiveCurrent;
            double leftCurrent;
            double rightCurrent;
            double target;

            void init(const ChipChannel& chipChannel, CurrentScale scale, bool positiveCurrent_);

            // Bracketing commands
            void bracketInit(const ChipChannel& chipChannel);
            void measureBracketBounds(const ChipChannel& chipChannel);
            bool bracketOneStep(const ChipChannel& chipChannel);

            // Binary search commands
            bool binaryStepInit(const ChipChannel& chipChannel);
            bool binaryStepReInit();
            void binaryOneStep(const ChipChannel& chipChannel);

            CurrentCalibration getBest() const;

        private:
            ClampController& controller;
        };
        /// \endcond

        /** \brief Configuration of clamp current generator.
         *
         *  The ClampController class contains a series of members that mimics this and other structures on the chip.
         *
         *  The clamp current generator generates currents in current clamp mode.  It has a very large dynamic range - it
         *  can generate currents down to the pA level and currents in the hundreds of nA level.  To achieve that,
         *  *current source scale* registers are set; these registers configure the step size of the current source -
         *  magnitudes of 5 pA, 50 pA, 500 pA or 1 nA are used for the scale.
         *
         *  As part of the calibration routine, optimal values of the *current source scale* parameters for the four
         *  scales mentioned above are calculated.  To control the current, you set the current scale to one of
         *  the four (do this once), then create a current waveform using the *clamp current sign* and 
         *  *clamp current magnitude* values.
         *
         * <img id="ClampCurrentGenerator" src="ClampCurrentGenerator.jpg" border="0">
         *
         * By convention, positive current is the flow of positive charges out of the chip, negative current is the reverse current direction.
         *
         *  See the "CLAMP Simplified Diagram" and "Self-Calibration Routines" section of the datasheet
         *  for more information.
         */
        class ClampCurrentGenerator : private ChipComponent {
        public:
            ClampCurrentGenerator(ClampController& c);

            void setEnable(const ChipChannel& chipChannel, bool connect);
            void setEnableImmediate(const ChipChannelList& channelList, bool connect);

            void setCurrent(const ChipChannel& chipChannel, bool isPositive, uint8_t magnitude);
            void setCurrent(const ChipChannel& chipChannel, int16_t value);
            void setCurrentImmediate(const ChipChannelList& channelList, int16_t value);

            void setPositiveCurrentScale(const ChipChannel& chipChannel, CurrentCalibration scale);
            void setNegativeCurrentScale(const ChipChannel& chipChannel, CurrentCalibration scale);
            void setCurrentScale(const ChipChannel& chipChannel, CurrentScale scale);
            void setCurrentScaleImmediate(const ChipChannelList& channelList, CurrentScale scale);

            void calibrate(const ChipChannelList& channelList);

            friend class ClampCurrentCalibrationHelper;
        private:
            std::vector<double> getMeasuredCurrentsForCalibration(const ChipChannel& chipChannel, Channel& thisChannel);
            std::pair<Registers::Register3::Resistance, uint8_t> getResistorAndStepForCalibration(const ChipChannel& chipChannel, double current) const;
            void createCommandsToCalibrateClampCurrent(const ChipChannel& chipChannel, bool positiveCurrent, double stepSize, std::vector<CurrentCalibration>& scaleValues);

            void calibrateClampCurrent_Internal(const ChipChannelList& channelList, CurrentScale scale, bool positive);
            void calibrateClampCurrent_Bracket(const ChipChannelList& channelList);
            void calibrateClampCurrent_Reseed(const ChipChannelList& channelList);
            void calibrateClampCurrent_BinarySearch(const ChipChannelList& channelList);

            friend struct GetResistorAndStepForCalibrationTest;
            friend struct ::CurrentSourceTest;
        };

        /** \brief Configuration of channel input.
         *
         * <img id="OffChipComponents" src="OffChipComponents.jpg" border="0">
         *
         *  See the "CLAMP Simplified Diagram" section of the datasheet for more information.
         */
        class OffChipComponents : private ChipComponent {
        public:
            OffChipComponents(ClampController& c);

            void setInput(const ChipChannel& chipChannel, Registers::Register8::InputSelect value);
            void setInputImmediate(const ChipChannelList& channelList, Registers::Register8::InputSelect value);
        };

        /// Controls the on-chip temperature sensor
        class TemperatureSensor : private ChipComponent {
        public:
            TemperatureSensor(ClampController& c);

            double readTemperature(unsigned int chip);

        private:
            unsigned int numCycles(double time);
            unsigned int startOffset;
            unsigned int endOffset;
            void createTemperatureSenseCommands(const ChipChannel& chipChannel);
            double getTemperature(const ChipChannel& chipChannel);
        };

        /** \brief Class with methods related to the mux and ADC.
         *
         *  Mostly handles converting data read from the ADC into voltages at the mux.  Converting those to
         *  voltages or currents at the electrode is done separately.
         */
        class Mux : private ChipComponent {
        public:
            Mux(ClampController& c);

            int32_t getValue(const ChipChannel& chipChannel, uint32_t miso, int32_t residual) const;
            double toVoltage(const ChipChannel& chipChannel, int32_t value) const;

        private:
            bool is18bitADC(const ChipChannel& chipChannel) const;
        };

        /** \brief High-level interface for configuring CLAMP chips and channels.
         *
         *  Contains a series of members that mimics the structures on
         *  the chip.  Member functions within each of these provide ways to:
         *       - set configuration parameters
         *       - calibrate the unit
         *       - etc.
         *
         *  The configuration functions come in two flavors.  
         *    - Add the appropriate command to the given channel's command list in memory.  
         *      This is useful when building a more complex command sequence.  
         *    - Update the settings for a list of channels immediately,
         *      i.e., it sends the command sequence to the FPGA then executes it.  These contain the
         *      word Immediate in the function name.
         *
         *  Click on a section of the image below to see more detailed documentation.
         *
         \htmlonly
             <img id="Clamp4_overview" src="Clamp_4_diagram.jpg" border="0" width="738" height="769" orgwidth="738" orgheight="769" usemap="#config-map" alt="" style="height: 770px; width: 738px;">
             <map name="config-map" id="config-map">
             <area alt="" title="Off chip components" href="class_clamp4_1_1_clamp_config_1_1_off_chip_components.html" shape="rect" coords="4,4,305,213" style="outline:none;" target="_self">
             <area alt="" title="Clamp Current Generator" href="class_clamp4_1_1_clamp_config_1_1_clamp_current_generator.html" shape="rect" coords="430,6,734,175" style="outline:none;" target="_self">
             <area alt="" title="Voltage Amplifier" href="class_clamp4_1_1_clamp_config_1_1_voltage_amplifier.html" shape="rect" coords="556,201,733,360" style="outline:none;" target="_self">
             <area alt="" title="Current-to-voltage converter" href="class_clamp4_1_1_clamp_config_1_1_current_to_voltage_converter.html" shape="rect" coords="260,287,471,595" style="outline:none;" target="_self">
             <area alt="" title="Fast transient capacitive compensation" href="class_clamp4_1_1_clamp_config_1_1_fast_transient_capacitive_compensation.html" shape="rect" coords="3,379,223,638" style="outline:none;" target="_self">
             <area alt="" title="Difference Amplifier" href="class_clamp4_1_1_clamp_config_1_1_difference_amplifier.html" shape="rect" coords="476,387,734,594" style="outline:none;" target="_self">
             <area alt="" title="Clamp voltage generator" href="class_clamp4_1_1_clamp_config_1_1_clamp_voltage_generator.html" shape="rect" coords="280,617,734,751" style="outline:none;" target="_self">
             </map>
         \endhtmlonly
         */
        class ClampController {
        public:
            /** \name Voltage clamp (Per-channel)
            */
            //@{
            /// Difference Amplifier configuration
            DifferenceAmplifier differenceAmplifier;
            /// Current-to-Voltage Converter configuration
            CurrentToVoltageConverter currentToVoltageConverter;
            /// Clamp Voltage Generator configuration
            ClampVoltageGenerator clampVoltageGenerator;
            //@}

            /** \name Current clamp (Per-channel)
            */
            //@{
            /// Voltage Amplifier configuration
            VoltageAmplifier voltageAmplifier;
            /// Clamp Current Generator configuration
            ClampCurrentGenerator clampCurrentGenerator;
            //@}

            /** \name Other/both (Per-channel)
            */
            //@{
            /// Input configuration
            OffChipComponents offChipComponents;
            /// Fast Transient Capacitive Compensation configuration
            FastTransientCapacitiveCompensation fastTransientCapacitiveCompensation;
            //@}

            /** \name Per-chip items
            */
            //@{
            /// %Mux/ADC-related functionality
            Mux mux;
            /// Temperature Sensor
            TemperatureSensor temperatureSensor;
            //@}

            ClampController(Board& board);

            void calibrate(const ChipChannelList& channelList);

            Channel& getChannel(const ChipChannel& chipChannel) const;
            Chip& getChip(const ChipChannel& chipChannel) const;
            Board& getBoard() const;

            void executeImmediate(const ChipChannelList& channelList);
            void createRepeatingWriteCurrent(const ChipChannel& chipChannel, WaveformControl::RepeatingCommand::ReadType read, bool MarkerOut, bool digOut, int8_t value, uint32_t numRepeats);
            void createRepeatingWriteVoltage(const ChipChannel& chipChannel, WaveformControl::RepeatingCommand::ReadType read, bool MarkerOut, bool digOut, int16_t value, uint32_t numRepeats);

            void logMeasuredCurrents(const ChipChannelList& channelList);
            void logMeasuredVoltages(const ChipChannelList& channelList);

            void switchToVoltageClamp(const ChipChannel& chipChannel, int holdingVoltage, double desiredBandwidth, Registers::Register3::Resistance r, double capacitiveCompensation);
            void switchToVoltageClampImmediate(const ChipChannelList& channelList, int holdingVoltage, double desiredBandwidth, Registers::Register3::Resistance r, double capacitiveCompensation);
            void switchToCurrentClamp(const ChipChannel& chipChannel, CurrentScale scale, int holdingCurrent, double capacitiveCompensation);
            void switchToCurrentClampImmediate(const ChipChannelList& channelList, CurrentScale scale, int holdingCurrent, double capacitiveCompensation);

            void simplifiedWaveformToWaveform(const ChipChannelList& channelList, bool isVoltageClamp, SimplifiedWaveform& simplifiedWaveform);

        private:
            Board& theBoard;

            void createRepeating(const ChipChannel& chipChannel, WaveformControl::RepeatingCommand::WriteType write, WaveformControl::RepeatingCommand::ReadType read, bool MarkerOut, bool DigOut, uint16_t L, uint32_t numRepeats);
        };
    }
}
