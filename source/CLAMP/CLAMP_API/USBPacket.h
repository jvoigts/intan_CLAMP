#pragma once

#include <cstdint>
#include <vector>
#include "ChipProtocol.h"
#include "Constants.h"
#include "Channel.h"

namespace CLAMP {
    /// \cond private
    // These classes are used internally in parsing data within ReadQueue.  They aren't part of the external API.
    class Channel;
    class Chip;
    class Board;
    namespace ClampConfig {
        struct ChipChannel;
    }

    class USBPerChannel {
    public:
        enum ConvertType {
            NONE,
            CURRENT,
            VOLTAGE
        };

        bool     enabled;
        ChipProtocol::MOSICommand MOSI;
        ChipProtocol::MISOReturn MISO;   // Raw MISO value

        // Processed values
        bool nextIsConvert;
        uint32_t convertValue;

        USBPerChannel() { reset();  }

        void reset();
        unsigned char* read(unsigned char* input, const Channel& channel);
    };

    class USBPerChip {
    public:
        USBPerChannel channel[MAX_NUM_CHANNELS];

        USBPerChip() { reset();  }

        void reset();

        void setMuxConvertValue(ChipProtocol::MuxSelection mux, uint32_t value);
    };

    class USBPacket {
    public:
        uint32_t timestamp;
        USBPerChip chip[MAX_NUM_CHIPS];
        uint16_t adcs[8];
        uint16_t digIn;
        uint16_t digOut;
        uint16_t filler[3];

        void reset();
        unsigned char* read(unsigned char* input, const Board& board, std::vector<ChannelNumber>& channels);

        USBPerChannel& extractChannel(const ClampConfig::ChipChannel& chipChannel);
        USBPerChip& extractChip(const ClampConfig::ChipChannel& chipChannel);

        static unsigned int getPerChannelSize();
        static unsigned int getChannelIndependentSize(const Board& board);
    };
    /// \endcond
}
