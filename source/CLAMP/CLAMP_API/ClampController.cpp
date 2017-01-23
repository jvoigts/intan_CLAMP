#include "ClampController.h"
#include "Channel.h"
#include "Chip.h"
#include "common.h"
#include "DataAnalysis.h"
#include <memory>
#include <algorithm>
#include <set>
#include <assert.h>
#include "Board.h"
#include "streams.h"
#include "SimplifiedWaveform.h"

// TEMP
// #include "../Common/Unit Tests/Util.h"

using std::vector;
using std::pair;
using std::runtime_error;
using std::invalid_argument;
using std::unique_ptr;
using std::set;
using namespace CLAMP::Registers;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::WaveformControl;
using namespace CLAMP::SignalProcessing;

namespace CLAMP {
    namespace ClampConfig {
        static bool logCalibrateVoltageAmplifier = true;
        static bool logCalibrateCurrentToVoltageConverter = true;
        static bool logCalibrateCurrentToVoltageConverterDetails = false;
        static bool logCalibrateDifferenceAmplifier1 = true;
        static bool logCalibrateClampVoltage = true;
        static bool logCalibrateDifferenceAmplifier2 = true;
        static bool logClampCurrent = true;
        static bool logCurrentCalibrationDetails = false;

        /** \brief Constructor
         *
         *  \param[in] c  ClampController to use; see controller member.
         */
        ChipComponent::ChipComponent(ClampController& c) :
            controller(c)
        {

        }

        /** \brief Convenience function to access the command list of the given ChipChannel.
         *
         *  \param[in] chipChannel  ChipChannel index
         *  \returns A reference to the command list
         */
        vector<WaveformCommand>& ChipComponent::commands(const ChipChannel& chipChannel) const {
            return controller.getChannel(chipChannel).commands;
        }

        /** \brief Convenience function to access the per-channel registers of the given ChipChannel.
         *
         *  \param[in] chipChannel  ChipChannel index
         *  \returns A reference to the per-channel registers
         */
        ChannelRegisters& ChipComponent::registers(const ChipChannel& chipChannel) const {
            return controller.getChannel(chipChannel).registers;
        }

        /** \brief Convenience function to access the per-chip registers of the given ChipChannel.
         *
         *  \param[in] chipChannel  ChipChannel index.  Note that only the chip part is used.
         *  \returns A reference to the per-chip registers
         */
        GlobalRegisters& ChipComponent::chipRegisters(const ChipChannel& chipChannel) const {
            return controller.getChip(chipChannel).chipRegisters;
        }

        //-------------------------------------------------------------------------
        /** \brief Constructor
         *
         *  \param[in] c  ClampController to use; see ChipComponent::controller member.
         */
        DifferenceAmplifier::DifferenceAmplifier(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Set the positive input to either its normal connection or to ground.
         *
         *  For calibration, the positive input of the the difference amplifier needs to
         *  be connected to ground.  Normally it remains connected to the voltage clamp
         *  op-amp.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] ground       True = connect to ground.  False = connect to voltage clamp op-amp.
         */
        void DifferenceAmplifier::setInPlus(const ChipChannel& chipChannel, bool ground) {
            registers(chipChannel).r5.value.diffAmpInPlusSelect = ground;
            commands(chipChannel).push_back(registers(chipChannel).r5.writeCommand());
        }

