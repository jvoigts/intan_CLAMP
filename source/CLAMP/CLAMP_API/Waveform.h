#pragma once

#include <cstdint>
#include <vector>
#include "Registers.h"

namespace CLAMP {
    class Board;

    namespace WaveformControl {
        /// \cond private

        /* The WaveformRAM class (below) is the in-memory storage of the data that goes into the Waveform RAM on the chip.
         *
         * If you're using multiple channels and those channels use the same waveforms, there's no need to store multiple
         * copies of the waveform in the global Waveform RAM - the various channels can refer to the same common copy.
         * To handle this, we have WaveformRAMCell, which is one memory position in Waveform RAM, but also keeps a reference
         * count of how many channels depend on that data.  Once the count goes to 0, that part of RAM is available, and may
         * be used by any channel.
         *
         * The other piece is the WaveformExtent - on the one hand, it's the object you hand off to a calling class that
         * contains the start and end addresses of the waveform.  On the other hand, it has a role in the reference counting:
         * the calling code keeps the instance of WaveformExtent around, and as long as it exists, the locations in
         * WaveformRAM are held.  When the WaveformExtent object goes out of scope, it decrements the reference count of
         * the part of the Waveform RAM that it marked.
         */

        struct WaveformRAMCell {
            uint32_t content;
            uint8_t  numRefs;
            bool     dirty;

            WaveformRAMCell() : content(0), numRefs(0), dirty(false) {}
        };

        class WaveformRAM;
        class WaveformExtent {
        public:
            uint32_t end : 15;
            uint32_t start : 15;

            WaveformExtent(WaveformRAM& ram, uint16_t start_, uint16_t end_) : end(end_), start(start_), owningRAM(ram) {}
            ~WaveformExtent();

        private:
            WaveformRAM& owningRAM;
        };

        class WaveformRAM {
        public:
            WaveformRAM(Board& clampBoard) : m_clampBoard(clampBoard), ram(1 << 15) {}

            std::shared_ptr<WaveformExtent> write(const std::vector<uint32_t>& data);
            // std::shared_ptr<WaveformExtent> writeDigitalOutputs(const std::vector<uint16_t>& data, uint16_t commandStart, uint16_t& offset);
            void clear(const WaveformExtent& command);
            void toFPGA();

        private:
            Board& m_clampBoard;
            std::vector<WaveformRAMCell> ram;
        };
        /// \endcond

        /// Class with static members to make some common waveforms
        class CommonWaveForms {
        public:
            static std::vector<WaveformCommand> readROMCommands();
            static std::vector<WaveformCommand> readRAMCommands();
        };
    }
}
