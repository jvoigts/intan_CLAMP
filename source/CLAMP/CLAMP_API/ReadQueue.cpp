#include "ReadQueue.h"
#include "USBPacket.h"
#include "Constants.h"
#include "ChipProtocol.h"
#include "common.h"
#include "Board.h"

using std::unique_ptr;
using std::vector;
using std::wstring;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::SignalProcessing;

namespace CLAMP {
    /// \cond private
    void ChannelIndexData::clear() {
        mosi.erase(mosi.begin(), mosi.end());
        miso.erase(miso.begin(), miso.end());
    }

    //-----------------------------------------------------------------------------------------------------
    ChannelData::ChannelData() :
		// TO FIX: Does not work correctly for 100 kS/s!  Filters at 12500 Hz instead of 25000 Hz in this case.
        muxFilter(new NthOrderBesselLowPassFilter(4, 25000, 1.0 / 200000)),
        voltageFilter(new NthOrderBesselLowPassFilter(4, 25000, 1.0 / 200000)), /* TEMP */
        currentFilter(new NthOrderBesselLowPassFilter(4, 25000, 1.0 / 200000))
    {
        clear(false); 
    }

    void ChannelData::clear(bool filtersToo) {
        index = 0;
        raw.erase(raw.begin(), raw.end());
        mux.erase(mux.begin(), mux.end());
        voltages.erase(voltages.begin(), voltages.end());
        currents.erase(currents.begin(), currents.end());
		clampVoltages.erase(clampVoltages.begin(), clampVoltages.end());
		clampCurrents.erase(clampCurrents.begin(), clampCurrents.end());

        if (filtersToo) {
            muxFilter->reset();
            voltageFilter->reset();
            currentFilter->reset();
        }
    }

    void ChannelData::push1(int32_t value, double muxVoltage, double voltage, double current, double clampVoltage, double clampCurrent, unsigned int channelRepetition) {
        if (channelRepetition == 1) {
            raw.push_back(value);
            mux.push_back(muxVoltage);
            voltages.push_back(voltage);
            currents.push_back(current);
			clampVoltages.push_back(clampVoltage);
			clampCurrents.push_back(clampCurrent);
        }
        else {
			// TEMP - eliminate 25 kHz Bessel filters
            double filteredMuxVoltage = muxFilter->filterOne(muxVoltage);
            double filteredVoltage = voltageFilter->filterOne(voltage);
            double filteredCurrent = currentFilter->filterOne(current);
			//double filteredMuxVoltage = muxVoltage;
			//double filteredVoltage = voltage;
			//double filteredCurrent = current;

			// Downsample by a factor of channelRepetition
            if (index == 0) {
                raw.push_back(value);
                mux.push_back(filteredMuxVoltage);
                voltages.push_back(filteredVoltage);
                currents.push_back(filteredCurrent);
				clampVoltages.push_back(clampVoltage);
				clampCurrents.push_back(clampCurrent);
            }
            index++;
            if (index == channelRepetition) {
                index = 0;
            }
        }
    }

    void ChannelData::pushChannelData(ClampController& controller, ChipChannel chipChannel, const USBPerChannel& usbchannel, unsigned int channelRepetition) {
        Channel& channel = controller.getChannel(chipChannel);
        Mux& mux = controller.mux;
        if (channel.getEnable()) {
            double rF = channel.getFeedbackResistance();

            int32_t value = INVALID_MUX_VALUE;
            double muxVoltage = std::numeric_limits<double>::quiet_NaN();
            double voltage = std::numeric_limits<double>::quiet_NaN();
            double current = std::numeric_limits<double>::quiet_NaN();
			double clampVoltage = std::numeric_limits<double>::quiet_NaN();
			double clampCurrent = std::numeric_limits<double>::quiet_NaN();

            if (usbchannel.MOSI.M == MuxSelection::Temperature) {
                value = mux.getValue(chipChannel, usbchannel.convertValue, 0);
                muxVoltage = mux.toVoltage(chipChannel, value);
            }
            else {
                bool isVoltage = (usbchannel.MOSI.M % 2) == 1;

                if (isVoltage) {
                    value = mux.getValue(chipChannel, usbchannel.convertValue, channel.voltageAmpResidual);
                    muxVoltage = mux.toVoltage(chipChannel, value);
                    voltage = muxVoltage / 8.0;
					clampCurrent = ((usbchannel.MOSI.D & 128) ? 1.0 : -1.0) * (usbchannel.MOSI.D & 127) * channel.recallCurrentStep();
                } else {
                    value = mux.getValue(chipChannel, usbchannel.convertValue, channel.differenceAmpResidual);
                    muxVoltage = mux.toVoltage(chipChannel, value);
                    current = muxVoltage / 10.0 / rF;
					clampVoltage = ((usbchannel.MOSI.D & 256) ? 1.0 : -1.0) * (usbchannel.MOSI.D & 255) * channel.getVoltageClampStep();
                }
            }
            push1(value, muxVoltage, voltage, current, clampVoltage, clampCurrent, channelRepetition);
        }
    }
    /// \endcond

