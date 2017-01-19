#pragma once

#include <vector>
#include "Channel.h"
#include "Registers.h"
#include "Constants.h"
#include "ChipProtocol.h"

namespace CLAMP {
    class Board;
    /** \brief In-memory representation one chip attached to a CLAMP evaluation board.
     *
     *  A CLAMP evaluation board can contain multiple chips.  This in-memory data structure represents one chip and contains data 
     *  related to its operations.  Many operations are accomplished via ClampConfig::ClampController, rather than here directly.
     *
     *  A chip has Registers::GlobalRegisters (member variable chipRegisters), which control its operation.  It also has a list of 
     *  channels, and a couple chip-wide functions.
     *
     *  See below for details.
     */
    class Chip {
    public:
        /// Index of this chip (e.g., chip 0 has index = 0).  [0-3] currently.
        unsigned int index;
        /// Reference to the board that contains this chip, for convenience
        Board& board;
        /// Channels on this chip (4 for CLAMP).
        Channel* channel[MAX_NUM_CHANNELS];
        /** \brief True if the chip exists.
         *
         *  The Board has in-memory data structures for all the chips that can be attached to an FPGA board (e.g., eight of them).
         *  However, only some of those may actually be present.  This variable tracks which are and aren't.  It's set in
         *  Board::initialize().
         */
        bool present;
		/** \brief Number of channels on the chip.
		*
		*  This variable tracks which type of chip is connected (a 1-channel CLAMP1 or a 4-channel CLAMP4).  It's set in
		*  Board::initialize().
		*/
		int numChannels;
        /** \brief In-memory representation of on-chip registers.
         *
         *  Frequently, these registers are not manipulated directly here, but rather via the ClampConfig::ClampController and its members.
         *  Those classes also have functionality that pushes these register values to the chips, rather than just changing them in memory.
         */
        Registers::GlobalRegisters chipRegisters;

        Chip(Board& board_, unsigned int index_);
        ~Chip();

        // Virtual Register
        void setCableDelay(uint8_t value);

        void readBackRegister(const ChipProtocol::MOSICommand& mosi, const ChipProtocol::MISOReturn& miso);
        std::vector<WaveformControl::WaveformCommand> writeAllRegistersCommands();

        /// Value, in &Omega;s of RCal1 attached to this chip
        double RCal1;
        /// Value, in &Omega;s of RCal2 attached to this chip
        double RCal2;

    private:
        void writeVirtualRegister(uint8_t address, uint16_t value);
        Registers::Register& getRegister(uint8_t address);
    };
}
