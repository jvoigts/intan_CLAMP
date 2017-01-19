#pragma once

#include <vector>
#include <memory>
#include "Channel.h"
#include <queue>
#include <map>
#include "ChipProtocol.h"
#include "Constants.h"
#include "BesselFilter.h"

namespace CLAMP {
    /// \cond private
    class USBPerChannel;
    class USBPerChip;
    struct ChannelIndexData {
        std::vector<ChipProtocol::MOSICommand> mosi;
        std::vector<ChipProtocol::MISOReturn> miso;

        void clear();
    };

    struct ChannelData { // Data indexed by channel
        std::vector<int32_t> raw; // Raw numbers measured at the mux (corrected by any software correction)
        std::vector<double> mux; // Voltages measured at mux
        std::vector<double> voltages; // Voltages before voltage amplifier
        std::vector<double> currents; // Currents across feedback resistor
		std::vector<double> clampVoltages; // Clamp voltages, directly from MOSI command stream
		std::vector<double> clampCurrents; // Clamp currents, directly from MOSI command stream

        ChannelData();
        void clear(bool filtersToo);
        void pushChannelData(ClampConfig::ClampController& controller, ClampConfig::ChipChannel chipChannel, const USBPerChannel& usbchannel, unsigned int channelRepetition);
        void push1(int32_t value, double muxVoltage, double voltage, double current, double clampVoltage, double clampCurrent, unsigned int channelRepetition);

    private:
        unsigned int index;
        std::unique_ptr<SignalProcessing::Filter> muxFilter;
        std::unique_ptr<SignalProcessing::Filter> voltageFilter;
        std::unique_ptr<SignalProcessing::Filter> currentFilter;
    };
    /// \endcond

    class USBPacket;
    /** \brief Main class used in interpreting data returned from the Board.
     *
     *  Data comes in as raw bytes (ReadQueue::parse) method, and is converted into channel and chip
     *  data.  Whatever data is of interest can be queried via an appropriate get method.
     *
     *  Internally, this class handles the various transformations to make that work, but externally,
     *  it hides the grunginess and makes it easy to convert from raw bytes to the data of interest.
     */
    class ReadQueue {
    public:
        ReadQueue(std::vector<ChannelNumber>& channels_, ClampConfig::ClampController& controller_);
        ~ReadQueue();

        void parse(unsigned char* usbBuffer, unsigned int numPackets);
        void clear(bool filtersToo = true);
        void pushLast();

        const std::vector<uint32_t>& getTimeStamps();
		const std::vector<uint16_t>& getDigIns();
		const std::vector<uint16_t>& getDigOuts();
        const std::vector<ChipProtocol::MOSICommand>& getMOSI(const ClampConfig::ChipChannel& chipChannel);
        const std::vector<ChipProtocol::MISOReturn>& getMISO(const ClampConfig::ChipChannel& chipChannel);
        const std::vector<int32_t>& getRawData(const ClampConfig::ChipChannel& chipChannel);
        const std::vector<double>& getMuxData(const ClampConfig::ChipChannel& chipChannel);
        const std::vector<double>& getMeasuredVoltages(const ClampConfig::ChipChannel& chipChannel);
        const std::vector<double>& getMeasuredCurrents(const ClampConfig::ChipChannel& chipChannel);
		const std::vector<double>& getClampVoltages(const ClampConfig::ChipChannel& chipChannel);
		const std::vector<double>& getClampCurrents(const ClampConfig::ChipChannel& chipChannel);
		const std::vector<std::vector<uint16_t>>& getADCs();

    private:
        ClampConfig::ClampController& controller;
        std::vector<ChannelNumber>& channels;
        std::unique_ptr<USBPacket> onDeck;
        ChannelIndexData rawDataIndexed[MAX_NUM_CHIPS][MAX_NUM_CHANNELS]; // Data stored indexed by channel index
        ChannelData rawData[MAX_NUM_CHIPS][MAX_NUM_CHANNELS]; // Data stored indexed by actual channel
        std::vector<std::vector<uint16_t>> adcs;
        std::vector<uint32_t> timestamps;
		std::vector<uint16_t> digIns;
		std::vector<uint16_t> digOuts;

        ChannelData& getChannelData(const ClampConfig::ChipChannel& chipChannel);
        ChannelIndexData& getIndexedChannelData(const ClampConfig::ChipChannel& chipChannel);
        void push(std::unique_ptr<USBPacket>& packet);
    };
}