    //-----------------------------------------------------------------------------------------------------
    /// Constructor
    ReadQueue::ReadQueue(std::vector<ChannelNumber>& channels_, ClampController& controller_) :
        controller(controller_),
        channels(channels_)
    {
        adcs.reserve(8);
        for (unsigned int adc = 0; adc < 8; adc++) {
            adcs.push_back(vector<uint16_t>());
        }
    }

    ReadQueue::~ReadQueue() {
    }

    /** \brief Convert data from raw bytes to structured channel/chip data.
     *
     *  Called by Board::read; you probably wouldn't need to call it directly.
     *
     *  \param[in] usbBuffer   Raw USB data
     *  \param[in] numPackets  The number of packets to parse
     */
    void ReadQueue::parse(unsigned char* usbBuffer, unsigned int numPackets) {
        unsigned char* p = usbBuffer;
        for (unsigned int i = 0; i < numPackets; i++) {
            unique_ptr<USBPacket> packet(new USBPacket());
            p = packet->read(p, controller.getBoard(), channels);
            push(packet);
        }
    }

    void ReadQueue::push(unique_ptr<USBPacket>& packet) {

        // Populate the converted values; these are pipelined one step behind the MOSI commands
        for (unsigned int chip = 0; chip < MAX_NUM_CHIPS; chip++) {
            USBPerChannel* p2 = nullptr;
            if (onDeck.get() != nullptr) {
                p2 = &onDeck->chip[chip].channel[channels.size() - 1];
            }

            USBPerChip& usbChip = packet->chip[chip];
            for (unsigned int channel = 0; channel < channels.size(); channel++)
            {
                USBPerChannel& usbChannel = usbChip.channel[channel];
                const MOSICommand& mosi = usbChannel.MOSI;
                const MISOReturn&  miso = usbChannel.MISO;
                // Decode the miso values
                switch (mosi.C) {
                case CONVERT:
                case WRITE_AND_CONVERT:
                    // Conversion is pipelined by 1, so if we're converting now, it's actually the 
                    // mux from the previous command; so store it there
                    if (p2) {
                        p2->nextIsConvert = true;
                        p2->convertValue = miso;
                    }
                    break;
                }
                p2 = &usbChannel;
            }
        }

        // Now push the packet onto the queue
        pushLast();
        onDeck = std::move(packet);
    }

    /** \brief Clear data from the read queue.
     *
     *  We read data up to 4x faster than it's returned to the user.  For instance, we may sample at 200 kHz, then
     *  pass the data through an anti-aliasing filter and downsample to 50 kHz.  
     *  Depending on *why* you're calling clear(), you may or may not want to reset the filters.
     *
     *  \li If you're reading continuously in chunks and you finish a chunk and call clear, you don't need to reset the filters.
     *      (Time continues, and the filters are still good.)
     *  \li If you stop reading and start again, you do need to reset the filters.
     *
     *  \param[in] filtersToo   Resets filters if true.  See above.
     */
    void ReadQueue::clear(bool filtersToo) {
        for (unsigned int chip = 0; chip < MAX_NUM_CHIPS; chip++) {
            for (unsigned int channel = 0; channel < MAX_NUM_CHANNELS; channel++) {
                rawDataIndexed[chip][channel].clear();
                rawData[chip][channel].clear(filtersToo);
            }
        }

        timestamps.erase(timestamps.begin(), timestamps.end());
		digIns.erase(digIns.begin(), digIns.end());
		digOuts.erase(digOuts.begin(), digOuts.end());

        for (auto& element : adcs) {
            element.erase(element.begin(), element.end());
        }
    }