        /** \brief Set the positive input to either its normal connection or to ground.
         *
         *  See DifferenceAmplifier::setInPlus for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] ground       True = connect to ground.  False = connect to voltage clamp op-amp.
         */
        void DifferenceAmplifier::setInPlusImmediate(const ChipChannelList& channelList, bool ground) {
            for (auto& index : channelList) {
                setInPlus(index, ground);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Set the negative input to either its normal connection or to ground.
         *
         *  For calibration, the negative input of the the difference amplifier needs to
         *  be connected to ground.  Normally it remains connected to the voltage clamp
         *  op-amp.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] ground       True = connect to ground.  False = connect to voltage clamp op-amp.
         */
        void DifferenceAmplifier::setInMinus(const ChipChannel& chipChannel, bool ground) {
            registers(chipChannel).r5.value.diffAmpInMinusSelect = ground;
            commands(chipChannel).push_back(registers(chipChannel).r5.writeCommand());
        }

        /** \brief Set the negative input to either its normal connection or to ground.
         *
         *  See DifferenceAmplifier::setInMinus for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] ground       True = connect to ground.  False = connect to voltage clamp op-amp.
         */
        void DifferenceAmplifier::setInMinusImmediate(const ChipChannelList& channelList, bool ground) {
            for (auto& index : channelList) {
                setInMinus(index, ground);
            }
            controller.executeImmediate(channelList);
        }

        void DifferenceAmplifier::setOffsetTrim(const ChipChannel& chipChannel, uint8_t value) {
            registers(chipChannel).r5.value.diffAmpOffsetTrim = value;
            commands(chipChannel).push_back(registers(chipChannel).r5.writeCommand());
        }

        void DifferenceAmplifier::createOffsetTrimCommands(const ChipChannel& chipChannel) {
            const unsigned int numRepeats = 100;

            controller.getChannel(chipChannel).differenceAmpResidual = 0;

            // 2. Measure the output of the difference amplifier(i.e., the current sensor).
            // 3. Adjust diff amp offset trim and repeat Step 2 until the output of the difference amplifier is as close to zero as possible.
            //    Any residual offset must be subtracted in software.
            for (unsigned int value = 0; value < 64; value++) {
                setOffsetTrim(chipChannel, value);
                controller.createRepeatingWriteVoltage(chipChannel, RepeatingCommand::READ_CURRENT, false, false, 0, numRepeats + 1);
            }
        }

        void DifferenceAmplifier::getBestOffsetTrimAndResidual(const ChipChannel& chipChannel) {
            // 3. Adjust diff amp offset trim and repeat Step 2 until the output of the difference amplifier is as close to zero as possible.
            //    Any residual offset must be subtracted in software.

            const vector<int32_t>& allRawData = controller.getBoard().readQueue.getRawData(chipChannel);

            vector<int32_t> values(64);
            vector<IndexedWaveformCommand> waveform = controller.getChannel(chipChannel).getIndexedWaveform();
            for (int index = 0; index < 64; index++) {
                auto begin = allRawData.begin() + waveform[2 * index + 1].index,
                     end = begin + waveform[2 * index + 1].command.numRepetitions() - 1;
                assert((end - begin) > 10);
                assert(*begin != INVALID_MUX_VALUE);
                assert(*(end - 1) != INVALID_MUX_VALUE);
                values[index] = DataAnalysis::calculateBestResidual(begin, end);
            }

            pair<unsigned int, int32_t> bestTrim = DataAnalysis::calculateBestTrim(values);

            // Set diffAmpOffsetTrim in memory and on the chips
            commands(chipChannel).clear();
            setOffsetTrim(chipChannel, bestTrim.first);

            controller.getChannel(chipChannel).differenceAmpResidual = bestTrim.second;
        }

        /** \brief Executes the "Difference Amplifier Calibration: Part 1" routine in the data sheet.
         *
         *  At the end of the calibration, stores the hardware offset(s) for the various channels
         *  in RAM and pushes them to the FPGA and chips, and stores the software offset(s) in
         *  the appropriate Channel instance(s).
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void DifferenceAmplifier::calibrate1(const ChipChannelList& channelList) {

            // 1. Connect the diff amp in + select and diff amp in - select switches to ground.
            setInPlusImmediate(channelList, true);
            setInMinusImmediate(channelList, true);

            controller.getBoard().enableChannels(channelList);

            LOG(logCalibrateDifferenceAmplifier1) << "Calibrating Difference Amplifier (1)\n";

            for (auto& index : channelList) {
                createOffsetTrimCommands(index);
            }

            // Now execute
            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            LOG(logCalibrateDifferenceAmplifier1) << "Chan" << "\t" << "Cal" << "\t" << "Res" << "\n";

            for (auto& index : channelList) {
                getBestOffsetTrimAndResidual(index);
                uint16_t value = registers(index).r5.value.diffAmpOffsetTrim;
                LOG(logCalibrateDifferenceAmplifier1) << controller.getChannel(index).channelIndex << "\t" << value << "\t" << controller.getChannel(index).differenceAmpResidual << "\n";
                if (value == 0 || value == 63) {
                    LOG(true) << "Warning: Offset trim is at limit: " << value << "\n";
                }
            }

            // Now push the values to the chips (they're set in getBestOffsetTrimAndResidual)
            controller.executeImmediate(channelList);
            LOG(logCalibrateDifferenceAmplifier1) << "\n";

            // 4. Restore the diff amp in + select and diff amp in - select switches to their default positions.
            setInPlusImmediate(channelList, false);
            setInMinusImmediate(channelList, false);
        }

        /** \brief Executes the "Difference Amplifier Calibration: Part 2" routine in the data sheet.
         *
         *  At the end of the calibration, stores the hardware offset(s) for the various channels
         *  in RAM and pushes them to the FPGA and chips, and stores the software offset(s) in
         *  the appropriate Channel instance(s).
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void DifferenceAmplifier::calibrate2(const ChipChannelList& channelList) {
            // 1. Open the voltage clamp connect switch. 
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, false);
            //    Select the smallest value of the feedback resistance (i.e., 200 kOhms).  
            controller.currentToVoltageConverter.setFeedbackResistanceImmediate(channelList, Register3::R200k);
            //    Select the largest value of feedback capacitance (i.e., 51 pF).
            controller.currentToVoltageConverter.setFeedbackCapacitanceImmediate(channelList, MAX_FEEDBACK_CAPACITANCE);

            // 2. Set the clamp voltage magnitude to zero.
            controller.clampVoltageGenerator.setClampVoltageImmediate(channelList, 0);


            controller.getBoard().enableChannels(channelList);

            LOG(logCalibrateDifferenceAmplifier2) << "Calibrating Difference Amplifier (2)\n";
            for (auto& index : channelList) {
                createOffsetTrimCommands(index);
            }

            // Now execute
            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            LOG(logCalibrateDifferenceAmplifier2) << "Chan" << "\t" << "Cal" << "\t" << "Res" << "\n";

            for (auto& index : channelList) {
                getBestOffsetTrimAndResidual(index);
                uint16_t value = registers(index).r5.value.diffAmpOffsetTrim;
                LOG(logCalibrateDifferenceAmplifier2) << controller.getChannel(index).channelIndex << "\t" << value << "\t" << controller.getChannel(index).differenceAmpResidual << "\n";
                if (value == 0 || value == 63) {
                    LOG(true) << "Warning: Offset trim is at limit: " << value << "\n";
                }
            }

            // Now push the values to the chips (they're set in getBestOffsetTrimAndResidual)
            controller.executeImmediate(channelList);
            LOG(logCalibrateDifferenceAmplifier2) << "\n";


            // 5. Close the voltage clamp connect switch.
            // We'll do this externally
        }

        //-------------------------------------------------------------------------
        /// Constructor
        CurrentToVoltageConverter::CurrentToVoltageConverter(ClampController& c) :
            ChipComponent(c),
            mostRecentCalibrationResistor(Register8::Open0) // Invalid value
        {
        }

        /** \brief Opens or closes the voltage clamp connect switch.
         *
         *  The voltage clamp connect switch connects the current-to-voltage converter to the input.
         *  In voltage clamp mode, this switch should be connected; in current clamp mode it should
         *  be disconnected.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void CurrentToVoltageConverter::setVoltageClampConnect(const ChipChannel& chipChannel, bool connect) {
            registers(chipChannel).r7.value.voltageClampConnect = connect;
            commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
        }

        /** \brief Opens or closes the voltage clamp connect switch.
         *
         *  See CurrentToVoltageConverter::setVoltageClampConnect for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void CurrentToVoltageConverter::setVoltageClampConnectImmediate(const ChipChannelList& channelList, bool connect) {
            for (auto& index : channelList) {
                setVoltageClampConnect(index, connect);
            }
            controller.executeImmediate(channelList);
        }

		void CurrentToVoltageConverter::switchToVoltageClampMode(const ChipChannel& chipChannel, bool connect) {
			registers(chipChannel).r7.value.voltageClampConnect = true;
			registers(chipChannel).r7.value.clampCurrentEnable = false;
			registers(chipChannel).r7.value.fastTransConnect = connect;
			commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
			registers(chipChannel).r6.value.fastTransInSelect = false;
			commands(chipChannel).push_back(registers(chipChannel).r6.writeCommand());
		}

		void CurrentToVoltageConverter::switchToCurrentClampMode(const ChipChannel& chipChannel, bool connect) {
			registers(chipChannel).r7.value.voltageClampConnect = false;
			registers(chipChannel).r7.value.clampCurrentEnable = true;
			registers(chipChannel).r7.value.fastTransConnect = connect;
			commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
			registers(chipChannel).r6.value.fastTransInSelect = true;
			commands(chipChannel).push_back(registers(chipChannel).r6.writeCommand());
		}

        /** \brief Gets the value of the feedback resistor.
         *
         *  The feedback resistor is used in voltage clamp mode.  Current from the input flows through the
         *  feedback resistor, thus producing a voltage that is measured by the DifferenceAmplifier.  Different
         *  values of the feedback resistor provide different amplification and noise characteristics.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \returns Feedback resistor value
         */
        Registers::Register3::Resistance CurrentToVoltageConverter::getFeedbackResistance(const ChipChannel& chipChannel) {
            return registers(chipChannel).r3.value.resistanceEnum();
        }

        /** \brief Sets the value of the feedback resistor.
         *
         *  The feedback resistor is used in voltage clamp mode.  Current from the input flows through the
         *  feedback resistor, thus producing a voltage that is measured by the DifferenceAmplifier.  Different
         *  values of the feedback resistor provide different amplification and noise characteristics.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Feedback resistor value
         */
        void CurrentToVoltageConverter::setFeedbackResistance(const ChipChannel& chipChannel, Register3::Resistance value) {
            registers(chipChannel).r3.value.feedbackResistance = value;
            commands(chipChannel).push_back(registers(chipChannel).r3.writeCommand());
        }

        /** \brief Sets the value of the feedback resistor.
         *
         *  See CurrentToVoltageConverter::setFeedbackResistance for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Feedback resistor value
         */
        void CurrentToVoltageConverter::setFeedbackResistanceImmediate(const ChipChannelList& channelList, Register3::Resistance value) {
            for (auto& index : channelList) {
                setFeedbackResistance(index, value);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the value of the feedback resistor and capacitor.
         *
         *  The feedback resistor is used in voltage clamp mode.  Current from the input flows through the
         *  feedback resistor, thus producing a voltage that is measured by the DifferenceAmplifier.  Different
         *  values of the feedback resistor provide different amplification and noise characteristics.
         *
         *  The feedback capacitor is parallel to the feedback resistor.  Together, the two form a low pass filter.
         *  This function lets you specify the resistor and automatically
         *  sets the feedback capacitor to meet the Channel's desired bandwidth (i.e., cutoff frequency of the low pass filter).
         *
         *  To set the values by hand, see CurrentToVoltageConverter::setFeedbackResistance and CurrentToVoltageConverter::setFeedbackCapacitance.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Feedback resistor value
         */
        void CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance(const ChipChannel& chipChannel, Register3::Resistance value) {
            controller.getChannel(chipChannel).setFeedbackResistanceInMemory(value);
            setFeedbackResistance(chipChannel, static_cast<Register3::Resistance>(registers(chipChannel).r3.value.feedbackResistance));
            setFeedbackCapacitance(chipChannel, registers(chipChannel).r4.value.capacitance());
        }

        /** \brief Sets the value of the feedback resistor and capacitor.
         *
         *  See CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Feedback resistor value
         */
        void CurrentToVoltageConverter::setFeedbackResistanceAndCapacitanceImmediate(const ChipChannelList& channelList, Register3::Resistance value) {
            for (auto& index : channelList) {
                setFeedbackResistanceAndCapacitance(index, value);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the value of the feedback capacitor.
         *
         *  The feedback capacitor is parallel to the feedback resistor.  Together, the two form a low pass filter.
         *  You can either use this function to set the feedback capacitor directly, or use 
         *  CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance, which lets you specify the resistor and automatically
         *  sets the feedback capacitor to meet the Channel's desired bandwidth (i.e., cutoff frequency of the low pass filter).
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Feedback capacitor value, in F.  See Registers::Register4::setFeedbackCapacitance() for limits.
         */
        void CurrentToVoltageConverter::setFeedbackCapacitance(const ChipChannel& chipChannel, double value) {
            registers(chipChannel).r4.value.setFeedbackCapacitance(value);
            commands(chipChannel).push_back(registers(chipChannel).r4.writeCommand());
        }

        /** \brief Sets the value of the feedback capacitor.
         *
         *  See CurrentToVoltageConverter::setFeedbackCapacitance for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Feedback capacitor value, in F.  See Registers::Register4::setFeedbackCapacitance() for limits.
         */
        void CurrentToVoltageConverter::setFeedbackCapacitanceImmediate(const ChipChannelList& channelList, double value) {
            for (auto& index : channelList) {
                setFeedbackCapacitance(index, value);
            }
            controller.executeImmediate(channelList);
        }

        void CurrentToVoltageConverter::createCommandsToCalibrateCurrentToVoltageConverter(const ChipChannel& chipChannel) {
            const unsigned int numRepeats = 100;

            // 3. Set clamp voltage sign and clamp voltage magnitude to produce a desired clamp voltage.  The voltage clamp will 
            //    drive this voltage across the calibration resistor.  The resulting current(I = V / R) will be sensed by the 
            //    current - to - voltage converter.The actual current - to - voltage "gain" may be measured using this known current.
            // 4. Because the clamp voltage DAC has some small nonlinearities, a more accurate measurement will be 
            //    obtained by measuring the current at several different clamp voltages(both positive and negative) and 
            //    fitting a line to the resulting data points.
            for (int value = 50; value >= -50; value--) {
                controller.createRepeatingWriteVoltage(chipChannel, RepeatingCommand::READ_CURRENT, false, false, value, numRepeats + 1);
            }
        }

        void CurrentToVoltageConverter::calculateMeasuredRValue(const ChipChannel& chipChannel, Register3::Resistance r) {
            const vector<double>& allMuxData = controller.getBoard().readQueue.getMuxData(chipChannel);

            vector<double> appliedVoltage;
            vector<double> measuredVoltage;

            vector<IndexedWaveformCommand> waveform = controller.getChannel(chipChannel).getIndexedWaveform();
            for (IndexedWaveformCommand& indexedCmd : waveform) {
                RepeatingCommand& cmd = indexedCmd.command.repeating;
                if (cmd.repeating) {
                    uint16_t tmp = cmd.L;
                    char* t2 = reinterpret_cast<char*>(&tmp);
                    Register0 r0 = *reinterpret_cast<Register0*>(t2);

                    auto begin = allMuxData.begin() + indexedCmd.index,
                         end = begin + indexedCmd.command.numRepetitions() - 1;

                    assert((end - begin) > 10);
                    assert(*begin != INVALID_MUX_VALUE);
                    assert(*(end - 1) != INVALID_MUX_VALUE);

                    measuredVoltage.push_back(DataAnalysis::calculateBestResidual(begin, end));

                    appliedVoltage.push_back(controller.clampVoltageGenerator.getAppliedVoltage(true, r0.clampVoltagePlusSign, r0.clampVoltageMagnitude));
                }
            }

            const unsigned int midpoint = measuredVoltage.size() / 2;
            unsigned int start = midpoint, end = midpoint;
            while (start > 0 && (measuredVoltage[start - 1] < 2.0) && (measuredVoltage[start - 1] > -2.0)) {
                start--;
            }
            while (end < measuredVoltage.size() && (measuredVoltage[end] < 2.0) && (measuredVoltage[end] > -2.0)) {
                end++;
            }

            LOG(logCalibrateCurrentToVoltageConverterDetails) << "i" << "\t" << "App" << "\t" << "Meas" << "\n";
            for (unsigned int i = start; i < end; i++) {
                LOG(logCalibrateCurrentToVoltageConverterDetails) << i << "\t" << *(appliedVoltage.begin() + i) << "\t" << *(measuredVoltage.begin() + i) << "\n";
            }
            LOG(logCalibrateCurrentToVoltageConverterDetails) << "\n";

            double slope = DataAnalysis::slope(appliedVoltage.begin() + start, appliedVoltage.begin() + end, measuredVoltage.begin() + start, measuredVoltage.begin() + end);
            double pearson = DataAnalysis::pearson(appliedVoltage.begin() + start, appliedVoltage.begin() + end, measuredVoltage.begin() + start, measuredVoltage.begin() + end);
            double pearsonDistance = 1 - pearson;
            // slope = step_size * 10 * Rf / Rcal
            double rcal;

            switch (mostRecentCalibrationResistor) {
            case Register8::RCal1:
                rcal = controller.getChip(chipChannel).RCal1;
                break;
            case Register8::RCal2:
                rcal = controller.getChip(chipChannel).RCal2;
                break;
            default:
                throw runtime_error("You shouldn't get here.");
            }
            double rf = slope * rcal / 10.0;
            controller.getChannel(chipChannel).rFeedback[r] = rf;

            LOG(logCalibrateCurrentToVoltageConverter) << "Channel: " << controller.getChannel(chipChannel).channelIndex << "\t" << "R[" << r << "] = " << (rf / 1e6) << " MOhms" << "\t" << pearsonDistance << "\n";;
            if (pearsonDistance > 1e-4) {
                double pearsonDistancePos = 1 - DataAnalysis::pearson(appliedVoltage.begin() + start, appliedVoltage.begin() + midpoint, measuredVoltage.begin() + start, measuredVoltage.begin() + midpoint);
                double pearsonDistanceNeg = 1 - DataAnalysis::pearson(appliedVoltage.begin() + midpoint, appliedVoltage.begin() + end, measuredVoltage.begin() + midpoint, measuredVoltage.begin() + end);
                if (pearsonDistancePos < 1e-4 && pearsonDistanceNeg < 1e-4) {
                    LOG(logCalibrateCurrentToVoltageConverter) << "Positive and negative resistance measurement mismatch.\n";
                }
                else {
                    LOG(logCalibrateCurrentToVoltageConverter) << "Pearson coefficient seems low.  Check for excess noise.\n";
                }
            }
        }

        void CurrentToVoltageConverter::calibrateOneR(const ChipChannelList& channelList, Register3::Resistance r) {
            // 1. Close the voltage clamp connect switch.
            setVoltageClampConnectImmediate(channelList, true);

            // 2. Set the clamp step size of the clamp voltage generator to the 5 mV step size, which is more accurate than the 2.5 mV step size.
            controller.clampVoltageGenerator.setClampStepSizeImmediate(channelList, true);

            // Set the input select switch to one of the calibration resistors(Rcal1 or Rcal2).
            // . . .
            // 5. The user may wish to use two calibration resistors with resistances varying by a factor of 100 or 1000 so 
            //    that the small resistor may be used to calibrate the low feedback resistor values(i.e., 200 kOhm and 2 MOhm) 
            //    while the other resistor may be used to calibrate the high feedback resistor values(i.e., 20 MOhm and 40 MOhm).
            Register8::InputSelect smaller, larger;
            if (controller.getChip(channelList.front()).RCal1 < controller.getChip(channelList.front()).RCal2) {
                smaller = Register8::InputSelect::RCal1;
                larger = Register8::InputSelect::RCal2;
            }
            else {
                smaller = Register8::InputSelect::RCal2;
                larger = Register8::InputSelect::RCal1;
            }
            if (r == Register3::R200k || r == Register3::R2M) {
                mostRecentCalibrationResistor = smaller;
            }
            else {
                mostRecentCalibrationResistor = larger;
            }

			// Override above algorithm and force calibration routine to use RCal1 ONLY.  (Headstage board does not have RCal2 populated.)
			mostRecentCalibrationResistor = Register8::InputSelect::RCal1;

			controller.offChipComponents.setInputImmediate(channelList, mostRecentCalibrationResistor);
			LOG(logCalibrateCurrentToVoltageConverterDetails) << "Calibration resistor: " << mostRecentCalibrationResistor << "\n";

            // Set the feedback resistor and capacitor values
            controller.currentToVoltageConverter.setFeedbackResistanceAndCapacitanceImmediate(channelList, r);



            for (auto& index : channelList) {
                createCommandsToCalibrateCurrentToVoltageConverter(index);
            }

            // Now execute
            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            for (auto& index : channelList) {
                calculateMeasuredRValue(index, r);
            }
            controller.getBoard().readQueue.clear();
            controller.getBoard().clearCommands();

            // Restore 0 voltage, clamp step size, and connection
            controller.clampVoltageGenerator.setClampVoltageImmediate(channelList, 0);
            controller.offChipComponents.setInputImmediate(channelList, Register8::Open0);
            controller.clampVoltageGenerator.setClampStepSizeImmediate(channelList, false);
            setVoltageClampConnectImmediate(channelList, false);
        }

        /** \brief Executes the "Current-to-Voltage Converter Calibration" routine in the data sheet.
         *
         *  At the end of the calibration, stores the calculated values of the resistors in the
         *  rFeedback member variable of the various Channel objects.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void CurrentToVoltageConverter::calibrate(const ChipChannelList& channelList) {
            // Do this separately for each channel, to handle this note from the data sheet:
            //     Note that all four patch clamp units on a chip share the same two calibration resistors, so only one patch clamp unit 
            //     should be connected to these resistors at any time.

            // Split out the input list into (zero or more) lists where each one contains only a single channel
            ChipChannelList channelList2[MAX_NUM_CHANNELS];
            for (auto& index : channelList)
            {
                channelList2[index.channel].push_back(index);
            }

            // Now iterate over those and run
            for (unsigned int channel = 0; channel < MAX_NUM_CHANNELS; channel++) {
                controller.getBoard().enableChannels(channelList2[channel]);
                if (!channelList2[channel].empty()) {
                    calibrateOneR(channelList2[channel], Register3::R200k);
                    calibrateOneR(channelList2[channel], Register3::R2M);
                    calibrateOneR(channelList2[channel], Register3::R20M);
                    calibrateOneR(channelList2[channel], Register3::R40M);
					calibrateOneR(channelList2[channel], Register3::R80M);
                }
            }

            // Restore list of enabled channels
            controller.getBoard().enableChannels(channelList);
        }

        //-------------------------------------------------------------------------
        /// Constructor
        ClampVoltageGenerator::ClampVoltageGenerator(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Sets the sign and magnitude of the clamp voltage.
         *
         *  The clamp voltage is controlled by its step size (2.5 mV steps or 5 mV steps), its sign, and its
         *  magnitude.  The magnitude may vary from 0-255.  This function takes a signed integer input, and uses it
         *  to set both the sign and magnitude of the clamp voltage (but not the step size, which is usually set once
         *  for the application, and left alone).
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Signed value in steps (-255...255).
         */
        void ClampVoltageGenerator::setClampVoltage(const ChipChannel& chipChannel, int16_t value) {
            registers(chipChannel).r0.value.setValue(value);
			commands(chipChannel).push_back(registers(chipChannel).r0.writeConvertCommand(MuxSelection::Unit0Current));  // TODO: Change MuxSelection based on ChipChannel.channel
        }

        /** \brief Sets the sign and magnitude of the clamp voltage.
         *
         *  See ClampVoltageGenerator::setClampVoltage for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Signed value in steps (-255...255).
         */
        void ClampVoltageGenerator::setClampVoltageImmediate(const ChipChannelList& channelList, int16_t value) {
            for (auto& index : channelList) {
                setClampVoltage(index, value);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the step size of the clamp voltage.
         *
         *  The clamp voltage is controlled by its step size, its sign, and its magnitude.
         *
         *  The clamp voltage step size of 2.5 mV/step is used for most patch clamp applications (range = &plusmn;0.6375 V).  The step
         *  size of 5 mV/step is used for FSCV (range = &plusmn;1.275 V).
         *
         * ClampVoltageGenerator::setClampVoltage is used to set sign and magnitude of the applied voltage.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] is5mV        True for 5 mV/step; false for 2.5 mV/step
         */
        void ClampVoltageGenerator::setClampStepSize(const ChipChannel& chipChannel, bool is5mV) {
            registers(chipChannel).r1.value.clampStepSize = is5mV;
            commands(chipChannel).push_back(registers(chipChannel).r1.writeCommand());
        }

        /** \brief Sets the step size of the clamp voltage.
        *
         *  See ClampVoltageGenerator::setClampStepSize for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] is5mV        True for 5 mV/step; false for 2.5 mV/step
         */
        void ClampVoltageGenerator::setClampStepSizeImmediate(const ChipChannelList& channelList, bool is5mV) {
            for (auto& index : channelList) {
                setClampStepSize(index, is5mV);
            }
            controller.executeImmediate(channelList);
        }

        void ClampVoltageGenerator::setClampVoltageOffsetTrim(const ChipChannel& chipChannel, uint8_t value) {
            registers(chipChannel).r2.value.clampVoltageOffsetTrim = value;
            commands(chipChannel).push_back(registers(chipChannel).r2.writeCommand());
        }


        void ClampVoltageGenerator::createCommandsToCalibrateClampVoltage(const ChipChannel& chipChannel) {
            const unsigned int numRepeats = 100;

            // 4. Measure the output of the difference amplifier.
            // 5. Adjust clamp voltage offset trim and repeat Step 4 until the output of the difference amplifier is as close to zero as possible.
            // See also below.
            for (unsigned int value = 0; value < 64; value++) {
                setClampVoltageOffsetTrim(chipChannel, value);
                controller.createRepeatingWriteVoltage(chipChannel, RepeatingCommand::READ_CURRENT, false, false, 0, numRepeats + 1);
            }
        }

        void ClampVoltageGenerator::getBestOffsetTrim(const ChipChannel& chipChannel) {
            // 4. Measure the output of the difference amplifier.
            // 5. Adjust clamp voltage offset trim and repeat Step 4 until the output of the difference amplifier is as close to zero as possible.

            const vector<int32_t>& allRawData = controller.getBoard().readQueue.getRawData(chipChannel);


            vector<int32_t> values(64);
            vector<IndexedWaveformCommand> waveform = controller.getChannel(chipChannel).getIndexedWaveform();
            for (int index = 0; index < 64; index++) {
                auto begin = allRawData.begin() + waveform[2 * index + 1].index,
                     end = begin + waveform[2 * index + 1].command.numRepetitions() - 1;
                assert((end - begin) > 10);
                assert(*begin != INVALID_MUX_VALUE);
                assert(*(end - 1) != INVALID_MUX_VALUE);
                values[index] = DataAnalysis::calculateBestResidual(begin, end);
            }

            pair<unsigned int, int32_t> bestTrim = DataAnalysis::calculateBestTrim(values);

            // Set diffAmpOffsetTrim in memory and on the chips
            commands(chipChannel).clear();
            setClampVoltageOffsetTrim(chipChannel, bestTrim.first);
            LOG(logCalibrateClampVoltage) << "Channel: " << controller.getChannel(chipChannel).channelIndex << "\t" << bestTrim.first << "\n";
            if (bestTrim.first == 0 || bestTrim.first == 63) {
                LOG(true) << "Warning: best trim is at limit " << bestTrim.first << "\n";
            }
        }

        /** \brief Executes the "Clamp Voltage Calibration" routine in the data sheet.
         *
         *  At the end of the calibration, stores the hardware offset(s) for the various channels
         *  in RAM and pushes them to the FPGA and chips.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampVoltageGenerator::calibrate(const ChipChannelList& channelList) {
            // 1. Connect the diff amp in- select switch to ground.
            controller.differenceAmplifier.setInMinusImmediate(channelList, true);

            // 2. Open the voltage clamp connect switch.  
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, false);

            //    Select the smallest value of the feedback resistance (i.e., 200 kOhms).  
            controller.currentToVoltageConverter.setFeedbackResistanceImmediate(channelList, Register3::R200k);
            //    Select the largest value of feedback capacitance (i.e., 51 pF).
            controller.currentToVoltageConverter.setFeedbackCapacitanceImmediate(channelList, MAX_FEEDBACK_CAPACITANCE);

            // 3. Set the clamp voltage magnitude to zero.  The settings of clamp voltage sign and clamp step size do not matter.
            setClampVoltageImmediate(channelList, 0);

            controller.getBoard().enableChannels(channelList);

            LOG(logCalibrateClampVoltage) << "Calibrating Clamp Voltage\n";
            for (auto& index : channelList) {
                createCommandsToCalibrateClampVoltage(index);
            }

            // Now execute
            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            for (auto& index : channelList) {
                getBestOffsetTrim(index);
            }

            // Now push the values to the chips (they're set in getBestOffsetTrim)
            controller.executeImmediate(channelList);
            LOG(logCalibrateClampVoltage) << "\n";

            // 6. Restore the diff amp in- select switch to its default position.
            controller.differenceAmplifier.setInMinusImmediate(channelList, false);
        }

        /** \brief Convenience method for converting an applied voltage in chip-units to volts.
         *
         *  \param[in] is5mV        True for 5 mV/step; false for 2.5 mV/step
         *  \param[in] isPlus       True for positive voltages; false for negative
         *  \param[in] magnitude    Magnitude in steps (whose size is controlled by is5mV)
         *  \returns Applied voltage in Volts.
         */
        double ClampVoltageGenerator::getAppliedVoltage(bool is5mV, bool isPlus, uint8_t magnitude) {
            double stepSize = is5mV ? 5e-3 : 2.5e-3;

            return (isPlus ? +1 : -1) * magnitude * stepSize;
        }

        //-------------------------------------------------------------------------
        /// Constructor
        FastTransientCapacitiveCompensation::FastTransientCapacitiveCompensation(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Returns whether the fast trans connect switch is connected or not.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \returns True if the switch is connected; false if it is not.
         */
        bool FastTransientCapacitiveCompensation::getConnect(const ChipChannel& chipChannel) {
            return registers(chipChannel).r7.value.fastTransConnect;
        }

        /** \brief Opens or closes the fast trans connect switch
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void FastTransientCapacitiveCompensation::setConnect(const ChipChannel& chipChannel, bool connect) {
            registers(chipChannel).r7.value.fastTransConnect = connect;
            commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
        }

        /** \brief Opens or closes the fast trans connect switch
         *
         *  See FastTransientCapacitiveCompensation::setConnect for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void FastTransientCapacitiveCompensation::setConnectImmediate(const ChipChannelList& channelList, bool connect) {
            for (auto& index : channelList) {
                setConnect(index, connect);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets fast transient capacitive compensation to voltage clamp or current clamp mode.
         *
         *  The fast trans input select switch has two positions, one where it's connected to the voltage
         *  from voltage clamp mode and one where it's connected to the electrode (used in current clamp
         *  mode).  This function chooses between those two positions.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] currentClamp True = current clamp mode (connect to electrode).  False = voltage clamp mode (connect to clamped voltage).
         */
        void FastTransientCapacitiveCompensation::setInSelect(const ChipChannel& chipChannel, bool currentClamp) {
            registers(chipChannel).r6.value.fastTransInSelect = currentClamp;
            commands(chipChannel).push_back(registers(chipChannel).r6.writeCommand());
        }

        /** \brief Sets fast transient capacitive compensation to voltage clamp or current clamp mode.
         *
         *  See FastTransientCapacitiveCompensation::setInSelect for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] currentClamp True = current clamp mode (connect to electrode).  False = voltage clamp mode (connect to clamped voltage).
         */
        void FastTransientCapacitiveCompensation::setInSelectImmediate(const ChipChannelList& channelList, bool currentClamp) {
            for (auto& index : channelList) {
                setInSelect(index, currentClamp);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the magnitude of fast transient compensation.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] magnitude    Amount of capacitive compensation in F.  See Registers::Register6::setMagnitude() for limits.
         */
        void FastTransientCapacitiveCompensation::setMagnitude(const ChipChannel& chipChannel, double magnitude) {
            registers(chipChannel).r6.value.setMagnitude(magnitude, registers(chipChannel).r8.value.getStepSize());
            commands(chipChannel).push_back(registers(chipChannel).r6.writeCommand());
        }

        /** \brief Gets the magnitude of fast transient compensation.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \returns Amount of capacitive compensation in F.  See Registers::Register6::setMagnitude() for limits.
         */
        double FastTransientCapacitiveCompensation::getMagnitude(const ChipChannel& chipChannel) {
            return registers(chipChannel).r8.value.getStepSize() * registers(chipChannel).r6.value.getMagnitude();
        }

        /** \brief Sets fast transient capacitive compensation to voltage clamp or current clamp mode.
         *
         *  See FastTransientCapacitiveCompensation::setInSelect for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Amount of capacitive compensation, in F.  See Registers::Register6::setMagnitude() for limits.
         */
        void FastTransientCapacitiveCompensation::setMagnitudeImmediate(const ChipChannelList& channelList, double value) {
            for (auto& index : channelList) {
                setMagnitude(index, value);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief "Buzz" functionality
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel      ChipChannel index.
         *  \param[in] largeAmplitude   If true, use +/- 1.28 V for buzz (i.e., 2.56 V total).  If false, use 0 and 1.28 V (i.e., 1.28 V total)
         *  \param[in] seconds          Number of seconds to buzz for
         */
        void FastTransientCapacitiveCompensation::buzz(const ChipChannel& chipChannel, bool largeAmplitude, double seconds) {
            unsigned int positiveValue = Register8::BuzzState::BuzzPos;
            unsigned int negativeValue = largeAmplitude ? Register8::BuzzState::BuzzNeg : Register8::BuzzState::BuzzGnd;
            unsigned int numCycles = std::lround(seconds * controller.getBoard().getSamplingRateHz() / 10); // Run at 5 kHz
            if (numCycles == 0) {
                numCycles = 1;
            }
            // FUTURE: if needed, we could change this waveform to be 1) set buzzSwitch, 2) do either voltage or current clamp 4x, to cut down on commands

			// Save current values of CC capacitor and switch connecting CC to electrode.
			unsigned int savedFastTransRangeValue = registers(chipChannel).r8.value.fastTransRange;
			unsigned int savedFastTransConnectValue = registers(chipChannel).r7.value.fastTransConnect;

			// Set value of CC for buzz and make sure switch connecting CC to electrode is closed.
			if (largeAmplitude) {
				registers(chipChannel).r8.value.fastTransRange = Register8::CompensationRange::Range20pF;
			}
			else {
				registers(chipChannel).r8.value.fastTransRange = Register8::CompensationRange::Range10pF;
			}
			commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
			registers(chipChannel).r7.value.fastTransConnect = 1;
			commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());

			// Generate buzz waveform.
            for (unsigned int i = 0; i < numCycles; i++) {
                registers(chipChannel).r8.value.buzzSwitch = positiveValue;
                for (unsigned int j = 0; j < 5; j++) {
                    commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
                }
                registers(chipChannel).r8.value.buzzSwitch = negativeValue;
                for (unsigned int j = 0; j < 5; j++) {
                    commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
                }
            }
            registers(chipChannel).r8.value.buzzSwitch = Register8::BuzzState::BuzzOff;
            commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());

			// Restore old values of CC capacitor and switch connecting CC to electrode.
			registers(chipChannel).r7.value.fastTransConnect = savedFastTransConnectValue;
			commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
			registers(chipChannel).r8.value.fastTransRange = savedFastTransRangeValue;
			commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
        }

        /** \brief "Buzz" functionality
         *
         *  See FastTransientCapacitiveCompensation::buzz for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList      List of chip/channel pairs to apply this operation to.
         *  \param[in] largeAmplitude   If true, use +/- 1.28 V for buzz (i.e., 2.56 V total).  If false, use 0 and 1.28 V (i.e., 1.28 V total)
         *  \param[in] seconds          Number of seconds to buzz for
         */
        void FastTransientCapacitiveCompensation::buzzImmediate(const ChipChannelList& channelList, bool largeAmplitude, double seconds) {
            for (auto& index : channelList) {
                buzz(index, largeAmplitude, seconds);
            }
            controller.executeImmediate(channelList);
        }

        //-------------------------------------------------------------------------
        /// Constructor
        VoltageAmplifier::VoltageAmplifier(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Turns the voltage amplifier on or off
         *
         *  The voltage amplifier can be turned off to save power.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] on           True = turn voltage amplifier on.  False = turn it off.
         */
        void VoltageAmplifier::setPower(const ChipChannel& chipChannel, bool on) {
            registers(chipChannel).r8.value.voltageAmpPower = on;
            commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
        }

        /** \brief Turns the voltage amplifier on or off
         *
         *  See VoltageAmplifier::setPower for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] on           True = turn voltage amplifier on.  False = turn it off.
         */
        void VoltageAmplifier::setPowerImmediate(const ChipChannelList& channelList, bool on) {
            for (auto& index : channelList) {
                setPower(index, on);
            }
            controller.executeImmediate(channelList);
        }

        void VoltageAmplifier::createCommandsToCalibrateVoltageAmplifier(const ChipChannel& chipChannel) {
            const unsigned int numRepeats = 5000 + 1;

            controller.getChannel(chipChannel).voltageAmpResidual = 0;

            //3. Measure the output of the voltage amplifier.
            controller.createRepeatingWriteVoltage(chipChannel, RepeatingCommand::READ_VOLTAGE, false, false, 0, numRepeats);
        }

        void VoltageAmplifier::getBestResidual(const ChipChannel& chipChannel) {
            //4. Store this value in software and subtract it from successive voltage amplifier measurements.
            const vector<int32_t>& allRawData = controller.getBoard().readQueue.getRawData(chipChannel);
            auto begin = allRawData.begin(),
                    end = allRawData.end() - 1;

            assert((end - begin) > 10);
            assert(*begin != INVALID_MUX_VALUE);
            assert(*(end - 1) != INVALID_MUX_VALUE);

            controller.getChannel(chipChannel).voltageAmpResidual = DataAnalysis::calculateBestResidual(begin, end);
        }

        /** \brief Executes the "Voltage Amplifier Calibration" routine in the data sheet.
         *
         *  At the end of the calibration, stores the software offset(s) for the various channels
         *  in RAM in the voltageAmpResidual member of the Channel object(s).
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        // The voltage amplifier, used in current clamp measurements, does not have a hardware offset trim capability.  Any offset must be
        // subtracted in software.
        void VoltageAmplifier::calibrate(const ChipChannelList& channelList) {
            LOG(logCalibrateVoltageAmplifier) << "Calibrating Voltage Amplifier\n";

            //2. Open the voltage clamp connect switch.
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, false);

            //   Set the input select switch to ground.
            controller.offChipComponents.setInputImmediate(channelList, Register8::InputSelect::Ground);

            //1. Disable fast transient compensation. (Make sure the fast trans connect switch is open.) 
            controller.fastTransientCapacitiveCompensation.setConnectImmediate(channelList, false);

            //   Make sure the voltage amp power bit is set.
            controller.voltageAmplifier.setPowerImmediate(channelList, true);

            //   Make sure the clamp current enable switch is open. 
            controller.clampCurrentGenerator.setEnableImmediate(channelList, false);

            controller.getBoard().enableChannels(channelList);

            for (auto& index : channelList) {
                createCommandsToCalibrateVoltageAmplifier(index);
            }

			// TEMP
			// TempFile raw(L"C:\\Users\\Reid\\Desktop\\CurrentSource_AD2.raw");
			// controller.getChannel(channelList[0]).openRawFile(raw.filename);

            // Now execute
            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            LOG(logCalibrateVoltageAmplifier) << "Chan" << "\t" << "Res" << "\n";

            for (auto& index : channelList) {
                //4. Store this value in software and subtract it from successive voltage amplifier measurements.
                getBestResidual(index);
                LOG(logCalibrateVoltageAmplifier) << controller.getChannel(index).channelIndex << "\t" << controller.getChannel(index).voltageAmpResidual << "\n";
            }
            LOG(logCalibrateVoltageAmplifier) << "\n";

            controller.logMeasuredVoltages(channelList); // Important that we do this after we've gotten the new voltageAmpResidual values

            controller.getBoard().readQueue.clear();
            controller.getBoard().clearCommands();

            //5. Restore the voltage clamp connect and input select switches to their default positions.
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, false);
            controller.offChipComponents.setInputImmediate(channelList, Register8::InputSelect::Open0);

			// TEMP
			// controller.getChannel(channelList[0]).closeRawFile();
        }

        //-------------------------------------------------------------------------
        /// \cond private
        void ClampCurrentCalibrationHelper::init(const ChipChannel& chipChannel, CurrentScale scale, bool positiveCurrent_) {
            positiveCurrent = positiveCurrent_;

            uint8_t nominalCoarse = getNominalCoarse(scale);

            left = CurrentCalibration(nominalCoarse, 0);
            right = CurrentCalibration(nominalCoarse + 1, CurrentCalibration::MAX_FINE);

            stepSize = getStepSize(scale);

            pair<Register3::Resistance, uint8_t> params = controller.clampCurrentGenerator.getResistorAndStepForCalibration(chipChannel, stepSize);
            target = stepSize * params.second;
        }

        void ClampCurrentCalibrationHelper::bracketInit(const ChipChannel& chipChannel) {
            vector<CurrentCalibration> bounds = { left, right };
            controller.getChannel(chipChannel).commands.clear();
            controller.clampCurrentGenerator.createCommandsToCalibrateClampCurrent(chipChannel, positiveCurrent, stepSize, bounds);
        }

        void ClampCurrentCalibrationHelper::measureBracketBounds(const ChipChannel& chipChannel) {
            vector<double> currents = controller.clampCurrentGenerator.getMeasuredCurrentsForCalibration(chipChannel, controller.getChannel(chipChannel));
            leftCurrent = currents[0];
            rightCurrent = currents[1];
        }

        bool ClampCurrentCalibrationHelper::bracketOneStep(const ChipChannel& chipChannel) {
            measureBracketBounds(chipChannel);

            LOG(logCurrentCalibrationDetails) << "Channel: " << controller.clampCurrentGenerator.controller.getChannel(chipChannel).channelIndex << "\t"
                << "(" << (int)left.coarse << "," << (int)left.fine << ")" << "\t" << leftCurrent << "\t"
                << "(" << (int)right.coarse << "," << (int)right.fine << ")" << "\t" << rightCurrent << "\t"
                << target << "\n";

            int delta = right.coarse - left.coarse;
            if (target > fabs(leftCurrent)) {
                int newCoarse = std::max(left.coarse - delta, 0);
                if (left.coarse == newCoarse) {
                    LOG(true) << "Limit reached; calibration failed\n";
                    return false;
                    //throw runtime_error("Limit reached; calibration failed");
                }
                left.coarse = newCoarse;
                return true;
            }
            else if (target < fabs(rightCurrent)) {
                int newCoarse = std::min(right.coarse + delta, (int)CurrentCalibration::MAX_COARSE);
                if (right.coarse == newCoarse) {
                    LOG(true) << "Limit reached; calibration failed\n";
                    return false;
                    //throw runtime_error("Limit reached; calibration failed");
                }
                right.coarse = newCoarse;
                return true;
            }
            return false;
        }

        bool ClampCurrentCalibrationHelper::binaryStepInit(const ChipChannel& chipChannel) {
            if ((left.coarse >= right.coarse) && (left.fine >= right.fine)) {
                // Kind of a no-op
                mid = left;
                LOG(logCurrentCalibrationDetails) << "Done:" << "\t" << "(" << (int)mid.coarse << "," << (int)mid.fine << ")" << "\n";
                return false;
            }

            if (left.coarse < right.coarse) {
                mid.coarse = (left.coarse + right.coarse) / 2;
                mid.fine = 64;
            } else {
                // left.first = right.first
                // left.second < right.second
                mid.coarse = left.coarse;
                mid.fine = (left.fine + right.fine) / 2;
            }

            vector<CurrentCalibration> bounds = { mid };

            controller.getChannel(chipChannel).commands.clear();
            controller.clampCurrentGenerator.createCommandsToCalibrateClampCurrent(chipChannel, positiveCurrent, stepSize, bounds);
            return true;
        }

        bool ClampCurrentCalibrationHelper::binaryStepReInit() {
            if ((saved.fine < (CurrentCalibration::MAX_FINE/4)) && (saved.coarse != 0)) {
                left = CurrentCalibration(saved.coarse - 1, 0);
                right = CurrentCalibration(saved.coarse - 1, CurrentCalibration::MAX_FINE);
                return true;
            }
            else if ((saved.fine >(3 * CurrentCalibration::MAX_FINE / 4)) && (saved.coarse != CurrentCalibration::MAX_COARSE)) {
                left = CurrentCalibration(saved.coarse + 1, 0);
                right = CurrentCalibration(saved.coarse + 1, CurrentCalibration::MAX_FINE);
                return true;
            }
            else {
                // We're good
                left = CurrentCalibration(saved.coarse, 0);
                right = CurrentCalibration(saved.coarse, CurrentCalibration::MAX_FINE);
                return false;
            }
        }


        void ClampCurrentCalibrationHelper::binaryOneStep(const ChipChannel& chipChannel) {
            vector<double> tmp = controller.clampCurrentGenerator.getMeasuredCurrentsForCalibration(chipChannel, controller.getChannel(chipChannel));
            double midCurrent = tmp[0];

            LOG(logCurrentCalibrationDetails) << "(" << (int)left.coarse << "," << (int)left.fine << ")" << "\t" << leftCurrent << "\t"
                << "(" << (int)mid.coarse << "," << (int)mid.fine << ")" << "\t" << midCurrent << "\t"
                << "(" << (int)right.coarse << "," << (int)right.fine << ")" << "\t" << rightCurrent << "\t"
                << target << "\n";

            if (fabs(midCurrent) < target) {
                // right = mid - 1
                if (left.coarse != right.coarse) {
                    right.coarse = mid.coarse - 1;
                    if (right.coarse < left.coarse) {
                        right.coarse = left.coarse;
                    }
                    right.fine = CurrentCalibration::MAX_FINE;
                }
                else {
                    // mid.first is also = left.first = right.first

                    right.coarse = mid.coarse;
                    right.fine = mid.fine - 1;
                }
                rightCurrent = midCurrent;
            }
            else if (fabs(midCurrent) > target) {
                // left = mid + 1
                if (left.coarse != right.coarse) {
                    left.coarse = mid.coarse + 1;
                    if (right.coarse < left.coarse) {
                        left.coarse = right.coarse;
                    }
                    left.fine = 0;
                }
                else {
                    // mid.first is also = left.first = right.first

                    left.coarse = mid.coarse;
                    left.fine = mid.fine + 1;
                }
                leftCurrent = midCurrent;
            }
            else {
                // left = right = mid;
                left = mid;
                right = mid;
            }
        }

        // Chooses either saved or mid
        CurrentCalibration ClampCurrentCalibrationHelper::getBest() const {
            int savedDiff = fabs(saved.fine - (CurrentCalibration::MAX_FINE/2));
            int midDiff = fabs(mid.fine - (CurrentCalibration::MAX_FINE / 2));
            return (midDiff < savedDiff) ? mid : saved;
        }
        /// \endcond

        /** \brief Gets the nominal (i.e., datasheet) coarse value for calibration.
         *
         *  This is used as a starting point for calibration only.
         *
         *  \param[in] scale   Current scale being used
         *  \returns A value for the coarse calibration
         */
        uint8_t getNominalCoarse(CurrentScale scale) {
            switch (scale)
            {
            case I_5pA:
                return 70;
            case I_50pA:
                return 8;
            case I_500pA:
                return 0;
            case I_1nA:
				return 0;
            default:
                return 0;
            }
        }

        /** \brief Converts CurrentScale enum to amperes.
         *
         *  \param[in] scale   Current scale being used
         *  \returns Value of the current scale step size, in amperes.
         */
        double getStepSize(CurrentScale scale) {
            switch (scale)
            {
            case I_5pA:
                return 5e-12;
            case I_50pA:
                return 50e-12;
            case I_500pA:
                return 500e-12;
            case I_1nA:
                return 1e-9;
            default:
                return 0;
            }
        }

        //-------------------------------------------------------------------------
        /// Constructor
        ClampCurrentGenerator::ClampCurrentGenerator(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Opens or closes the clamp current enable switch
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void ClampCurrentGenerator::setEnable(const ChipChannel& chipChannel, bool connect) {
            registers(chipChannel).r7.value.clampCurrentEnable = connect;
            commands(chipChannel).push_back(registers(chipChannel).r7.writeCommand());
        }

        /** \brief Opens or closes the clamp current enable switch
         *
         *  See ClampCurrentGenerator::setEnable for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] connect      True = connect (i.e., close) the switch.  False = disconnect (i.e., open) the switch.
         */
        void ClampCurrentGenerator::setEnableImmediate(const ChipChannelList& channelList, bool connect) {
            for (auto& index : channelList) {
                setEnable(index, connect);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the sign and magnitude of the clamp current.
         *
         *  The clamp current is controlled by its step size (set via ClampCurrentGenerator::setCurrentScale and related),
         *  its sign, and its magnitude.  This function sets both the sign and magnitude of the clamp current.
         *  See also ClampCurrentGenerator::setCurrent(const ChipChannel& chipChannel, int16_t value), which lets you
         *  combine sign and magnitude into a single signed value.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] isPositive   True for positive current, false for negative.
         *  \param[in] magnitude    Unsigned magnitude in steps (0...127).
         */
        void ClampCurrentGenerator::setCurrent(const ChipChannel& chipChannel, bool isPositive, uint8_t magnitude) {
            registers(chipChannel).r9.value.clampCurrentMagnitude = CheckBits(magnitude, 7);
            registers(chipChannel).r9.value.clampCurrentSign = isPositive;
            commands(chipChannel).push_back(registers(chipChannel).r9.writeCommand());
        }

        /** \brief Sets the sign and magnitude of the clamp current.
         *
         *  The clamp current is controlled by its step size (set via ClampCurrentGenerator::setCurrentScale and related),
         *  its sign, and its magnitude.  The magnitude may vary from 0-127.  This function takes a signed integer input, and uses it
         *  to set both the sign and magnitude of the clamp current.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Signed value in steps (-127...127).
         */
        void ClampCurrentGenerator::setCurrent(const ChipChannel& chipChannel, int16_t value) {
            registers(chipChannel).r9.value.setValue(value);
			commands(chipChannel).push_back(registers(chipChannel).r9.writeConvertCommand(MuxSelection::Unit0Voltage));  // TODO: Change MuxSelection based on ChipChannel.channel
        }

        /** \brief Sets the sign and magnitude of the clamp current.
         *
         *  See ClampCurrentGenerator::setCurrent for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Signed value in steps (-127...127).
         */
        void ClampCurrentGenerator::setCurrentImmediate(const ChipChannelList& channelList, int16_t value) {
            for (auto& index : channelList) {
                setCurrent(index, value);
            }
            controller.executeImmediate(channelList);
        }

        /** \brief Sets the current scale for positive currents to a manual value
         *
         *  This function is typically used for diagnostic testing only; typically you would use
         *  ClampCurrentGenerator::setCurrentScale instead.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] scale        Manual calibration values to use
         */
        void ClampCurrentGenerator::setPositiveCurrentScale(const ChipChannel& chipChannel, CurrentCalibration scale) {
            registers(chipChannel).r12.value.positiveCurrentScaleCoarse = scale.coarse;
            commands(chipChannel).push_back(registers(chipChannel).r12.writeCommand());
            registers(chipChannel).r13.value.positiveCurrentScaleFine = scale.fine;
            commands(chipChannel).push_back(registers(chipChannel).r13.writeCommand());
        }

        /** \brief Sets the current scale for negative currents to a manual value
         *
         *  This function is typically used for diagnostic testing only; typically you would use
         *  ClampCurrentGenerator::setCurrentScale instead.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] scale        Manual calibration values to use
         */
        void ClampCurrentGenerator::setNegativeCurrentScale(const ChipChannel& chipChannel, CurrentCalibration scale) {
            registers(chipChannel).r10.value.negativeCurrentScaleCoarse = scale.coarse;
            commands(chipChannel).push_back(registers(chipChannel).r10.writeCommand());
            registers(chipChannel).r11.value.negativeCurrentScaleFine = scale.fine;
            commands(chipChannel).push_back(registers(chipChannel).r11.writeCommand());
        }

        /** \brief Sets the current scale to one of the four pre-defined scales
         *
         *  The clamp current is controlled by its step size, its sign, and its magnitude.  Sign and magnitude
         *  are set via the ClampCurrentGenerator::setCurrent function; step size is set via the current function.
         *
         *  This function uses the calibration values that were stored when ClampCurrentGenerator::calibrate
         *  was run.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] scale        Scale to use (5 pA steps, 50 pA steps, 500 pA steps, or 1 nA steps)
         */
        void ClampCurrentGenerator::setCurrentScale(const ChipChannel& chipChannel, CurrentScale scale) {
            setPositiveCurrentScale(chipChannel, controller.getChannel(chipChannel).bestCalibration[1][scale]);
            setNegativeCurrentScale(chipChannel, controller.getChannel(chipChannel).bestCalibration[0][scale]);
			controller.getChannel(chipChannel).rememberCurrentStep(getStepSize(scale));
        }

        /** \brief Sets the current scale to one of the four pre-defined scales
         *
         *  See ClampCurrentGenerator::setCurrentScale for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] scale        Scale to use (5 pA steps, 50 pA steps, 500 pA steps, or 1 nA steps)
         */
        void ClampCurrentGenerator::setCurrentScaleImmediate(const ChipChannelList& channelList, CurrentScale scale) {
            for (auto& index : channelList) {
                setCurrentScale(index, scale);
            }
            controller.executeImmediate(channelList);
        }


        void ClampCurrentGenerator::createCommandsToCalibrateClampCurrent(const ChipChannel& chipChannel, bool positiveCurrent, double stepSize, std::vector<CurrentCalibration>& scaleValues) {
            pair<Register3::Resistance, uint8_t> params = getResistorAndStepForCalibration(chipChannel, stepSize);

            const unsigned int numRepeats = 100;

            // Set the feedback resistor and capacitor values
            // Need to leave this one here, because it depends on params, which in turn depends on the actual rF values of the various channels
            controller.getChannel(chipChannel).setFeedbackResistanceInMemory(params.first);
            controller.currentToVoltageConverter.setFeedbackResistance(chipChannel, static_cast<Register3::Resistance>(registers(chipChannel).r3.value.feedbackResistance));
            controller.currentToVoltageConverter.setFeedbackCapacitance(chipChannel, registers(chipChannel).r4.value.capacitance());

            // 3. Set clamp current magnitude to its maximum value (i.e., 127).
            //    Repeat for both positive and negative currents by setting clamp current sign appropriately.
            setCurrent(chipChannel, positiveCurrent, params.second);

            // 4. Measure the resulting current using the voltage clamp circuit and adjust the current source scale registers(chipChannel) to achieve the desired current.  
            for (auto& value : scaleValues)
            {
                if (positiveCurrent) {
                    setPositiveCurrentScale(chipChannel, value);
                }
                else {
                    setNegativeCurrentScale(chipChannel, value);
                }
                controller.createRepeatingWriteVoltage(chipChannel, RepeatingCommand::READ_CURRENT, false, false, 0, numRepeats + 1);
            }
        }

        /** \brief Executes the "Clamp Current Calibration" routine in the data sheet.
         *
         *  At the end of the calibration, stores the measured current source scale values
         *  for best achieving the four current scales in RAM in the bestCalibration member of the Channel object(s).
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampCurrentGenerator::calibrate(const ChipChannelList& channelList) {
            controller.getBoard().enableChannels(channelList);

            calibrateClampCurrent_Internal(channelList, CurrentScale::I_5pA, true);
            calibrateClampCurrent_Internal(channelList, CurrentScale::I_5pA, false);
            LOG(logClampCurrent) << "\n";

            calibrateClampCurrent_Internal(channelList, CurrentScale::I_50pA, true);
            calibrateClampCurrent_Internal(channelList, CurrentScale::I_50pA, false);
            LOG(logClampCurrent) << "\n";

            calibrateClampCurrent_Internal(channelList, CurrentScale::I_500pA, true);
            calibrateClampCurrent_Internal(channelList, CurrentScale::I_500pA, false);
            LOG(logClampCurrent) << "\n";

            calibrateClampCurrent_Internal(channelList, CurrentScale::I_1nA, true);
            calibrateClampCurrent_Internal(channelList, CurrentScale::I_1nA, false);
            LOG(logClampCurrent) << "\n";
        }

        void ClampCurrentGenerator::calibrateClampCurrent_BinarySearch(const ChipChannelList& channelList) {
            LOG(logCurrentCalibrationDetails) << "Left" << "\t" << "Current" << "\t\t" << "Mid" << "\t" << "Current" << "\t\t" << "Right" << "\t" << "Current" << "\t\t" << "Target" << "\n";

            for (;;) {
                bool keepGoing = false;
                for (auto& index : channelList) {
                    Channel& thisChannel = controller.getChannel(index);
                    bool stepNeeded = thisChannel.calibrationHelper.binaryStepInit(index);
                    keepGoing = keepGoing || stepNeeded;
                }

                if (!keepGoing) {
                    break;
                }

                // Now execute the command
                controller.getBoard().commandsToFPGA();
                controller.getBoard().runAndReadOneCycle(0);

                controller.logMeasuredCurrents(channelList);

                for (auto& index : channelList) {
                    Channel& thisChannel = controller.getChannel(index);
                    thisChannel.calibrationHelper.binaryOneStep(index);
                }
                controller.getBoard().readQueue.clear();
            }
            controller.getBoard().clearCommands();
        }

        void ClampCurrentGenerator::calibrateClampCurrent_Bracket(const ChipChannelList& channelList) {
            LOG(logCurrentCalibrationDetails) << "Left" << "\t" << "Current1" << "\t" << "Right" << "\t\t" << "Current2" << "\t" << "Target" << "\n";
            set<ChipChannel> done;
            for (;;) {
                for (auto& index : channelList) {
                    if (done.find(index) == done.end()) { // Not in the done list, so we keep going with it
                        Channel& thisChannel = controller.getChannel(index);
                        thisChannel.calibrationHelper.bracketInit(index);
                    }
                }

                // Now execute the command
                controller.getBoard().commandsToFPGA();
                controller.getBoard().runAndReadOneCycle(0);

                controller.logMeasuredCurrents(channelList);

                bool keepGoing = false;
                for (auto& index : channelList) {
                    if (done.find(index) == done.end()) { // Not in the done list, so we keep going with it
                        Channel& thisChannel = controller.getChannel(index);
                        bool stepNeeded = thisChannel.calibrationHelper.bracketOneStep(index);
                        keepGoing = keepGoing || stepNeeded;
                        if (!stepNeeded) {
                            done.insert(index);
                        }
                    }
                }
                controller.getBoard().readQueue.clear();

                if (!keepGoing) {
                    break;
                }
            }

            LOG(logCurrentCalibrationDetails) << "\n\n";

            controller.getBoard().clearCommands();
        }

        void ClampCurrentGenerator::calibrateClampCurrent_Reseed(const ChipChannelList& channelList) {
            bool keepGoing = false;
            for (auto& index : channelList) {
                Channel& thisChannel = controller.getChannel(index);
                bool stepNeeded = thisChannel.calibrationHelper.binaryStepReInit();
                keepGoing = keepGoing || stepNeeded;
            }

            if (keepGoing) {
                LOG(logCurrentCalibrationDetails) << "Reseeding\n";

                // Get the initial upper and lower bounds
                for (auto& index : channelList) {
                    Channel& thisChannel = controller.getChannel(index);
                    thisChannel.calibrationHelper.bracketInit(index);
                }

                // Now execute the command
                controller.getBoard().commandsToFPGA();
                controller.getBoard().runAndReadOneCycle(0);

                for (auto& index : channelList) {
                    Channel& thisChannel = controller.getChannel(index);
                    thisChannel.calibrationHelper.measureBracketBounds(index);
                }
                controller.getBoard().readQueue.clear();
                controller.getBoard().clearCommands();

                // And run the binary search
                calibrateClampCurrent_BinarySearch(channelList);
            }
        }

        void ClampCurrentGenerator::calibrateClampCurrent_Internal(const ChipChannelList& channelList, CurrentScale scale, bool positiveCurrent_) {
            //    Close the voltage clamp connect switch.
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, true);
            //    Set the clamp voltage magnitude to zero.
            controller.clampVoltageGenerator.setClampVoltageImmediate(channelList, 0);
            // 1. Set the input select switch to open circuit.
            controller.offChipComponents.setInputImmediate(channelList, Register8::Open0);

            // 2. Close the clamp current enable switch.
            setEnableImmediate(channelList, true);

            for (auto& index : channelList) {
                Channel& thisChannel = controller.getChannel(index);
                thisChannel.calibrationHelper.init(index, scale, positiveCurrent_);
            }

            // Bracket
            calibrateClampCurrent_Bracket(channelList);

            // Binary Search
            calibrateClampCurrent_BinarySearch(channelList);

            // Store it
            for (auto& index : channelList) {
                Channel& thisChannel = controller.getChannel(index);
                ClampCurrentCalibrationHelper& ch = thisChannel.calibrationHelper;
                ch.saved = ch.mid;
            }

            LOG(logCurrentCalibrationDetails) << "\n\n";

            // Binary Search 2 - reseed
            calibrateClampCurrent_Reseed(channelList);

            for (auto& index : channelList) {
                Channel& thisChannel = controller.getChannel(index);
                CurrentCalibration best = thisChannel.calibrationHelper.getBest();
                thisChannel.bestCalibration[positiveCurrent_][scale] = best;
                LOG(logClampCurrent) << "Best:" << "\t" << "(" << (int)best.coarse << "," << (int)best.fine << ")" << "\n";
            }
            controller.getBoard().clearCommands();

            // Restore 0 current
            setEnableImmediate(channelList, false);
            // Open the voltage clamp connect switch.
            controller.currentToVoltageConverter.setVoltageClampConnectImmediate(channelList, false);
        }

        vector<double> ClampCurrentGenerator::getMeasuredCurrentsForCalibration(const ChipChannel& chipChannel, Channel& thisChannel) {
            const vector<double>& allCurrents = controller.getBoard().readQueue.getMeasuredCurrents(chipChannel);

            vector<IndexedWaveformCommand> waveform = thisChannel.getIndexedWaveform();

            vector<double> currents;

            for (IndexedWaveformCommand& indexedCommand : waveform) {
                RepeatingCommand& cmd = indexedCommand.command.repeating;
                if (cmd.repeating) {
                    auto begin = allCurrents.begin() + indexedCommand.index,
                         end = begin + indexedCommand.command.numRepetitions() - 1;
                    assert((end - begin) > 10);
                    assert(!std::isnan(*begin));
                    assert(!std::isnan(*(end - 1)));
                    currents.push_back(fabs(DataAnalysis::calculateBestResidual(begin, end)));
                }
            }

            return currents;
        }

        pair<Register3::Resistance, uint8_t> ClampCurrentGenerator::getResistorAndStepForCalibration(const ChipChannel& chipChannel, double current) const {
            Register3::Resistance options[5] = { Register3::R200k, Register3::R2M, Register3::R20M, Register3::R40M, Register3::R80M };

            unsigned int optionIndex = 0;
            uint8_t numSteps = 127;
            for (;;) {
                Register3::Resistance rf = options[optionIndex];
                double r = controller.getChannel(chipChannel).rFeedback[rf]; // Use the actual measured value from current-to-voltage converter calibration
                double voltage = current * r * numSteps * 10;

                if (voltage < 0.3) {
                    // If voltage is really low, use the next bigger resistor
                    if (optionIndex < 4) {
                        optionIndex++;
                        continue;
                    }
                    else {
                        // Of course, if there isn't a next bigger resistor, we're done
                        return pair<Register3::Resistance, uint8_t>(rf, numSteps);
                    }
                }
                else if (voltage < 3.0) {
                    if (voltage < 1.5) {
                        // Fine like we are
                        return pair<Register3::Resistance, uint8_t>(rf, numSteps);
                    }
                    else {
                        // Do fewer steps
                        return pair<Register3::Resistance, uint8_t>(rf, 63);
                    }
                }
                else {
                    throw runtime_error("Shouldn't be able to get here");
                }
            }
        }

        //-------------------------------------------------------------------------
        /// Constructor
        OffChipComponents::OffChipComponents(ClampController& c) :
            ChipComponent(c)
        {

        }

        /** \brief Sets a channel's input
         *
         *  In normal use, the channels remain connected to the electrode.  Other inputs (calibration resistors, open, ground)
         *  are used during calibration routines.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Input to use
         */
        void OffChipComponents::setInput(const ChipChannel& chipChannel, Register8::InputSelect value) {
            registers(chipChannel).r8.value.inputSelect = value;
            commands(chipChannel).push_back(registers(chipChannel).r8.writeCommand());
        }

        /** \brief Sets channels' inputs
         *
         *  See OffChipComponents::setInput for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         *  \param[in] value        Input to use
         */
        void OffChipComponents::setInputImmediate(const ChipChannelList& channelList, Register8::InputSelect value) {
            for (auto& index : channelList) {
                setInput(index, value);
            }
            controller.executeImmediate(channelList);
        }

        //-------------------------------------------------------------------------
        /// Constructor
        TemperatureSensor::TemperatureSensor(ClampController& c) :
            ChipComponent(c)
        {
        }

        /** \brief Reads the temperature of a given chip
         *
         * \param[in] chip  %Chip index [0-3]
         * \returns Temperature in C.
         */
        double TemperatureSensor::readTemperature(unsigned int chip) {
            ChipChannelList channelList = { ChipChannel(chip, 0) };
            controller.getBoard().enableChannels(channelList);

            // Create commands
            createTemperatureSenseCommands(channelList.front());

            controller.getBoard().commandsToFPGA();
            controller.getBoard().runAndReadOneCycle(0);

            double temperatureC = getTemperature(channelList.front());

            controller.getBoard().readQueue.clear();
            controller.getBoard().clearCommands();

            return temperatureC;
        }

        unsigned int TemperatureSensor::numCycles(double time) {
            return lround(time * controller.getBoard().getSamplingRateHz());
        }

        void TemperatureSensor::createTemperatureSenseCommands(const ChipChannel& chipChannel) {
            chipRegisters(chipChannel).r0.value.tempen = true;
            chipRegisters(chipChannel).r0.value.tempS1 = true;
            for (unsigned int i = 0; i < numCycles(200e-6); i++) {
                commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
            }

            startOffset = 0;
            chipRegisters(chipChannel).r0.value.tempS1 = false;
            for (unsigned int cycle = 0; cycle < 4; cycle++) {
                chipRegisters(chipChannel).r0.value.tempS2 = false;
                chipRegisters(chipChannel).r0.value.tempS3 = false;
                for (unsigned int i = 0; i < numCycles(100e-6); i++) {
                    commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
                }
                chipRegisters(chipChannel).r0.value.tempS2 = true;
                for (unsigned int i = 0; i < numCycles(100e-6); i++) {
                    commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
                }
                chipRegisters(chipChannel).r0.value.tempS3 = true;
                for (unsigned int i = 0; i < numCycles(100e-6); i++) {
                    commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
                }

                startOffset = commands(chipChannel).size();
                chipRegisters(chipChannel).r0.value.tempS2 = false;
                for (unsigned int i = 0; i < numCycles(100e-6); i++) {
                    commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
                }
            }

            endOffset = commands(chipChannel).size();
            chipRegisters(chipChannel).r0.value.tempS3 = false;
            for (unsigned int i = 0; i < numCycles(100e-6); i++) {
                commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
            }
            chipRegisters(chipChannel).r0.value.tempS1 = true;
            for (unsigned int i = 0; i < numCycles(200e-6); i++) {
                commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
            }
            chipRegisters(chipChannel).r0.value.tempS1 = false;
            chipRegisters(chipChannel).r0.value.tempen = false;
            commands(chipChannel).push_back(chipRegisters(chipChannel).r0.writeConvertCommand(MuxSelection::Temperature));
        }

        double TemperatureSensor::getTemperature(const ChipChannel& chipChannel) {
            const vector<double>& result = controller.getBoard().readQueue.getMuxData(chipChannel);
            double avg = DataAnalysis::average(result.begin() + startOffset + 1, result.begin() + endOffset + 1);

            // Convert from Volts to degrees C
            return (avg / 6.02e-3) - 273;
        }

        //-------------------------------------------------------------------------
        /// Constructor
        ClampController::ClampController(Board& board) : 
            differenceAmplifier(*this),
            currentToVoltageConverter(*this),
            clampVoltageGenerator(*this),
            voltageAmplifier(*this),
            clampCurrentGenerator(*this),
            offChipComponents(*this),
            fastTransientCapacitiveCompensation(*this),
            mux(*this),
            temperatureSensor(*this),
            theBoard(board)
        {
        }

        /** \brief Helper function to get the Channel that corresponds to this ChipChannel
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \returns Reference to the appropriate Channel.
         */
        Channel& ClampController::getChannel(const ChipChannel& chipChannel) const {
            return *getChip(chipChannel).channel[chipChannel.channel];
        }

        /** \brief Helper function to get the Chip that corresponds to this ChipChannel
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \returns Reference to the appropriate Chip.
         */
        Chip& ClampController::getChip(const ChipChannel& chipChannel) const {
            return *theBoard.chip[chipChannel.chip];
        }

        /** \brief Helper function to get the Board that corresponds to this object
         *
         *  \returns Reference to the appropriate Board.
         */
        Board& ClampController::getBoard() const {
            return theBoard;
        }

        /** \brief Executes the "Self-Calibration Routines" in the data sheet.
         *
         *  Stores results in the chips for hardware offsets, in memory for software offsets.
         *  See calibration routines for individual members of this class for more information.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampController::calibrate(const ChipChannelList& channelList) {
            voltageAmplifier.calibrate(channelList);
            differenceAmplifier.calibrate1(channelList);
            clampVoltageGenerator.calibrate(channelList);
            differenceAmplifier.calibrate2(channelList);
            currentToVoltageConverter.calibrate(channelList);
            clampCurrentGenerator.calibrate(channelList);
        }

        /** \brief Debugging function to log measured currents in a simple binary format
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampController::logMeasuredCurrents(const ChipChannelList& channelList) {
            for (auto& index : channelList) {
                Channel& thisChannel = getChannel(index);
                thisChannel.log(getBoard().readQueue.getMeasuredCurrents(index));
            }
        }

        /** \brief Debugging function to log measured voltages in a simple binary format
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampController::logMeasuredVoltages(const ChipChannelList& channelList) {
            for (auto& index : channelList) {
                Channel& thisChannel = getChannel(index);
                thisChannel.log(getBoard().readQueue.getMeasuredVoltages(index));
            }
        }

        /** \brief Executes the commands already queued up for the given channels.
         *
         *  This is mostly a helper function used in the various "Immediate" versions of the functions in the
         *  members of this class, but can be used externally if needed.
         *
         *  It clears out both the read queue and the commands when it's finished; you should use alternate methods
         *  if you want to get the results back.
         *
         *  \param[in] channelList  List of chip/channel pairs to apply this operation to.
         */
        void ClampController::executeImmediate(const ChipChannelList& channelList) {
            getBoard().enableChannels(channelList);

            getBoard().commandsToFPGA();
            getBoard().runAndReadOneCycle(channelList.front().chip); // TODO: What about other chips?
            getBoard().readQueue.clear();
            //getBoard().clearCommands();
			getBoard().clearSelectedCommands(channelList);
        }

        void ClampController::createRepeating(const ChipChannel& chipChannel, RepeatingCommand::WriteType write, RepeatingCommand::ReadType read, bool markerOut, bool digOut, uint16_t L, uint32_t numRepeats) {
            if (numRepeats > 0xFFFF) {
                getChannel(chipChannel).commands.push_back(RepeatingCommand::create(write, read, RepeatingCommand::LITERAL, true, markerOut, digOut, L, numRepeats >> 16));
            }
            getChannel(chipChannel).commands.push_back(RepeatingCommand::create(write, read, RepeatingCommand::LITERAL, false, markerOut, digOut, L, numRepeats & 0xFFFF));
        }

        /** \brief Helper function to create a repeating command in current clamp mode
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] read         What mux to read.  Typically the voltage mux in current clamp mode.
         *  \param[in] value        Signed value (i.e., sign and magnitude) of current to write
         *  \param[in] numRepeats   How many timesteps it should repeat for
         */
        void ClampController::createRepeatingWriteCurrent(const ChipChannel& chipChannel, RepeatingCommand::ReadType read, bool markerOut, bool digOut, int8_t value, uint32_t numRepeats) {
            Register9 r9;
            r9.setValue(value);

            createRepeating(chipChannel, RepeatingCommand::WRITE_CURRENT, read, markerOut, digOut, static_cast<uint16_t>(r9), numRepeats);
        }

        /** \brief Helper function to create a repeating command in voltage clamp mode
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] read         What mux to read.  Typically the current mux in voltage clamp mode.
         *  \param[in] value        Signed value (i.e., sign and magnitude) of voltage to write
         *  \param[in] numRepeats   How many timesteps it should repeat for
         */
        void ClampController::createRepeatingWriteVoltage(const ChipChannel& chipChannel, RepeatingCommand::ReadType read, bool markerOut, bool digOut, int16_t value, uint32_t numRepeats) {
            Register0 r0;
            r0.setValue(value);

            createRepeating(chipChannel, RepeatingCommand::WRITE_VOLTAGE, read, markerOut, digOut, static_cast<uint16_t>(r0), numRepeats);
        }

        /** \brief Switches to Voltage Clamp mode
        *
        *  This function implements the procedure in the data sheet for safely switching from current clamp mode to voltage clamp mode.
        *
        *  Note that this function adds the appropriate command to the command list for
        *  the given channel in computer memory.  It *does not* transmit that command to
        *  the FPGA or the chips; other functions in this class do.
        *
        *  \param[in] chipChannel              ChipChannel index.
        *  \param[in] holdingVoltage           Value of the holding voltage, as a signed value in steps (-255...255)
        *  \param[in] desiredBandwidth         Desired cutoff frequency of the low-pass filter
        *  \param[in] r                        Feedback resistance
        *  \param[in] capacitiveCompensation   Amount of capacitive compensation, in F.  See Registers::Register6::setMagnitude() for limits.
        */
        void ClampController::switchToVoltageClamp(const ChipChannel& chipChannel, int holdingVoltage, double desiredBandwidth, Register3::Resistance r, double capacitiveCompensation) {
            // Set the feedback resistor and capacitor values
            getChannel(chipChannel).desiredBandwidth = desiredBandwidth;
            currentToVoltageConverter.setFeedbackResistanceAndCapacitance(chipChannel, r);

            // Set holding
            clampVoltageGenerator.setClampVoltage(chipChannel, holdingVoltage);

            // Connect voltage clamp
            // currentToVoltageConverter.setVoltageClampConnect(chipChannel, true);

            // Disconnect current clamp
            // clampCurrentGenerator.setEnable(chipChannel, false);

            // Set fastTrans
            // This has the side effect of setting fastTransConnect to the value stored in r7  <-- NOT ANYMORE IN REV4!
            // fastTransientCapacitiveCompensation.setInSelect(chipChannel, false);

			// NEW (replaces last three old commands):
			currentToVoltageConverter.switchToVoltageClampMode(chipChannel, fastTransientCapacitiveCompensation.getConnect(chipChannel));

            // Adjust CC
            fastTransientCapacitiveCompensation.setMagnitude(chipChannel, capacitiveCompensation);

            // Current -> 0
            clampCurrentGenerator.setCurrent(chipChannel, 0);
			clampCurrentGenerator.setCurrent(chipChannel, 0); // Repeat this command to ensure that switching to voltage clamp uses the same number of commands as switching
															  // to current clamp.
        }


        /** \brief Switches to Voltage Clamp mode
         *
         *  See ClampController::switchToVoltageClamp for a description of the logic.  In addition to that logic,
         *  this function *does* transmit commands to the FPGA and executes them.
         *
         *  \param[in] channelList              List of chip/channel pairs to apply this operation to.
         *  \param[in] holdingVoltage           Value of the holding voltage, as a signed value in steps (-255...255)
         *  \param[in] desiredBandwidth         Desired cutoff frequency of the low-pass filter
         *  \param[in] r                        Feedback resistance
         *  \param[in] capacitiveCompensation   Amount of capacitive compensation, in pF [0-20].
         */
        void ClampController::switchToVoltageClampImmediate(const ChipChannelList& channelList, int holdingVoltage, double desiredBandwidth, Register3::Resistance r, double capacitiveCompensation) {
            for (auto& index : channelList) {
                switchToVoltageClamp(index, holdingVoltage, desiredBandwidth, r, capacitiveCompensation);
            }
            executeImmediate(channelList);
        }

        /** \brief Switches to Current Clamp mode
         *
         *  This function implements the procedure in the data sheet for safely switching from voltage clamp mode to current clamp mode.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips; other functions in this class do.
         *
         *  \param[in] chipChannel              ChipChannel index.
         *  \param[in] scale                    Scale to use (5 pA steps, 50 pA steps, 500 pA steps, or 1 nA steps)
         *  \param[in] holdingCurrent           Value of the holding current, as a signed value in steps (-127...127)
         *  \param[in] capacitiveCompensation   Amount of capacitive compensation, in F.  See Registers::Register6::setMagnitude() for limits.
         */
        void ClampController::switchToCurrentClamp(const ChipChannel& chipChannel, CurrentScale scale, int holdingCurrent, double capacitiveCompensation) {
            // Set holding current
            clampCurrentGenerator.setCurrent(chipChannel, holdingCurrent);

            // Set the scale.  Do this second, so that if you're switching from (5 pA scale, value = 100) to (500 pA scale, value = 1), you go in the
            // order (5 pA scale, value = 100), (5 pA scale, value = 1), (500 pA scale, value = 1), and don't shoot a huge current through the system
            clampCurrentGenerator.setCurrentScale(chipChannel, scale);

            // Connect current
            // clampCurrentGenerator.setEnable(chipChannel, true);

            // Disconnect voltage
            // currentToVoltageConverter.setVoltageClampConnect(chipChannel, false);

			// NEW (replaces three old commands):
			currentToVoltageConverter.switchToCurrentClampMode(chipChannel, fastTransientCapacitiveCompensation.getConnect(chipChannel));

            // Adjust CC
            fastTransientCapacitiveCompensation.setMagnitude(chipChannel, capacitiveCompensation);

            // Set fastTrans
            // This has the side effect of setting fastTransConnect to the value stored in r7  <-- NOT ANYMORE IN REV4!
            // fastTransientCapacitiveCompensation.setInSelect(chipChannel, fastTransientCapacitiveCompensation.getConnect(chipChannel));


        }

        /** \brief Switches to Current Clamp mode
        *
        *  See ClampController::switchToCurrentClamp for a description of the logic.  In addition to that logic,
        *  this function *does* transmit commands to the FPGA and executes them.
        *
        *  \param[in] channelList              List of chip/channel pairs to apply this operation to.
        *  \param[in] scale                    Scale to use (5 pA steps, 50 pA steps, 500 pA steps, or 1 nA steps)
        *  \param[in] holdingCurrent           Value of the holding current, as a signed value in steps (-127...127)
        *  \param[in] capacitiveCompensation   Amount of capacitive compensation, in pF [0-20].
        */
        void ClampController::switchToCurrentClampImmediate(const ChipChannelList& channelList, CurrentScale scale, int holdingCurrent, double capacitiveCompensation) {
            for (auto& index : channelList) {
                switchToCurrentClamp(index, scale, holdingCurrent, capacitiveCompensation);
            }
            executeImmediate(channelList);
        }

        /** \brief Creates waveform commands corresponding to a simplified waveform on the given channels.
         *
         *  Note that this function adds the appropriate command to the command list for
         *  the given channel in computer memory.  It *does not* transmit that command to
         *  the FPGA or the chips.
         *
         *  \param[in] channelList              List of chip/channel pairs to apply this operation to.
         *  \param[in] isVoltageClamp           True for voltage clamp (command list will write voltage and read current),
         *                                      false for current clamp (command list will write current and read voltage).
         *  \param[in] simplifiedWaveform       SimplifiedWaveform to convert.
         */
        void ClampController::simplifiedWaveformToWaveform(const ChipChannelList& channelList, bool isVoltageClamp, SimplifiedWaveform& simplifiedWaveform) {
            for (auto& index : channelList) {
                Channel& channel = getChannel(index);
                for (unsigned int i = 0; i < simplifiedWaveform.size(); i++) {
                    unsigned int sz = channel.commands.size();

                    int value = simplifiedWaveform.waveform[i].appliedDiscreteValue;
					bool markerOut = simplifiedWaveform.waveform[i].markerOut;
					bool digOut = simplifiedWaveform.waveform[i].digOut;
                    unsigned int numReps = getBoard().channelRepetition * simplifiedWaveform.waveform[i].numReps();
                    if (isVoltageClamp) {
                        createRepeatingWriteVoltage(index, RepeatingCommand::READ_CURRENT, markerOut, digOut, value, numReps);
                    }
                    else {
                        createRepeatingWriteCurrent(index, RepeatingCommand::READ_VOLTAGE, markerOut, digOut, value, numReps);
                    }

                    simplifiedWaveform.waveform[i].numCommands = channel.commands.size() - sz;
                }
            }
        }

        //-------------------------------------------------------------------------
        /// Constructor
        Mux::Mux(ClampController& c) :
            ChipComponent(c)
        {

        }

        bool Mux::is18bitADC(const ChipChannel& chipChannel) const {
            return controller.getChip(chipChannel).chipRegisters.r3.value.is18bitADC;
        }

        /** \brief Converts a MISO value to a (residual-corrected) signed value.
         *
         *  Handles 16- or 18-bit ADCs and corrects the software residual for the VoltageAmplifier
         *  or DifferenceAmplifier.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] miso         32-bit MISO return value from the chip
         *  \param[in] residual     Software residual for the VoltageAmplifier or DifferenceAmplifier (i.e., negative of the value that was measured when 0 V / 0 A was put across the appropriate amplifier)
         *  \returns Signed value adjusted for residual
         */
        int32_t Mux::getValue(const ChipChannel& chipChannel, uint32_t miso, int32_t residual) const {
            char* tmp = reinterpret_cast<char*>(&miso);
            MISOReturn ret = *reinterpret_cast<MISOReturn*>(tmp);
            if (is18bitADC(chipChannel)) {
                return ret.convert18.value - residual;
            }
            else {
                return ret.convert16.value - residual;
            }
        }

        /** \brief Converts a value to a voltage at the mux.
         *
         *  Takes a corrected value from Mux::getValue and converts it to a voltage at the mux.  Converting to
         *  voltages or currents at the electrode is done separately.
         *
         *  \param[in] chipChannel  ChipChannel index.
         *  \param[in] value        Signed value adjusted for residual
         *  \returns Voltage, in Volts, at the mux.
         */
        double Mux::toVoltage(const ChipChannel& chipChannel, int32_t value) const {
            if (is18bitADC(chipChannel)) {
                return value * STEP18;
            }
            else {
                return value * STEP16;
            }
        }
    }
}

