#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "Registers.h"
#include "WaveformCommand.h"
#include "ClampController.h"
#include <string>
#include "streams.h"

class BinaryWriter;

namespace CLAMP {
    namespace WaveformControl {
        class WaveformExtent;
    }

    /// Enum of the channel numbers
    enum ChannelNumber {
        Channel0 = 0,
        Channel1,
        Channel2,
        Channel3
    };

    /// \cond private
    class BandwidthHelper {
    public:
        static double getBandwidth(double r, double c);
        static double getC(double r, double bandwidth);
    };
    /// \endcond

    class Chip;
    /** \brief In-memory representation of one channel on one chip.
     *
     *  A channel is the basic building block of the CLAMP chip.  There are four channels on a CLAMP4 chip, one on a CLAMP1 chip.
     *
     *  A channel has Registers::ChannelRegisters (member variable registers), which control its operation.  It also has a list of waveform commands
     *  that are played out when the FPGA loop is run, assuming this channel is part of the loop.  Channels also have virtual registers (i.e.,
     *  registers on the FPGA, not on the CLAMP chips) that control their behavior.  Finally, the various per-channel calibration parameters, 
     *  measured or calculated during the 'calibrate' methods of the ClampConfig::ClampController and related, are stored here.
     *
     *  See below for details.
     */
    class Channel {
    public:
        Channel(Chip& chip_, ChannelNumber channelIndex_);
        ~Channel();

        /** \name Virtual Register Control
         *
         *   Channels have virtual registers (i.e., registers on the FPGA, not on the CLAMP chips) that control their behavior.
         *
         *   These control
         *    \li the start &amp; end addresses of the waveform (in the Board's WaveformRAM) to play out for this channel
         *    \li whether or not to return readings from this channel over the USB (which is different from whether or not to play the
         *        waveform on the channel)
         *    \li values for ADC selection and scaling, in the mode where ADC input to the FPGA board controls the channel's voltage or current
         *        generator
         *
         *   See the CLAMP Programmer's Guide for more information.
         */
        //@{
        void setStartAddress(uint16_t value);
        uint16_t getStartAddress() const;
        void setEndAddress(uint16_t value);
//        void setADCSelector(uint8_t value);
//        void setADCMultiplier(uint8_t value);
        void setEnable(bool value);
        bool getEnable() const;
        //@}

        /** \brief In-memory representation of on-chip registers.
        *
        *  Typically, these registers are not manipulated directly here, but rather via the ClampConfig::ClampController and its members.
        *  Those classes also have functionality that pushes these register values to the chips, rather than just changing them in memory.
        */
        Registers::ChannelRegisters registers;

        /** \name Waveform control for this channel
         *
         *  The waveform that is played out for this channel is controlled by these members.
         *
         *  The waveform itself is specified in the *commands* member.  It is sent to the FPGA with the commandsToFPGA member.
         *  The members numTimesteps and getIndexedWaveform are useful in interpreting results coming back from the FPGA.
         */
        //@{
        /// List of waveform commands to execute on this channel
        std::vector<WaveformControl::WaveformCommand> commands;
        void commandsToFPGA();
		void nullCommandToFPGA();
        uint32_t numTimesteps();
        std::vector<WaveformControl::IndexedWaveformCommand> getIndexedWaveform();
        //@}

        /// Reference to the chip that contains this channel, for convenience
        Chip& chip;
        /// Index of this channel (e.g., 0 for channel 0).  [0-3] for CLAMP.
        ChannelNumber channelIndex;

        /** \name Calibration parameters
         */
        //@{
        /// Software offset of the ClampConfig::DifferenceAmplifier, measured during ClampConfig::DifferenceAmplifier::calibrate1 and ClampConfig::DifferenceAmplifier::calibrate2.
        int32_t differenceAmpResidual;
        /// Software offset of the ClampConfig::VoltageAmplifier, measured during ClampConfig::VoltageAmplifier::calibrate.
        int32_t voltageAmpResidual;

        /** Best values of calibration for the ClampConfig::ClampCurrentGenerator for various scales
         *
         *  \li bestCalibration[0][*s*] is the calibration for the negative current generator
         *  \li bestCalibration[1][*s*] is the calibration for the positive current generator
         *
         *  Here *s* is of type ClampConfig::CurrentScale
         */
        ClampConfig::CurrentCalibration bestCalibration[2][4];
        /// \cond private
        ClampConfig::ClampCurrentCalibrationHelper calibrationHelper; // Used internally during calibration
        /// \endcond

        /** \brief Measured value of the five feedback resistors, in &Omega;s.
         *
         *  Elsewhere (Registers::Register3::Resistance and on chip), the feedback resistors are referenced 1, 2, 3, 4, and 5, corresponding to on-chip register values.
         *  For convenience, the values here follow that same scheme, i.e., rFeedback[1] is the value of the nominally 200 k&Omega; resistor,
         *  rFeedback[2] the 2 M&Omega;, rFeedback[3] the 20 M&Omega;, rFeedback[4] the 40 M&Omega;, rFeedback[5] is the 80 M&Omega;.
         */
        double rFeedback[6];
        //@}

        /** \name Voltage clamp resistor and bandwidth
         *
         *  These members control the feedback resistor and capacitor.
         *
         *  The feedback resistor is used in voltage clamp mode.  Current from the input flows through the
         *  feedback resistor, thus producing a voltage that is measured by the ClampConfig::DifferenceAmplifier.  Different
         *  values of the feedback resistor provide different amplification and noise characteristics.
         *
         *  The feedback capacitor is parallel to the feedback resistor.  Together, the two form a low pass filter.
         *  Typically the value of the feedback capacitor isn't set directly, but rather the desired bandwidth of
         *  that lowpass filter is specified, and the appropriate capacitance is chosen automatically.
         *
         *  Typically these are set via ClampConfig::CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance, not directly here.
         */
        //@{
        double getFeedbackResistance() const;
        double getNominalResistance() const;
        void setFeedbackResistanceInMemory(Registers::Register3::Resistance r);
        void setDesiredBandwidth(double freq);
        double getActualBandwidth() const;

		/// Step size of voltage clamp DAC, in volts.
		double getVoltageClampStep() const;

		/// Methods for saving the current clamp step size.  These methods do not affect the actual current step
		//  size in any way; they only read and write from/to the rememberedCurrentStep private variable.
        void Channel::rememberCurrentStep(double currentStep);
        double Channel::recallCurrentStep() const;

        /// Desired bandwidth of the low-pass filter, in Hz.
        double desiredBandwidth;
        //@}

        /** \name Logging
         *
         *  When developing code or debugging, it's frequently useful to have a per-channel log of returned voltages or currents.
         *  These members control that, and logging code is found throughout the API that uses them.
         */
        //@{
        void openRawFile(const FILENAME& filename);
        void log(const std::vector<double>& data);
        void closeRawFile();
        //@}

        unsigned int onDiskSize() const;
        friend BinaryWriter& operator<<(BinaryWriter& out, const Channel& channel);

    private:
        void writeVirtualRegister(uint8_t address, uint16_t value);
        bool enable;
        void getBestCapacitorInMemory();

        std::vector<WaveformControl::WaveformCommand> writtenCommands;
        std::shared_ptr<WaveformControl::WaveformExtent> extent;

        // Writer used for file logging
        std::unique_ptr<BinaryWriter> writer;

        friend class Chip;

		double rememberedCurrentStep;
    };

}