    /** \brief Pushes the on-deck member of the read queue into the data set.
     *
     *  Due to the nature of the CLAMP chip, data returns are delayed by one timestep from commands.  So if you send a
     *  command of "Convert," you won't get the converted value back until the next command.  This interacts with multi-channel
     *  reading: if you issue all four channels of "Convert" commands, you'll get three back in the current timestep, and the
     *  last one back in the next timestep.
     *
     *  To handle this, we keep an "on-deck" data packet.  Whenever we read another data packet, we use the commands from the "on-deck"
     *  packet and the data from the "on-deck" and the new packet, and push all that data into the appropriate variables.  We then
     *  promote the new data packet to the "on-deck" position, and repeat as data come in.
     *
     *  At the very end of a long read, we'll still have an "on-deck" packet around, and we need to flush that out, even though we'll
     *  be missing convert values from the very last command.  This function causes that flush to occur.
     */
    void ReadQueue::pushLast() {
        if (onDeck.get() != nullptr) {
            timestamps.push_back(onDeck->timestamp);
            for (unsigned int adc = 0; adc < 8; adc++) {
                if (controller.getBoard().adcTransfer[adc]) {
                    adcs[adc].push_back(onDeck->adcs[adc]);
                }
            }
			digIns.push_back(onDeck->digIn);
			// DEBUGOUT("DigIns = " << onDeck->digIn << endl);
			digOuts.push_back(onDeck->digOut);

            for (unsigned int chip = 0; chip < MAX_NUM_CHIPS; chip++) {
                for (unsigned int channelIndex = 0; channelIndex < channels.size(); channelIndex++) {
                    ChipChannel chipChannelUSB(chip, channelIndex);
                    const USBPerChannel& usbchannel = onDeck->extractChannel(chipChannelUSB);
                    ChannelIndexData& cid = getIndexedChannelData(chipChannelUSB);
                    cid.mosi.push_back(usbchannel.MOSI);
                    cid.miso.push_back(usbchannel.MISO);

                    unsigned int channelNumber;
                    switch (usbchannel.MOSI.M)
                    {
                    case MuxSelection::Unit0Current:
                    case MuxSelection::Unit0Voltage:
                        channelNumber = 0;
                        break;
                    case MuxSelection::Unit1Current:
                    case MuxSelection::Unit1Voltage:
                        channelNumber = 1;
                        break;
                    case MuxSelection::Unit2Current:
                    case MuxSelection::Unit2Voltage:
                        channelNumber = 2;
                        break;
                    case MuxSelection::Unit3Current:
                    case MuxSelection::Unit3Voltage:
                        channelNumber = 3;
                        break;
                    case MuxSelection::Temperature:
                        channelNumber = channels[channelIndex];
                        break;
                    default:
                        // This is an error case we should never hit.  If we do (probably due to flaky transmission), we should get not crash below.
                        continue;
                        break;
                    }

                    ChipChannel chipChannel(chip, channelNumber);
                    ChannelData& cd = getChannelData(chipChannel);
                    cd.pushChannelData(controller, chipChannel, usbchannel, controller.getBoard().channelRepetition);
                }
            }
            onDeck.reset();
        }
    }

    ChannelData& ReadQueue::getChannelData(const ChipChannel& chipChannel) {
        return rawData[chipChannel.chip][chipChannel.channel];
    }

    ChannelIndexData& ReadQueue::getIndexedChannelData(const ChipChannel& chipChannel) {
        return rawDataIndexed[chipChannel.chip][chipChannel.channel];
    }

