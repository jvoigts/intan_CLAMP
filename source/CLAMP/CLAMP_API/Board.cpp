#include "Board.h"
#include "common.h"
#include <sstream>
#include <exception>
#include "USBPacket.h"
#include <stdexcept>
#include "Registers.h"
#include <memory>
#include "DataAnalysis.h"
#include <algorithm>
#include <ctime>
#include "RAM.h"
#include "Constants.h"


using std::string;
using std::ostringstream;
using std::runtime_error;
using std::endl;
using std::vector;
using std::invalid_argument;
using std::unique_ptr;
using std::pair;
using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::exception;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::WaveformControl;

namespace CLAMP {

    const unsigned int INIT_USB_BUFFER_SIZE = 2 * MEGA;

    //----------------------------------------------------------------------------------------
    /** \brief Constructor
     *
     *  \param[in] is18bit  True if 18-bit ADCs are used; false for 16-bit ADCs.
     */
    Board::Board(bool is18bit) : 
        controller(*this),
        readQueue(channels, controller), 
        fifoPercentageFull(0), 
        latency(0),
        is18bitADC(is18bit), 
        usbBuffer(nullptr), 
        usbBufferSize(0), 
        waveformRAM(*this)
    {
        for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
            chip[i] = new Chip(*this, i);
        }
        setUSBBufferSize(INIT_USB_BUFFER_SIZE);

        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            chip[chipIndex]->chipRegisters.r3.value.is18bitADC = is18bit;
        }
        channelRepetition = 1;
		
		usingDac.resize(8);
		for (int i = 0; i < 8; i++) {
			dacInUse[i] = false;
			dacOutputClamp[i] = false;
			usingDac[i].chip = 0;
			usingDac[i].channel = 0;
			digOutEnabled[i] = false;
			digOutDestination[i] = 0;
		}
    }

    Board::~Board() {
        stop();
        flush();
		setStatusLeds(false, 0);
		setSpiPortLeds(0);
        for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
            delete chip[i];
        }
        delete[]usbBuffer;
        digitalOutputExtent.reset();
    }

    /** \brief Open this board's connection to the FPGA
     *
     *  This will almost always be the first thing you call after creating a Board object.
     *
     *  \param[in] dllPath      Path of the Opal Kelly dll.  "" to use default path.
     *  \param[in] bitfilePath  Path of the bit file to upload.  "" to use default path, "main.bit".
     */
    bool Board::open(const string& dllPath, const string& bitfilePath) {
		if (!okb.open(dllPath, bitfilePath)) {
			return false;
		}

        reset();

        enableChannels(getAllChannels());
        setSamplingRate();
        bool adcs[8] = { true, true, true, true, true, true, true, true }; // TODO - delete
        setDataTransfer(adcs, true, true); // TODO - get rid of this function
		for (int i = 0; i < MAX_NUM_CHIPS; i++) {
			enableDigitalMarker(i, false);
			setDigitalMarkerDestination(i, 0);
		}

		// Disable ADC clamp controllers
		for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
			for (unsigned int j = 0; j < MAX_NUM_CHANNELS; j++) {
				enableAdcControl(false, i, 0);
				adcClampControlEnable[i][j] = false;
				adcClampControlSelect[i][j] = 0;
			}
		}

		// Set command sensitivities to default values
		for (int i = 0; i < 8; i++) {
			setVoltageMultiplier(0, 0.1);
			setCurrentMultiplier(0, 1024);
		}
		// updateAdcClampControl(); // redundant

		// Configure DACs with default gains and offsets
		for (int i = 0; i < 8; i++) {
			configureDac(i, false, 0, 0, false);
			setDacVoltageMultiplier(i, 10.0); // V/V
			setDacCurrentMultiplier(i, 100.0); // mV/pA
			setDacVoltageOffset(i, 0);
			setDacCurrentOffset(i, 0);
		}

		readDigitalInManual();
        initialize();

		return true;
    }

    /** \brief Set which channels will be used by the FPGA
     *
     *  Sets two things:
        \li Which channels are enabled (i.e., return results)
        \li On-FPGA list of channels to loop through
     *
     *  The former is controlled by the Channel::setEnable.
     *
     *  As to the latter: for example, if your channelList is
     *  \code  { (0, Channel0), (0, Channel1), (1, Channel0), (2, Channel0), (3, Channel0) } \endcode
     *  (Channels 0 and 1 on chip1, channel 0 on chips 1, 2, 3.)
     *  This would loop through channels 0 and 1.
     *
     *  \param[in] channelList  List of chip/channel pairs to be enabled
     *  \param[in] allowMulti   If true, allows the looping list to be padded with multiple copies of the same channel.  For example,
     *                          if you're only sampling Channel0 (at 50 kHz), you could loop through it 4 times per timestamp (i.e., sample
     *                          at 200 kHz), then the ReadQueue would automatically downsample to 50 kHz.  This produces lower noise, as it's
     *                          better able to filter out high-frequency noise in the 50 kHz - 200 kHz range.  If you specify 1 channel,
     *                          allowMulti=true will repeat it 4 times; 2 channels => 2 times each; 4 channels => 1 time each.  The number of
     *                          times/repetitions is stored in the \ref channelRepetition member.
     */
    void Board::enableChannels(const ChipChannelList& channelList, bool allowMulti) {
        // Disable everything
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            for (unsigned int channelIndex = 0; channelIndex < MAX_NUM_CHANNELS; channelIndex++) {
                chip[chipIndex]->channel[channelIndex]->setEnable(false);
            }
        }

        // Enable the requested channels
        for (auto& index : channelList) {
            chip[index.chip]->channel[index.channel]->setEnable(true);
        }

        // Set the channel loop based on the enabled channels
        bool channelPresent[4] = { false, false, false, false };
        for (auto& index : channelList) {
            channelPresent[index.channel] = true;
        }
        vector<ChannelNumber> newChannels;
        for (unsigned int i = 0; i < 4; i++) {
            if (channelPresent[i]) {
                newChannels.push_back(static_cast<ChannelNumber>(i));
            }
        }
        if (!newChannels.empty()) {
            if (allowMulti) {
                switch (newChannels.size())
                {
                case 1:
                    channelRepetition = 4;
                    newChannels.push_back(newChannels[0]);
                    newChannels.push_back(newChannels[0]);
                    newChannels.push_back(newChannels[0]);
                    break;
                case 2:
                    channelRepetition = 2;
                    newChannels.push_back(newChannels[0]);
                    newChannels.push_back(newChannels[1]);
                    break;
                case 4:
                default:
                    channelRepetition = 1;
                    break;
                }
            }
            else {
                channelRepetition = 1;
            }
            setChannelLoopOrder(newChannels);
        }
    }
	
	void Board::addEnabledChannels(const ChipChannelList& channelList) {
		// Enable the requested channels
		for (auto& index : channelList) {
			chip[index.chip]->channel[index.channel]->setEnable(true);
		}
	}

    void Board::setClockFreq_internal(uint8_t M, uint8_t D) {
        lock_guard<mutex> lockio(commandMutex);
        // Wait for DcmProgDone = 1 before reprogramming clock synthesizer
        for (;;) {
            okb.updateWiresOut();
            if (okb.getWireOutBit(WireOut::Programming, BitMask::DcmProgDoneBitMask) == true)
                break;
        }

        // Reprogram clock synthesizer
        okb.setWireIn(WireIn::DCMFreq, (256 * M + D));
        okb.updateWiresIn();
        okb.activateTriggerIn(Triggers::DCMProgram, Bit::DCMProgramBit);

        // Wait for DataClkLocked = 1 before allowing data acquisition to continue
        for (;;) {
            okb.updateWiresOut();
            if (okb.getWireOutBit(WireOut::Programming, BitMask::DataClkLockedBitMask) == true)
                break;
        }
    }

    void Board::setUSBBufferSize(unsigned int size) {
        if (size > usbBufferSize) {
            if (usbBuffer != nullptr) {
                delete[]usbBuffer;
                usbBuffer = nullptr;
            }
            usbBuffer = new unsigned char[size];
            usbBufferSize = size;
        }
    }

    /** \brief The number of words in the FPGA's FIFO.
     *  \returns See above.
     */
    uint32_t Board::numWordsInFifo() {
        lock_guard<mutex> lockio(commandMutex);
        okb.updateWiresOut();
        return okb.getWireOutDWord(WireOut::NumWordsFIFOHigh, WireOut::NumWordsFIFOLow);
    }

    /** \brief Reads up to numPackets packets into the ReadQueue.  Doesn't block.
     *
     *  Always reads at least a minimum chunk size worth of data, if it is available.
     *  May loop while waiting for the minimum chunk size.  (If, after looping, still
     *  not enough data is available, throws an exception.)
     *
     *  If the FPGA board has some data (more than minimum chunk size), but less than
     *  the requested amount, reads that amount of data.
     *
     *  \param[in] packetsToRead  Maximum number of packets to read
     *  \returns The actual number of packets read
     */
    unsigned int Board::read(unsigned int packetsToRead) {
        unsigned int perPacketSizeWords = (USBPacket::getPerChannelSize() * numActiveChannels() * channelRepetition + USBPacket::getChannelIndependentSize(*this)) / 2;
        unsigned int minChunkPackets = static_cast<unsigned int>(getSamplingRateHz())/30;
        setUSBBufferSize(minChunkPackets * perPacketSizeWords * 2 * 10);
        unsigned int maxPacketsPerRead = usbBufferSize / 2 / perPacketSizeWords;

        minChunkPackets = std::min(minChunkPackets, packetsToRead);
        unsigned int minChunkWords = perPacketSizeWords * minChunkPackets;

        // Wait until we have minChunk words available
        uint32_t inFIFO = numWordsInFifo();
        clock_t begin = clock();
        while (inFIFO < minChunkWords) {
            inFIFO = numWordsInFifo();
            clock_t now = clock();
            double elapsed = double(now - begin) / CLOCKS_PER_SEC;
            if (elapsed > 10.0) { // A chunk should take 1,000/200,000; if it takes 10 s, we're definitely having trouble
                throw runtime_error("Not getting a minimum chunk size.");
            }
        }

        // Now read however many complete packets we can from the FIFO
        unsigned int packetsThisRead = inFIFO / perPacketSizeWords;
        packetsThisRead = std::min(packetsThisRead, packetsToRead);
        packetsThisRead = std::min(packetsThisRead, maxPacketsPerRead);
        okb.readFromPipeOut(PipeOut::Data, 2 * perPacketSizeWords * packetsThisRead, usbBuffer);

        readQueue.parse(usbBuffer, packetsThisRead);

        updateFIFOStats(perPacketSizeWords);

        return packetsThisRead;
    }

    /** \brief Reads numPackets packets into the ReadQueue.  Blocks.
     *
     *  If the FPGA board doesn't have enough data, loops until it does.
     *
     *  \param[in] numPackets  Number of packets to read.
     */
    void Board::blockingRead(unsigned int numPackets) {
        unsigned int packetsToRead = numPackets;

        while (packetsToRead > 0) {
            unsigned int packetsThisRead = read(packetsToRead);

            packetsToRead -= packetsThisRead;
        }

        // For blockingRead, we need to push the last data onto the ReadQueue (since there isn't more coming).
        // If there is more coming and you want to loop, you should do read() instead (essentially copy this loop).
        readQueue.pushLast();
    }

    void Board::updateFIFOStats(unsigned int perPacketSizeWords) {
        uint32_t wordsInFifo = numWordsInFifo();
        fifoPercentageFull = 100.0 * wordsInFifo / FIFO_CAPACITY_WORDS;
        // LOG(false) << "FIFO: " << fifoPercentageFull << " % full.\n";

        double boardSampleRate = getSamplingRateHz();
        double samplePeriod = 1.0 / boardSampleRate;
        latency = 1000.0 * (wordsInFifo / perPacketSizeWords) * samplePeriod;
        // LOG(false) << "LAG: " << latency << " ms.\n";
    }

    void Board::writeRAM(uint16_t start_addr, const vector<uint32_t>& data) {
        lock_guard<mutex> lockio(commandMutex);
        uint16_t length = data.size();
        okb.setWireIn(WireIn::RAMWriteStart, start_addr);
        okb.setWireIn(WireIn::RAMWriteEnd, start_addr + length - 1);
        okb.updateWiresIn();
        okb.activateTriggerIn(Triggers::RAMWrite, Bit::RamWriteBit);

        okb.writeToPipeIn(PipeIn::Waveform, length * sizeof(uint32_t), const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(data.data())));
    }

    /** \brief Sets a sequence of digital output values to be written
     *
     *  The functions writeDigitalOutputRAM(), enableDigitalOutputs(), and enableCommandControlOfDigitalOutputs() control the digital
     *  outputs on the FPGA board.
     *
     *  If the two other functions are called (i.e., both enabled), the digital outputs are controlled with a command sequence synchronized
     *  with the chip-level command sequence.  For example, if a WaveformCommand list of length 10 is used, the *data* parameter below
     *  would be a list of length 10, and when WaveformCommand[i] is sent to the chip, data[i] is sent to the digital outputs.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] data  List of 16-bit digital output values to be run synchronized with the chip command sequence.
     */
  //  void Board::writeDigitalOutputRAM(int port, const std::vector<uint16_t>& data) {
  //      uint16_t offset;
  //      digitalOutputExtent = waveformRAM.writeDigitalOutputs(data, chip[port]->channel[channels[0]]->getStartAddress(), offset);
  //      setDigitalCommandOffset(offset);
  //  }

    /** \brief Starts the board running, and runs it in continuous mode.
     * 
     * Caution: The board will run indefinitely in this mode. Be sure that
     * whatever logic you implement always calls stop() (or deletes the Board object)
     * to stop the board running, even if an error occurred.
     *
     * Caution: Care should be taken in this mode to handle the incoming data
     * promptly (via read() or blockingRead()). The board gets into a weird state
     * if its FIFO fills up.
     *
     * Tip: You can (and should) check whether you are reading data fast enough
     * by using \ref latency and \ref fifoPercentageFull.  This shouldn't be an issue
     * unless you are doing significant amounts of processing without reading from
     * the board.
     */
    void Board::runContinuously() {
        lock_guard<mutex> lockio(commandMutex);
        okb.setWireInBit(WireIn::RunControl, BitMask::RunContinuouslyBitMask, true);
        okb.updateWiresIn();
        okb.activateTriggerIn(Triggers::Start, Bit::StartBit);
    }

    /** \brief Stops the board from running.
     *
     * Only boards that are running continuously (see runContinuously()) need to be 
     * stopped, but boards that are running fixed (see runFixed()) may be stopped
     * if early termination of their command sequence is desired.
     *
     * Note: Does not flush the board's FIFO. This allows you to read the
     * remaining data from the FIFO if you want.
     *
     * Note: A board is automatically stopped when the Board object is deleted.
     */
    void Board::stop() {
        lock_guard<mutex> lockio(commandMutex);
        if (okb.isOpen()) {
            okb.setWireIn(WireIn::MaxTimestepLow, 0);
            okb.setWireIn(WireIn::MaxTimestepHigh, 0);
            okb.setWireInBit(WireIn::RunControl, BitMask::RunContinuouslyBitMask, false);
            okb.updateWiresIn();
        }
    }

    /** \brief Flush the board's FIFO that contains data that has been acquired but not yet returned to the computer.
     *
     * This function can be called after you call stop(), to
     * clear out any remaining data stored in the board's FIFO.
     *
     * A board's FIFO is automatically flushed when the Board object is deleted.
     */
    void Board::flush()
    {
        try {
            if (okb.isOpen()) {
                while (numWordsInFifo() >= usbBufferSize / 2) {
                    okb.readFromPipeOut(PipeOut::Data, usbBufferSize, usbBuffer);
                }
                while (numWordsInFifo() > 0) {
                    okb.readFromPipeOut(PipeOut::Data, 2 * numWordsInFifo(), usbBuffer);
                }
            }
        }
        catch (exception& e) {
            // Since flush may be called in cleanup situations, we should just eat any error that occurs
            LOG(true) << "Exception occurred during flush " << e.what() << "\n";
        }
    }

    void Board::reset() {
        lock_guard<mutex> lockio(commandMutex);
        okb.setWireInBit(WireIn::RunControl, BitMask::ResetBitMask, true);
        okb.updateWiresIn();

        okb.setWireInBit(WireIn::RunControl, BitMask::ResetBitMask, false);
        okb.updateWiresIn();
    }

    /** \brief Sets the values of the 8 on-FPGA-board LEDs
     *
     *  This occurs immediately, whether the board is running or not.
     *
     *  \param[in] value   8-bit value corresponding to 8 LEDs OR-ed together.
     */
    void Board::setFpgaLeds(uint8_t value) {
        lock_guard<mutex> lockio(commandMutex);
        okb.setWireIn(WireIn::LedDisplay, value);
        okb.updateWiresIn();
    }

	/** \brief Sets the values of the 8 SPI port LEDs
	*
	*  This occurs immediately, whether the board is running or not.
	*
	*  \param[in] value   8-bit value corresponding to 8 LEDs OR-ed together.
	*/
	void Board::setSpiPortLeds(uint8_t value) {
		lock_guard<mutex> lockio(commandMutex);
		okb.setWireIn(WireIn::SpiPortLeds, value);
		okb.updateWiresIn();
	}

	/** \brief Sets the values of the 3 status LEDs
	*
	*  This occurs immediately, whether the board is running or not.
	*
	*  \param[in] digitalInControl    true = LEDs B and C are controlled by Digital In 1 and 2
	*  \param[in] value   8-bit value corresponding to 3 LEDs OR-ed together.
	*/
	void Board::setStatusLeds(bool digitalInControl, uint8_t value) {
		lock_guard<mutex> lockio(commandMutex);
		okb.setWireIn(WireIn::StatusLeds, value | (digitalInControl ? (1 << 3) : 0));
		okb.updateWiresIn();
	}

    void Board::setChannelLoopOrder(const std::vector<ChannelNumber>& channels_) {
        bool differentNumberOfChannels = (channels.size() != channels_.size());

        struct {
            uint16_t channel0 : 2;
            uint16_t channel1 : 2;
            uint16_t channel2 : 2;
            uint16_t channel3 : 2;
            uint16_t numChannels : 3;
            uint16_t unused : 5;
        } wireInValue;

        switch (channels_.size())
        {
        case 1:
            wireInValue.channel0 = channels_[0];
            wireInValue.channel1 = 0;
            wireInValue.channel2 = 0;
            wireInValue.channel3 = 0;
            wireInValue.numChannels = 1;
            wireInValue.unused = 0;
            break;
        case 2:
            wireInValue.channel0 = channels_[0];
            wireInValue.channel1 = channels_[1];
            wireInValue.channel2 = 0;
            wireInValue.channel3 = 0;
            wireInValue.numChannels = 2;
            wireInValue.unused = 0;
            break;
        case 4:
            wireInValue.channel0 = channels_[0];
            wireInValue.channel1 = channels_[1];
            wireInValue.channel2 = channels_[2];
            wireInValue.channel3 = channels_[3];
            wireInValue.numChannels = 4;
            wireInValue.unused = 0;
            break;
        default:
            throw invalid_argument("Must specify 1, 2, or 4 channels");
        }

        {
            lock_guard<mutex> lockio(commandMutex);
            char* tmp = reinterpret_cast<char*>(&wireInValue);
            okb.setWireIn(WireIn::Channels, *reinterpret_cast<uint16_t*>(tmp));
            okb.updateWiresIn();
        }

        channels = channels_;
        if (differentNumberOfChannels) {
            setSamplingRate();
        }
    }

    /** \brief Runs the board for a fixed number of time steps.
     *
     * Note: You don't need to call stop() after running this function; it stops
     * automatically when the given number of time steps are finished.
     *
     * \param[in] numTimesteps  Number of timesteps to run for.  Frequently the result of getNumTimesteps().
     */
    void Board::runFixed(uint32_t numTimesteps) {
        lock_guard<mutex> lockio(commandMutex);

        uint32_t maxTimestep = numTimesteps - 1;
        okb.setWireIn(WireIn::MaxTimestepLow, (maxTimestep & 0xFFFF));
        okb.setWireIn(WireIn::MaxTimestepHigh, ((maxTimestep >> 16) & 0xFFFF));

        okb.setWireInBit(WireIn::RunControl, BitMask::RunContinuouslyBitMask, false);
        okb.updateWiresIn();
        okb.activateTriggerIn(Triggers::Start, Bit::StartBit);
    }

    void Board::setSamplingRate() {
        switch (channels.size()) {
        case 1:
            setClockFreq_internal(7, 20);
            break;
        case 2:
            setClockFreq_internal(7, 10);
            break;
        case 4:
            setClockFreq_internal(35, 25);
            break;
        default:
            throw invalid_argument("Invalid number of channels");
        }
    }

    /** \brief Board sampling rate, in Hz.
     *  \returns The rate.
     */
    double Board::getSamplingRateHz() const {
        return 50000;
    }

    /** \brief Returns the number of enabled channels.
     *
     *  See enableChannels() for more information.
     *
     * \returns The number.
     */
    unsigned int Board::numActiveChannels() const {
        unsigned int numChannels = 0;
        ChipChannelList channelList = getAllChannels();

        for (auto& index : channelList) {
            const Channel& thisChannel = *chip[index.chip]->channel[index.channel];
            if (thisChannel.getEnable()) {
                numChannels++;
            }
        }
        return numChannels;
    }

    /// Clears all command lists from all channels
    void Board::clearCommands() {
        ChipChannelList channelList = getAllChannels();

        for (auto& index : channelList) {
            Channel& thisChannel = *chip[index.chip]->channel[index.channel];
            thisChannel.commands.clear();
        }
        commandsToFPGA();
    }

	/// Clears all command lists from selected channels
	void Board::clearSelectedCommands(const ChipChannelList& channelList) {
		for (auto& index : channelList) {
			Channel& thisChannel = *chip[index.chip]->channel[index.channel];
			thisChannel.commands.clear();
		}
		commandsToFPGA();
	}

    /** \brief Takes a command list that should be executed once per chip, and configures the channels.
     *
     *  Essentially this means enabling channel 0 on each chip with the command list and disabling
     *  all other channels.
     *
     *  \param[in] commands  The command list.
     */
    void Board::configurePerChipCommands(const vector<WaveformCommand>& commands) {
        clearCommands();

        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            if (chip[chipIndex]->present) {
                chip[chipIndex]->channel[0]->commands = commands;
            }
        }

        // Enable channel 0 (only) on all present chips
        ChipChannelList channelList;
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            if (chip[chipIndex]->present) {
                channelList.push_back(ChipChannel(chipIndex, 0));
            }
        }
        enableChannels(channelList);

        commandsToFPGA();
    }

    /** \brief Scans for connected chips.
     *
     *  Sets the chip[i].present value, and sets optimal cable delays for chips that are present.
     *
     *  This is called internally in open().  You could call it again later if you wanted to rescan for newly attached chips.
     */
    void Board::scanForChips() {
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            chip[chipIndex]->present = true;
        }

        vector<WaveformCommand> commands = CommonWaveForms::readROMCommands();
        configurePerChipCommands(commands);

        bool isGood[MAX_NUM_CHIPS][MAX_NUM_DELAYS];

        for (unsigned int delay = 0; delay < MAX_NUM_DELAYS; delay++) {
            for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
                chip[chipIndex]->setCableDelay(delay);
            }

            runFixed(commands.size());
            blockingRead(commands.size());

            readBackAll();
            for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
                isGood[chipIndex][delay] = (chip[chipIndex]->chipRegisters.getCompanyDesignation() == L"INTAN");
				// DEBUGOUT(chip[chipIndex]->chipRegisters.company[0].value.value << " " << 
				// 	chip[chipIndex]->chipRegisters.company[1].value.value << " " <<
				// 	chip[chipIndex]->chipRegisters.company[2].value.value << " " <<
				//	chip[chipIndex]->chipRegisters.company[3].value.value << " " <<
				//	chip[chipIndex]->chipRegisters.company[4].value.value << " " << endl);
				// DEBUGOUT("isGood[" << chipIndex << "][" << delay << "] = " << isGood[chipIndex][delay] << endl);
            }
        }

        // Now set optimal delays.  This algorithm sets the cable delay to the second-highest value, which seems to work
		// well.  Originially we selected the second-lowest delay value, but for some reason the lower delay values lead
		// to errors in the SPI data.
		for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
			vector<unsigned int> goodDelays;
			for (int delay = MAX_NUM_DELAYS - 1; delay >= 0; delay--) {
				if (isGood[chipIndex][delay]) {
					goodDelays.push_back(delay);
				}
			}
			if (goodDelays.empty()) {
				chip[chipIndex]->present = false;
				chip[chipIndex]->channel[0]->setEnable(false);
			}
			else if (goodDelays.size() == 1 || goodDelays.size() == 2) {
				chip[chipIndex]->present = true;
				chip[chipIndex]->setCableDelay(goodDelays[0]);
				// DEBUGOUT("Set chip " << chipIndex << " cable delay to " << goodDelays[0] << endl);
			}
			else {
				chip[chipIndex]->present = true;
				chip[chipIndex]->setCableDelay(goodDelays[1]);
				// DEBUGOUT("Set chip " << chipIndex << " cable delay to " << goodDelays[1] << endl);
			}
		}

        // Now run once more, so that we store the ROM values at the best delay
        runFixed(commands.size());
        blockingRead(commands.size());
        readBackAll();
        clearCommands();

		for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
			chip[chipIndex]->numChannels = chip[chipIndex]->chipRegisters.numUnits.value;
		}
    }

    /** \brief Stores the result of all register READ commands in the ReadQueue in the appropriate in-RAM registers.
     *
     *  Clears the ReadQueue.
     */
    void Board::readBackAll() {
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            ChipChannel chipChannel(chipIndex, 0);
            const vector<MOSICommand>& mosi = readQueue.getMOSI(chipChannel);
            const vector<MISOReturn>&  miso = readQueue.getMISO(chipChannel);

            for (unsigned int i = 0; i < mosi.size(); i++) {
                chip[chipIndex]->readBackRegister(mosi[i], miso[i]);
            }
        }
        readQueue.clear();
    }

	void Board::readDigitalInManual() {
		lock_guard<mutex> lockio(commandMutex);

		okb.updateWiresOut();
		expanderBoardDetected = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::ExpanderDetectBitMask);
		expanderBoardIdNumber = (okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::ExpanderIdBitMask) ? 1 : 0);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 2);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);  // Load digital in shift registers on falling edge of serial_LOAD
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[7] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[6] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[5] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[4] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[3] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[2] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[1] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		spiPortPresent[0] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		digOutVoltageLevel = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		userId[2] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		userId[1] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		userId[0] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		serialId[3] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		serialId[2] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		serialId[1] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		okb.setWireIn(WireIn::SerialDigitalInCntl, 1);
		okb.updateWiresIn();
		okb.setWireIn(WireIn::SerialDigitalInCntl, 0);
		okb.updateWiresIn();

		okb.updateWiresOut();
		serialId[0] = okb.getWireOutBit(WireOut::SerialDigitalIn, BitMask::TtlInSerial);

		// DEBUGOUT("expanderBoardDetected: " << expanderBoardDetected << endl);
		// DEBUGOUT("expanderBoardId: " << expanderBoardIdNumber << endl);
		// DEBUGOUT("spiPortPresent: " << spiPortPresent[7] << spiPortPresent[6] << spiPortPresent[5] << spiPortPresent[4] << spiPortPresent[3] << spiPortPresent[2] << spiPortPresent[1] << spiPortPresent[0] << endl);
		// DEBUGOUT("serialId: " << serialId[3] << serialId[2] << serialId[1] << serialId[0] << endl);
		// DEBUGOUT("userId: " << userId[2] << userId[1] << userId[0] << endl);
		// DEBUGOUT("digOutVoltageLevel: " << digOutVoltageLevel << endl);
	}

    void Board::initialize() {
        // Write RAM registers (same for all chips).  Do this first to put the chips into a known good state

        // And, of course, do it for all chips
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            chip[chipIndex]->present = true;
        }

        vector<WaveformCommand> commands = chip[0]->writeAllRegistersCommands();
        configurePerChipCommands(commands);

        runFixed(commands.size());
        blockingRead(commands.size());
        readQueue.clear();
        clearCommands();
        commandsToFPGA();

        // Now figure out which chips are actually present
        scanForChips();

        enableChannels(getPresentChannels());
    }

    ChipChannelList Board::getAllChannels() const {
        ChipChannelList channelList;
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            for (unsigned int channelIndex = 0; channelIndex < MAX_NUM_CHANNELS; channelIndex++) {
                channelList.push_back(ChipChannel(chipIndex, channelIndex));
            }
        }
        return channelList;
    }

    /** \brief Returns a list of all chip/channel combinations, for chips that are present only.
     *
     *  \returns The list.
     */
    ChipChannelList Board::getPresentChannels() const {
        ChipChannelList channelList;
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            if (chip[chipIndex]->present) {
                for (int channelIndex = 0; channelIndex < chip[chipIndex]->numChannels; channelIndex++) {
                    channelList.push_back(ChipChannel(chipIndex, channelIndex));
                }
            }
        }
        return channelList;
    }

    /// Pushes all in-memory command lists to the FPGA.
    void Board::commandsToFPGA() {
		ChipChannelList channelList = getAllChannels();

        for (auto& index : channelList) {
            Channel& thisChannel = *chip[index.chip]->channel[index.channel];
            thisChannel.commandsToFPGA();
        }
    }

	/// Pushes in-memory command lists for particular chips/channels to the FPGA.
	void Board::commandsToFPGA(const ChipChannelList &channelList) {
		for (auto& index : channelList) {
			Channel& thisChannel = *chip[index.chip]->channel[index.channel];
			thisChannel.commandsToFPGA();
		}
	}

	/// Pushes in-memory command lists for one chip only to FPGA; send null commands (reading from ROM) on other chips.
	void Board::commandsToFPGASinglePort(int port) {
		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			if (i == port) {
				Channel& thisChannel = *chip[i]->channel[0];
				thisChannel.commandsToFPGA();
			}
			else {
				Channel& thisChannel = *chip[i]->channel[0];
				thisChannel.nullCommandToFPGA();
			}
		}
	}

    /** \brief Number of timesteps of the command list for a specified chip.
     *
     *  \returns The number of timesteps.
     */
    unsigned int Board::getNumTimesteps(int chipIndex) const {            
		for (unsigned int channelIndex = 0; channelIndex < MAX_NUM_CHANNELS; channelIndex++) {
            Channel& thisChannel = *chip[chipIndex]->channel[channelIndex];
            if (thisChannel.getEnable()) {
                return numRepetitions(thisChannel.commands) / channelRepetition;
            }
        }
        return 0;
    }

    /** \brief Runs the board for one cycle
     *
     *  The length of the cycle is the number of timesteps returned by getNumTimesteps(chipIndex).
     *
     *  \param[in] extraTimesteps  If you want to run slightly longer than one cycle, you can add extra timesteps via
     *                             this parameter.  For example, the very last command's result won't come back (because
     *                             of pipelining in the chip), so you might use a value of 1 to get one extra cycle, and
     *                             throw the very last one away.  But that way you'd get full data for the command cycle.
     */
    void Board::runOneCycle(int chipIndex, unsigned int extraTimesteps) {
        runFixed(getNumTimesteps(chipIndex) + extraTimesteps);
    }

    /// Blocking read of data corresponding to one cycle
    void Board::readOneCycle(int chipIndex) {
        blockingRead(getNumTimesteps(chipIndex));
    }

    /// Convenience method that calls runOneCycle() then readOneCycle().
    void Board::runAndReadOneCycle(int chipIndex) {
        runOneCycle(chipIndex);
        readOneCycle(chipIndex);
    }

    void Board::writeGlobalVirtualRegister(uint8_t address, uint16_t value) {
        GlobalVirtualRegisterAddress addr(address);

        vector<uint32_t> data(1, value);

        writeRAM(addr, data);
    }

    /** \brief Enables/disables board-level data coming back over the USB
     *
     *  This is analogous to Channel::setEnable().
     *
     *  \param[in] adcs     Return board-level ADC values (8 booleans let you return anywhere from 0 to 8 of them)
     *  \param[in] digin    Return digital input values (single 16-bit return value contains all digital inputs)
     *  \param[in] digout   Return digital output values (single 16-bit return value contains all digital outputs)
     */
    void Board::setDataTransfer(bool adcs[8], bool digin, bool digout) {
        uint8_t adcUint = 0;
        for (unsigned int i = 0; i < 8; i++) {
            adcTransfer[i] = adcs[i];
            if (adcs[i]) {
                adcUint |= (1 << i);
            }
        }
        writeGlobalVirtualRegister(0, adcUint);
        
        uint8_t digUint = 0;
        diginTransfer = digin;
        if (digin) {
            digUint |= 1;
        }
        digoutTransfer = digout;
        if (digout) {
            digUint |= 2;
        }
        writeGlobalVirtualRegister(1, digUint);
    }

    void Board::setDigitalCommandOffset(uint16_t offset) {
        writeGlobalVirtualRegister(2, offset);
    }

    /** \brief Enables the digital outputs on the FPGA board.
     *
     *  The functions writeDigitalOutputRAM(), enableDigitalOutputs(), and enableCommandControlOfDigitalOutputs() control the digital
     *  outputs on the FPGA board.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] enable  The value of enable[i] enables/disables the i-th digital output.  Disabled digital outputs are tied to ground.
     */
    //void Board::enableDigitalOutputs(bool enable[16]) {
    //    uint16_t value = 0;
    //    for (unsigned int i = 0; i < 16; i++) {
    //        if (enable[i]) {
    //            value |= (1 << i);
    //        }
    //    }
    //    writeGlobalVirtualRegister(3, value);
    //}

    /** \brief Enables control of the FPGA board's digital outputs via the commands uploaded in writeDigitalOutputRAM().
     *
     *  The functions writeDigitalOutputRAM(), enableDigitalOutputs(), and enableCommandControlOfDigitalOutputs() control the digital
     *  outputs on the FPGA board.
     *
     *  See the CLAMP Programmer's Guide for more information.
     *
     *  \param[in] enable  If true, the values of enabled digital outputs are updated as described in writeDigitalOutputRAM().  If false,
     *                     the enabled digital outputs retain their current values.
     */
    //void Board::enableCommandControlOfDigitalOutputs(bool enable) {
    //    writeGlobalVirtualRegister(4, enable);
    //}

	/** \brief Configure DAC on interface board.
	*
	*
	*  \param[in] dac               DAC channel (0-7).
	*  \param[in] enable            Enable DAC (true/false).
	*  \param[in] port              DAC source: port (0-7 = A-F).
	*  \param[in] channel           DAC source: channel (0-3).
	*  \param[in] outputClamp       DAC source: true = clamp value; false = measured value.
	*
	*  Note: This does NOT configure the CLAMP chips.  It assumes you have already done so, and
	*  merely tells the DACs on the interface box how to interpret ADC results from the headstages.
	*/
	void Board::configureDac(int dac, bool enable, int port, int channel, bool outputClamp) {
		lock_guard<mutex> lockio(commandMutex);

		uint16_t commandWord = 
			static_cast<uint16_t>((enable ? (1 << 0) : 0) + (outputClamp ? (1 << 1) : 0) + (channel << 2) + (port << 4) + (is18bitADC ? (1 << 7) : 0));
		okb.setWireIn(WireIn::DacConfigWord, commandWord);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::DacConfigLoad, dac);

		dacInUse[dac] = enable;
		dacOutputClamp[dac] = outputClamp;
		usingDac[dac].chip = port;
		usingDac[dac].channel = channel;
	}

	bool Board::isDacInUse(int dac, ChipChannel& chipChannel, bool& outputClamp) {
		if (dacInUse[dac]) {
			chipChannel = usingDac[dac];
			outputClamp = dacOutputClamp[dac];
			return true;
		}
		else {
			return false;
		}
	}

	/** \brief Sets the scale factor for DAC output of voltage signals.
	*
	*  \param[in] dac    DAC channel (0-7).
	*  \param[in] gain   Voltage scale factor.
	*/
	void Board::setDacVoltageMultiplier(int dac, double gain) {
		lock_guard<mutex> lockio(commandMutex);
		// TODO: What about 5 mV DAC step size?
		double gainAbs = (gain < 0.0) ? -gain : gain;
		unsigned int gainAbsFixedPoint = static_cast<unsigned int>(gainAbs * 1024.0);
		if (gainAbsFixedPoint > 32767) {
			gainAbsFixedPoint = 32767;
		}
		if (gain < 0.0) {
			gainAbsFixedPoint = 32768 - gainAbsFixedPoint;
			if (gainAbsFixedPoint > 32767) {
				gainAbsFixedPoint = 32767;
			}
		}
		uint16_t gain16bit = static_cast<uint16_t>(((gain < 0.0) ? 32768U : 0U) + gainAbsFixedPoint);
		okb.setWireIn(WireIn::DacConfigWord, gain16bit);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::DacVoltageMultiplierLoad, dac);
	}

	/** \brief Sets the scale factor for DAC output of current signals.
	*
	*  \param[in] dac    DAC channel (0-7).
	*  \param[in] gain   Current-to-voltage scale factor, in units of mV/step.
	*/
	void Board::setDacCurrentMultiplier(int dac, double gain) {
		lock_guard<mutex> lockio(commandMutex);
		double gainAbs = (gain < 0.0) ? -gain : gain;
		unsigned int gainAbsFixedPoint = static_cast<unsigned int>(gainAbs * 204.8);
		if (gainAbsFixedPoint > 32767) {
			gainAbsFixedPoint = 32767;
		}
		if (gain < 0.0) {
			gainAbsFixedPoint = 32768 - gainAbsFixedPoint;
			if (gainAbsFixedPoint > 32767) {
				gainAbsFixedPoint = 32767;
			}
		}
		uint16_t gain16bit = static_cast<uint16_t>(((gain < 0.0) ? 32768U : 0U) + gainAbsFixedPoint);
		okb.setWireIn(WireIn::DacConfigWord, gain16bit);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::DacCurrentMultiplierLoad, dac);
	}

	/** \brief Sets the offset for DAC output of voltage signals.
	*
	*  Note: The offset is only applied to measured values, not clamp values.
	*
	*  \param[in] dac      DAC channel (0-7).
	*  \param[in] value    Signed 16-bit offset.
	*/
	void Board::setDacVoltageOffset(int dac, int16_t value) {
		lock_guard<mutex> lockio(commandMutex);
		okb.setWireIn(WireIn::DacConfigWord, value);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::DacVoltageOffsetLoad, dac);
	}

	/** \brief Sets the offset for DAC output of current signals.
	*
	*  Note: The offset is only applied to measured values, not clamp values.
	*
	*  \param[in] dac      DAC channel (0-7).
	*  \param[in] value    Signed 16-bit offset.
	*/
	void Board::setDacCurrentOffset(int dac, int16_t value) {
		lock_guard<mutex> lockio(commandMutex);
		okb.setWireIn(WireIn::DacConfigWord, value);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::DacCurrentOffsetLoad, dac);
	}

	/** \brief Sets the scale factor for ADC control of voltage clamps.
	*
	*  \param[in] port    SPI port affected.
	*  \param[in] gain   Voltage scale factor.
	*/
	void Board::setVoltageMultiplier(int port, double gain) {
		lock_guard<mutex> lockio(commandMutex);
		// TODO: What about 5 mV DAC step size?
		double gainAbs = (gain < 0.0) ? -gain : gain;
		unsigned int gainAbsFixedPoint = static_cast<unsigned int>(gainAbs * 16384.0);
		if (gainAbsFixedPoint > 32767) {
			gainAbsFixedPoint = 32767;
		}
		if (gain < 0.0) {
			gainAbsFixedPoint = 32768 - gainAbsFixedPoint;
			if (gainAbsFixedPoint > 32767) {
				gainAbsFixedPoint = 32767;
			}
		}
		uint16_t gain16bit = static_cast<uint16_t>(((gain < 0.0) ? 32768U : 0U) + gainAbsFixedPoint);
		okb.setWireIn(WireIn::AdcConfigWord, gain16bit);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::AdcVoltageMultiplierLoad, port);
	}

	/** \brief Sets the scale factor for ADC control of current clamps.
	*
	*  \param[in] port    SPI port affected.
	*  \param[in] value   16-bit value corresponding to signed fixed-point scale factor.
	*/
	void Board::setCurrentMultiplier(int port, uint16_t value) {
		lock_guard<mutex> lockio(commandMutex);
		okb.setWireIn(WireIn::AdcConfigWord, value);
		okb.updateWiresIn();
		okb.activateTriggerIn(Triggers::AdcCurrentMultiplierLoad, port);
	}

	void Board::enableAdcControl(bool enable, int chip, int channel) {
		adcClampControlEnable[chip][channel] = enable;
		updateAdcClampControl();
	}

	void Board::selectAdcControl(int adc, int chip, int channel) {
		adcClampControlSelect[chip][channel] = adc;
		updateAdcClampControl();
	}

	void Board::updateAdcClampControl() {
		lock_guard<mutex> lockio(commandMutex);

		okb.setWireIn(WireIn::AdcSelectEnChip0,
			(adcClampControlEnable[0][3] << 15) + (adcClampControlSelect[0][3] << 12) +
			(adcClampControlEnable[0][2] << 11) + (adcClampControlSelect[0][2] << 8)  +
			(adcClampControlEnable[0][1] << 7) +  (adcClampControlSelect[0][1] << 4)  +
			(adcClampControlEnable[0][0] << 3) +  (adcClampControlSelect[0][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip1,
			(adcClampControlEnable[1][3] << 15) + (adcClampControlSelect[1][3] << 12) +
			(adcClampControlEnable[1][2] << 11) + (adcClampControlSelect[1][2] << 8) +
			(adcClampControlEnable[1][1] << 7) +  (adcClampControlSelect[1][1] << 4) +
			(adcClampControlEnable[1][0] << 3) +  (adcClampControlSelect[1][0] << 0));
		okb.setWireIn(WireIn::AdcSelectEnChip2,
			(adcClampControlEnable[2][3] << 15) + (adcClampControlSelect[2][3] << 12) +
			(adcClampControlEnable[2][2] << 11) + (adcClampControlSelect[2][2] << 8) +
			(adcClampControlEnable[2][1] << 7) +  (adcClampControlSelect[2][1] << 4) +
			(adcClampControlEnable[2][0] << 3) +  (adcClampControlSelect[2][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip3,
			(adcClampControlEnable[3][3] << 15) + (adcClampControlSelect[3][3] << 12) +
			(adcClampControlEnable[3][2] << 11) + (adcClampControlSelect[3][2] << 8) +
			(adcClampControlEnable[3][1] << 7) +  (adcClampControlSelect[3][1] << 4) +
			(adcClampControlEnable[3][0] << 3) +  (adcClampControlSelect[3][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip4,
			(adcClampControlEnable[4][3] << 15) + (adcClampControlSelect[4][3] << 12) +
			(adcClampControlEnable[4][2] << 11) + (adcClampControlSelect[4][2] << 8) +
			(adcClampControlEnable[4][1] << 7) +  (adcClampControlSelect[4][1] << 4) +
			(adcClampControlEnable[4][0] << 3) +  (adcClampControlSelect[4][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip5,
			(adcClampControlEnable[5][3] << 15) + (adcClampControlSelect[5][3] << 12) +
			(adcClampControlEnable[5][2] << 11) + (adcClampControlSelect[5][2] << 8) +
			(adcClampControlEnable[5][1] << 7) +  (adcClampControlSelect[5][1] << 4) +
			(adcClampControlEnable[5][0] << 3) +  (adcClampControlSelect[5][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip6,
			(adcClampControlEnable[6][3] << 15) + (adcClampControlSelect[6][3] << 12) +
			(adcClampControlEnable[6][2] << 11) + (adcClampControlSelect[6][2] << 8) +
			(adcClampControlEnable[6][1] << 7) +  (adcClampControlSelect[6][1] << 4) +
			(adcClampControlEnable[6][0] << 3) +  (adcClampControlSelect[6][0] << 0) );
		okb.setWireIn(WireIn::AdcSelectEnChip7,
			(adcClampControlEnable[7][3] << 15) + (adcClampControlSelect[7][3] << 12) +
			(adcClampControlEnable[7][2] << 11) + (adcClampControlSelect[7][2] << 8) +
			(adcClampControlEnable[7][1] << 7) +  (adcClampControlSelect[7][1] << 4) +
			(adcClampControlEnable[7][0] << 3) +  (adcClampControlSelect[7][0] << 0) );
		okb.updateWiresIn();
	}

	bool Board::expanderBoardPresent() {
		return expanderBoardDetected;
	}

	int Board::expanderBoardId() {
		return expanderBoardIdNumber;
	}

	void Board::enableDigitalMarker(int port, bool enabled) {
		digOutEnabled[port] = enabled;
		updateDigOutConfig();
	}

	void Board::setDigitalMarkerDestination(int port, int digOut) {
		digOutDestination[port] = digOut;
		updateDigOutConfig();
	}

	bool Board::digitalMarkerEnabled(int port) {
		return digOutEnabled[port];
	}

	int Board::digitalMarkerDestination(int port) {
		return digOutDestination[port];
	}

	void Board::updateDigOutConfig() {
		lock_guard<mutex> lockio(commandMutex);

		okb.setWireIn(WireIn::DigOutEnable,
			(digOutEnabled[7] << 7) + (digOutEnabled[6] << 6) + (digOutEnabled[5] << 5) + (digOutEnabled[4] << 4) +
			(digOutEnabled[3] << 3) + (digOutEnabled[2] << 2) + (digOutEnabled[1] << 1) + (digOutEnabled[0] << 0));
		okb.setWireIn(WireIn::DigOutSelectA,
			(digOutDestination[3] << 12) + (digOutDestination[2] << 8) + (digOutDestination[1] << 4) + (digOutDestination[0] << 0));
		okb.setWireIn(WireIn::DigOutSelectB,
			(digOutDestination[7] << 12) + (digOutDestination[6] << 8) + (digOutDestination[5] << 4) + (digOutDestination[4] << 0));
		okb.updateWiresIn();
	}

	void Board::enableAllPorts() {
		lock_guard<mutex> lockio(commandMutex);

		okb.setWireIn(WireIn::DisablePorts, 0);
		okb.updateWiresIn();
	}

	void Board::enableOnePortOnly(int port) {
		lock_guard<mutex> lockio(commandMutex);

		okb.setWireIn(WireIn::DisablePorts, 0xff ^ (1 << port));
		okb.updateWiresIn();
	}
}