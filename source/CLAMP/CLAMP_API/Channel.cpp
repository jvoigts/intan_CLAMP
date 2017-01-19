#include "Channel.h"
#include "Board.h"
#include "RAM.h"
#include "common.h"
#include "WaveformCommand.h"
#include <stdexcept>
#include <cmath>
#include <sstream>

using std::endl;
using std::vector;
using std::shared_ptr;
using std::runtime_error;
using std::pair;
using std::unique_ptr;
using std::wstring;
using CLAMP::Registers::Register3;
using namespace CLAMP::WaveformControl;
using namespace CLAMP::ChipProtocol;

namespace CLAMP {
    static bool logBandwidth = false;

    /// \cond private
    double BandwidthHelper::getBandwidth(double r, double c) {
        return 1.0 / (2.0 * PI * r * c);
    }

    double BandwidthHelper::getC(double r, double bandwidth) {
        return 1.0 / (2.0 * PI * r * bandwidth);
    }
    /// \endcond

    //-----------------------------------------------------------------------------------------------------

    /// Constructor
    Channel::Channel(Chip& chip_, ChannelNumber channelIndex_) :
        registers(),
        chip(chip_),
        channelIndex(channelIndex_),
        differenceAmpResidual(0),
        voltageAmpResidual(0),
        calibrationHelper(chip_.board.controller),
        desiredBandwidth(10000)
    {
        registers.setChannelIndex(channelIndex_);

        rFeedback[Register3::R200k] = Register3::nominalResistance(Register3::R200k);
        rFeedback[Register3::R2M] = Register3::nominalResistance(Register3::R2M);
        rFeedback[Register3::R20M] = Register3::nominalResistance(Register3::R20M);
        rFeedback[Register3::R40M] = Register3::nominalResistance(Register3::R40M);
		rFeedback[Register3::R80M] = Register3::nominalResistance(Register3::R80M);

		rememberedCurrentStep = 0.0;
	}

    Channel::~Channel() {

    }

    /** \brief Sets the start address of the waveform on this channel.
     *
     *  This function and Channel::setEndAddress specify the waveform addresses (in global Waveform RAM)
     *  for the waveform that is played on this channel.  The start and end addresses take effect when the
     *  next waveform playback starts - either when you start running or when the address gets to the
     *  end of the waveform and loops to the beginning (at that point, the value in this virtual register
     *  will be used).
     *
     *  It's likely you wouldn't call these directly, but rather create a waveform in the *commands* member
     *  and call Channel::commandsToFPGA().
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] value  Start address of the waveform.
     */
    void Channel::setStartAddress(uint16_t value) {
        writeVirtualRegister(1, CheckBits(value, 15));
    }

    /** \brief Gets the start address of the waveform on this channel.
     *
     *  \returns Start address of the waveform.
     */
    uint16_t Channel::getStartAddress() const {
        if (extent) {
            return extent->start;
        }
        else {
            return 0;
        }
    }

    /** \brief Sets the end address of the waveform on this channel.
     *
     *  See Channel::setStartAddress for more information.
     *
     *  \param[in] value  End address of the waveform.  Note that this is the last address executed,
     *                    i.e., the execution is start..end inclusive.
     */
    void Channel::setEndAddress(uint16_t value) {
        writeVirtualRegister(2, CheckBits(value, 15));
    }

    /** \brief Selects an ADC to use in ADC-input mode
     *
     *  Certain waveform commands take input from an on-FPGA-board ADC and use that to control the clamp voltage
     *  or clamp current.  This virtual register sets which ADC to use in that case.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] value  ADC [0-7].
     */
//    void Channel::setADCSelector(uint8_t value) {
//        writeVirtualRegister(3, CheckBits(value, 3));
//    }