    /** \brief Get raw MOSI (i.e., command) data from the read data
     *
     *  Primarily used internally.  May also be useful for debugging command/return sequences.
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of MOSI commands that have been sent to the chip
     */
    const vector<MOSICommand>& ReadQueue::getMOSI(const ChipChannel& chipChannel) {
        return rawDataIndexed[chipChannel.chip][chipChannel.channel].mosi;
    }

    /** \brief Get raw MISO (i.e., chip return) data from the read data
     *
     *  Primarily used internally.  May also be useful for debugging command/return sequences.
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of MISO values that have been returned from the chip
     */
    const vector<MISOReturn>& ReadQueue::getMISO(const ChipChannel& chipChannel) {
        return rawDataIndexed[chipChannel.chip][chipChannel.channel].miso;
    }

    /** \brief Get raw values from the mux
     *
     *  Primarily used internally.  Raw values have been adjusted for any residuals in the amplifiers,
     *  as described in the calibration section of the datasheet.
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of raw mux values that have been returned from the chip
     */
    const vector<int32_t>& ReadQueue::getRawData(const ChipChannel& chipChannel) {
        return rawData[chipChannel.chip][chipChannel.channel].raw;
    }

    /** \brief Get voltages at the mux
     *
     *  Primarily used internally.  Converting to voltages or currents at the *electrode* is done separately.
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of mux voltages that have been returned from the chip
     */
    const vector<double>& ReadQueue::getMuxData(const ChipChannel& chipChannel) {
        return rawData[chipChannel.chip][chipChannel.channel].mux;
    }

    /** \brief Get voltages measured at the electrode (current clamp mode)
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of measured voltages that have been returned from the chip
     */
    const vector<double>& ReadQueue::getMeasuredVoltages(const ChipChannel& chipChannel) {
        return rawData[chipChannel.chip][chipChannel.channel].voltages;
    }

    /** \brief Get currents measured at the electrode (voltage clamp mode)
     *
     *  \param[in] chipChannel  ChipChannel index
     *  \returns A vector of measured currents that have been returned from the chip
     */
    const vector<double>& ReadQueue::getMeasuredCurrents(const ChipChannel& chipChannel) {
		return rawData[chipChannel.chip][chipChannel.channel].currents;
    }

	/** \brief Get clamp voltages directly from the MOSI command stream (voltage clamp mode)
	*
	*  \param[in] chipChannel  ChipChannel index
	*  \returns A vector of clamp voltages according to the MOSI command stream
	*/
	const vector<double>& ReadQueue::getClampVoltages(const ChipChannel& chipChannel) {
		return rawData[chipChannel.chip][chipChannel.channel].clampVoltages;
	}

	/** \brief Get clamp currents directly from the MOSI command stream (current clamp mode)
	*
	*  \param[in] chipChannel  ChipChannel index
	*  \returns A vector of clamp currents according to the MOSI command stream
	*/
	const vector<double>& ReadQueue::getClampCurrents(const ChipChannel& chipChannel) {
		return rawData[chipChannel.chip][chipChannel.channel].clampCurrents;
	}


    /// Get the timestamps from the read data
    const vector<uint32_t>& ReadQueue::getTimeStamps() {
        return timestamps;
    }

	/// Get the digital inputs from the read data
	const vector<uint16_t>& ReadQueue::getDigIns() {
		return digIns;
	}

	/// Get the digital outputs from the read data
	const vector<uint16_t>& ReadQueue::getDigOuts() {
		return digOuts;
	}

    /** \brief Get a matrix of FPGA-board ADC values
     *
     *  Note that these are the values of the ADCs on the FPGA-board, not on the CLAMP chip.
     *
     *  For space reasons (i.e., we don't want to store arrays of doubles, which are 4 times bigger), 
     *  this returns the raw uint16_t values; those need to be converted to voltages where
     *  they are used.
     *
     *  \returns A vector of size 8 of vectors of measured ADC values returned from the FPGA board
     */
    const vector<vector<uint16_t>>& ReadQueue::getADCs() {
        return adcs;
    }


}
