#include "USBPacket.h"
#include "Chip.h"
#include "Channel.h"
#include "Board.h"
#include "common.h"

using std::vector;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::ClampConfig;

namespace CLAMP {
    /// \cond private
    void USBPerChannel::reset() {
        enabled = false;
        MOSI = MOSICommand();
        MISO = MISOReturn();
        nextIsConvert = false;
        convertValue = 0;
    }

    unsigned char* USBPerChannel::read(unsigned char* input, const Channel& channel) {
        enabled = channel.getEnable();
        if (enabled) {
            MOSI = *reinterpret_cast<MOSICommand*>(input);
            input += sizeof(MOSICommand);
            MISO = *reinterpret_cast<MISOReturn*>(input);
            input += sizeof(MISOReturn);
        }
        return input;
    }

    //------------------------------------------------------------------------------
    void USBPerChip::reset() {
        for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
            channel[i].reset();
        }
    }

    //------------------------------------------------------------------------------

    unsigned int USBPacket::getPerChannelSize() {
        return sizeof(MOSICommand) + sizeof(MISOReturn);
    }

    unsigned int USBPacket::getChannelIndependentSize(const Board& board) {
        unsigned int numADCs = 0;
        for (bool b : board.adcTransfer) {
            if (b) {
                numADCs++;
            }
        }
        unsigned int timestampBytes = sizeof(uint32_t);
        unsigned int adcBytes = numADCs * sizeof(uint16_t);
        unsigned int digitalBytes = (board.diginTransfer ? sizeof(uint16_t) : 0) + (board.digoutTransfer ? sizeof(uint16_t) : 0);
        unsigned int totalBytes = timestampBytes + adcBytes + digitalBytes;
        if (totalBytes % 8 == 0) {
            return totalBytes;
        }
        else {
            return (totalBytes/8 + 1) * 8;
        }
    }

    void USBPacket::reset() {
        for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
            chip[chipIndex].reset();
        }
    }

    unsigned char* USBPacket::read(unsigned char* input, const Board& board, std::vector<ChannelNumber>& channels) {
        unsigned char* p0 = input;
        timestamp = *reinterpret_cast<uint32_t*>(input);
        input += sizeof(uint32_t);

        for (unsigned int channelIndex = 0; channelIndex < channels.size(); channelIndex++) {
            for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
                const Channel& channel = *board.chip[chipIndex]->channel[channels[channelIndex]];
                input = chip[chipIndex].channel[channelIndex].read(input, channel);
            }
        }

        for (unsigned int adc = 0; adc < 8; adc++) {
            if (board.adcTransfer[adc]) {
                adcs[adc] = *reinterpret_cast<uint16_t*>(input);
                input += sizeof(uint16_t);
            }
        }
        if (board.diginTransfer) {
            digIn = *reinterpret_cast<uint16_t*>(input);
            input += sizeof(uint16_t);
        }
        if (board.digoutTransfer) {
            digOut = *reinterpret_cast<uint16_t*>(input);
            input += sizeof(uint16_t);
        }
        unsigned int numBytes = (input - p0);
        unsigned int fillerBytes = 8 - (numBytes % 8);
        if (fillerBytes == 8) {
            fillerBytes = 0;
        }
        for (unsigned int i = 0; i < fillerBytes / 2; i++) {
            filler[i] = *reinterpret_cast<uint16_t*>(input);
            input += sizeof(uint16_t);
        }

        return input;
    }

    USBPerChannel& USBPacket::extractChannel(const ChipChannel& chipChannel) {
        return chip[chipChannel.chip].channel[chipChannel.channel];
    }

    USBPerChip& USBPacket::extractChip(const ChipChannel& chipChannel) {
        return chip[chipChannel.chip];
    }
    /// \endcond
}