    /** \brief Sets the ADC scale factor in ADC-input mode
     *
     *  Certain waveform commands take input from an on-FPGA-board ADC and use that to control the clamp voltage
     *  or clamp current.  
     *
     *  This variable sets the scale factor when current output is controlled by the scaled value of an ADC (which
     *  ADC is selected by Channel::setADCSelector). The value is scaled value is given by:
     *      \code Voltage_or_current_value = ADC_input * 2^(ADC multiplier - 14) \endcode
     *  So, for example, a value of 14 corresponds to 
     *      \code output = input \endcode
     *  a value of 13 corresponds to 
     *      \code output = input/2 \endcode
     *  etc. Note that the conversion is in ADC units, not in volts; that conversion is done prior to this scaling.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] value  Voltage_or_current_value = ADC_input * 2^(ADC multiplier - 14).
     */
//    void Channel::setADCMultiplier(uint8_t value) {
//        writeVirtualRegister(4, CheckBits(value, 5));
//    }

    /** \brief Controls whether this channel returns data when running.
     *
     *  Note that this is different from whether or not to play the waveform on the channel.  That property
     *  is controlled by Board::changeChannelLoopOrder.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] value  True to enable data return for this channel
     */
    void Channel::setEnable(bool value) {
        writeVirtualRegister(3, value);
        enable = value;
    }

    /** \brief Does this channel return data when running?
     *
     *  \returns  True if this channel is enabled and data will be returned when running
     */
    bool Channel::getEnable() const { 
        return enable; 
    }

    void Channel::writeVirtualRegister(uint8_t address, uint16_t value) {
        VirtualRegisterAddress addr(chip.index, channelIndex, address);

        vector<uint32_t> data(1, value);

        chip.board.writeRAM(addr, data);
    }

    /** \brief Sends the commands in *commands* to the FPGA
     *
     *  Adjusts start and end address appropriately.
     */
    void Channel::commandsToFPGA() {
        if (commands != writtenCommands) {
            extent.reset();

//			if (commands.empty()) {
//				// If the command list for this channel is empty, add a repeating "read from ROM" command since controller always sends SOMETHING when running.
//				commands.push_back(NonrepeatingCommand::create(READ, None, 0xFF, 0));
//			}
            if (!commands.empty()) {
                vector<uint32_t> cmdsAsUint;
                cmdsAsUint.insert(cmdsAsUint.end(), commands.begin(), commands.end());
                extent = chip.board.waveformRAM.write(cmdsAsUint);
                setStartAddress(extent->start);
                setEndAddress(extent->end);
			}
            writtenCommands = commands;
        }
    }

	void Channel::nullCommandToFPGA() {
		commands.clear();
		// Add a repeating "read from ROM" command since controller always sends SOMETHING when running.
		commands.push_back(NonrepeatingCommand::create(READ, None, 0xFF, 0));
		vector<uint32_t> cmdsAsUint;
		cmdsAsUint.insert(cmdsAsUint.end(), commands.begin(), commands.end());
		extent = chip.board.waveformRAM.write(cmdsAsUint);
		setStartAddress(extent->start);
		setEndAddress(extent->end);
		writtenCommands = commands;
	}

    /** \brief Length, in timesteps, of the current command sequence on this channel
     *
     *  For simple/non-repeating commands, there is a one-to-one correspondence between command and timestep.
     *  For repeating commands, the number of repetitions must be considered.  This function calculates
     *  the total length in timesteps of the command sequence.  It's useful for knowing how many timesteps to run
     *  for, in order to run the complete command sequence, for example.
     *
     *  \returns The number of timesteps it will take the current command sequence to execute fully.
     */
    uint32_t Channel::numTimesteps() {
        uint32_t sum = 0;
        for (const WaveformCommand& command : writtenCommands) {
            sum += command.numRepetitions();
        }
        return sum;
    }

    /** \brief Returns an augmented version of the waveform commands.
     *
     *  For interpreting the results of a waveform command, it's frequently useful to have the commands that execute
     *  indexed by how many timesteps they take.  For example, if there are three repeating commands with (8, 6, 4) repetitions,
     *  respectively, it is useful to know that the first command starts at time 0, the second at time 8, and the third at time 14.
     *  This structure calculates those cumulative timesteps, so that calling code doesn't have to.
     *
     * \returns Waveform command list, plus indices.
     */
    vector<IndexedWaveformCommand> Channel::getIndexedWaveform() {
        // Set up the waveform offset structure
        vector<IndexedWaveformCommand> indexedWaveform;
        indexedWaveform.insert(indexedWaveform.end(), commands.begin(), commands.end());

        uint32_t index = 0;
        for (IndexedWaveformCommand& indexedCommand : indexedWaveform)
        {
            indexedCommand.index = index;
            index += indexedCommand.command.numRepetitions();
        }
        return indexedWaveform;
    }

    /** \brief Measured value (in &Omega;s) of the currently selected feedback resistor.
     *
     *  This combines rFeedback (which contains measured values of all four feedback resistors), 
     *  with knowledge of which one is currently selected.
     *
     *  \returns Measured feedback resistance in &Omega;s.
     */
    double Channel::getFeedbackResistance() const {
        return rFeedback[registers.r3.value.resistanceEnum()];
    }

    /** \brief Nominal value (in &Omega;s) of the currently selected feedback resistor.
     *
     *  For example, if you've chosen the 40 M&Omega; feedback resistor, this will return
     *  40 M&Omega;.  In that case, getFeedbackResistance() might return 39.8 M&Omega; or
     *  some other value.
     *
     *  \returns Nominal feedback resistance in &Omega;s.
     */
    double Channel::getNominalResistance() const {
        return registers.r3.value.nominalResistance();
    }

	/** \brief Value (in volts) of the currently selected clamp voltage DAC step.
	*
	*  This value will be either 2.5e-3 or 5.0e-3 depending on the value of the 'clamp step size'
	*  bit in Regeister N,1.
	*
	*  \returns Clamp voltage DAC step in volts.
	*/
	double Channel::getVoltageClampStep() const {
		return registers.r1.value.clampStepSize ? 5.0e-3 : 2.5e-3;
	}

	/** \brief Save a floating point number in the private variable rememberedCurrentStep.
	*
	*  Calls to this method should always pass the known value of the clamp current step, in amps.
	*  This method does NOT change the clamp current step.
	*/
	void Channel::rememberCurrentStep(double currentStep) {
		rememberedCurrentStep = currentStep;
	}

	/** \brief Returns the last value set by rememberCurrentStep()
	*
	*  This returns the value of the private variable rememberedCurrentStep.
	*
	*  \returns Last 'remembered' current step.
	*/
	double Channel::recallCurrentStep() const {
		return rememberedCurrentStep;
	}

    /** \brief Sets the desired bandwidth of the low pass filter (in memory only)
     *
     *  Updates the in-memory capacitor to achieve the desired bandwidth.  (The exact desired 
     *  bandwidth may not be achievable; to see the actual
     *  bandwidth with the selected capacitor, use Channel::getActualBandwidth()).
     *
     *  This function only changes the values in the *registers* member variable; it doesn't send commands
     *  to the FPGA to change the registers, or even put commands into the *commands* member that would do
     *  that.  For that reason, you should probably use ClampConfig::CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance
     *  or related.
     *
     *  \param[in] freq  Cutoff frequency, in Hz.
     */
    void Channel::setDesiredBandwidth(double freq) {
        desiredBandwidth = freq;
        getBestCapacitorInMemory();
    }

    void Channel::getBestCapacitorInMemory() {
        double r = getFeedbackResistance();
        double desiredC = BandwidthHelper::getC(r, desiredBandwidth);
        registers.r4.value.setFeedbackCapacitance(desiredC);
    }

    /** \brief Returns actual bandwidth of the low-pass filter
     *
     *  The low-pass filter formed by the feedback resistor and capacitor is typically controlled
     *  by explicitly setting the feedback resistor and then setting the desired bandwidth.  The
     *  feedback capacitor is set implicitly to achieve as close to the desired bandwidth as possible,
     *  but the exact value may not be achievable.
     *
     *  This function returns the actual bandwidth that will occur with the settings given.
     *
     *  \returns Actual cutoff frequency of the low-pass filter, in Hz.
     */
    double Channel::getActualBandwidth() const {
        double r = getFeedbackResistance();
        double c = registers.r4.value.capacitance();
        return BandwidthHelper::getBandwidth(r, c);
    }

    /** \brief Sets the value of the feedback resistor (explicitly) and capacitor (implicitly) in RAM
     *
     *  Sets the feedback resistor to the value given.  Sets the best feedback capacitor to achieve
     *  the desired bandwidth.  (The exact desired bandwidth may not be achievable; to see the actual
     *  bandwidth with the selected capacitor, use Channel::getActualBandwidth()).
     *
     *  This function only changes the values in the *registers* member variable; it doesn't send commands
     *  to the FPGA to change the registers, or even put commands into the *commands* member that would do
     *  that.  For that reason, you should probably use ClampConfig::CurrentToVoltageConverter::setFeedbackResistanceAndCapacitance
     *  or related.
     *
     *  \param[in] r   Value of the feedback resistor.
     */
    void Channel::setFeedbackResistanceInMemory(Register3::Resistance r) {
        registers.r3.value.feedbackResistance = r;
        getBestCapacitorInMemory();
        LOG(logBandwidth) << "Actual bandwidth:" << "\t" << getActualBandwidth() << "\n";
    }

    /** \brief Open a logging file
     *
     *  \param[in] filename  File name.  Wide string to support Unicode on Windows.
     */
    void Channel::openRawFile(const FILENAME& filename) {
        unique_ptr<FileOutStream> out(new FileOutStream());
        out->open(filename);
        writer.reset(new BinaryWriter(std::move(out), KILO));
    }

    /** \brief Log data to channel-specific logging file, if one is open
     *
     *  \param[in] data  Data to log
     */
    void Channel::log(const vector<double>& data) {
        if (writer.get() != nullptr) {
            for (double value : data) {
                if (!std::isnan(value)) {
                    *writer << value;
                }
            }
        }
    }

    /// Close the logging file
    void Channel::closeRawFile() {
        writer.reset();
    }

    /** Size of this object on disk
     *  \returns The size in bytes
     */
    unsigned int Channel::onDiskSize() const {
        return 14 * sizeof(uint16_t) /* registers */
               + sizeof(differenceAmpResidual) + sizeof(voltageAmpResidual) + 8 * (sizeof(bestCalibration[0][0].coarse) + sizeof(bestCalibration[0][0].fine)) /* calibration */
               + 5 * sizeof(float) /* feedback resistors */
               +sizeof(float); /* desired bandwidth */
    }

    /** \brief Writes Channel-related settings to an output file
     *
     *  Used in the header of the output file, to record calibration values, etc.
     *
     *  \param[in] out      Output stream.
     *  \param[in] channel  %Channel to write.
     *  \returns Output stream.
     */
    BinaryWriter& operator<<(BinaryWriter& out, const Channel& channel) {
        for (unsigned int registerIndex = 0; registerIndex <= 13; registerIndex++) {
            uint16_t registerValue = const_cast<Channel&>(channel).registers.get(registerIndex).value;
            out << registerValue;
        }
        out << channel.differenceAmpResidual;
        out << channel.voltageAmpResidual;
        for (unsigned int a = 0; a < 2; a++) {
            for (unsigned int b = 0; b < 4; b++) {
                out << channel.bestCalibration[a][b].coarse;
                out << channel.bestCalibration[a][b].fine;
            }
        }
        out << channel.rFeedback[1];
        out << channel.rFeedback[2];
        out << channel.rFeedback[3];
        out << channel.rFeedback[4];
		out << channel.rFeedback[5];

        out << channel.desiredBandwidth;

        return out;
    }
}
