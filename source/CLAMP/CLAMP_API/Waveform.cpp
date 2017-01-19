#include "Waveform.h"
#include "Board.h"
#include "RAM.h"
#include "common.h"
#include "Channel.h"
#include "Registers.h"
#include <stdexcept>
#include "ChipProtocol.h"
#include "WaveformCommand.h"

using std::vector;
using std::runtime_error;
using std::shared_ptr;
using CLAMP::Registers::Unit;
using namespace CLAMP::ChipProtocol;

namespace CLAMP {
    namespace WaveformControl {
        /// \cond private
        //----------------------------------------------------------------------------------------
        WaveformExtent::~WaveformExtent() {
            owningRAM.clear(*this);
        }

        //----------------------------------------------------------------------------------------
        shared_ptr<WaveformExtent> WaveformRAM::write(const vector<uint32_t>& data) {
            if (data.empty()) {
                throw runtime_error("Error - you shouldn't call this without any data");
            }

            // Note: this isn't the world's fastest comparison algorithm.  But the faster ones are much more complicated.
            unsigned int rampos = 0;
            bool good = false;
            while (!good && (rampos + data.size() < ram.size())) {
                bool conflict = false;
                for (unsigned int i = 0; !conflict && i < data.size(); i++) {
                    conflict = (ram[rampos + i].numRefs != 0) &&
                        (ram[rampos + i].content != data[i]);
                }
                if (conflict) {
                    rampos++;
                }
                else {
                    good = true;
                }
            }

            if (!good) {
                throw runtime_error("Out of Waveform RAM");
            }

            for (unsigned int i = 0; i < data.size(); i++) {
                bool same = (ram[rampos + i].content == data[i]);
                ram[rampos + i].content = data[i];
                ram[rampos + i].dirty = (ram[rampos + i].numRefs == 0) || !same;
                ram[rampos + i].numRefs++;
            }

            toFPGA();

            shared_ptr<WaveformExtent> current(new WaveformExtent(*this, rampos, rampos + data.size() - 1));
            return current;
        }

        void WaveformRAM::toFPGA() {
            const unsigned int RAMSIZE = ram.size();
            for (unsigned int startPos = 0; startPos < RAMSIZE;) {
                if (!ram[startPos].dirty) {
                    startPos++;
                }
                else {
                    unsigned int endPosPlus1;
                    for (endPosPlus1 = startPos; (endPosPlus1 < RAMSIZE) && (ram[endPosPlus1].dirty); endPosPlus1++) {
                        ; // Do nothing
                    }

                    vector<uint32_t> tmp;
                    for (unsigned int pos = startPos; pos < endPosPlus1; pos++) {
                        tmp.push_back(ram[pos].content);
                        ram[pos].dirty = false;
                    }

                    WaveformRAMAddress addr(startPos);
                    m_clampBoard.writeRAM(addr, tmp);

                    startPos = endPosPlus1 + 1;
                }
            }
        }

        void WaveformRAM::clear(const WaveformExtent& extent) {
            for (unsigned int position = extent.start; position <= extent.end; position++) {
                if (ram[position].numRefs == 0) {
                    throw runtime_error("Trying to clear to a part of RAM that's already clear.");
                }
                ram[position].numRefs--;
            }
        }
        /// \endcond

        //----------------------------------------------------------------------------------------
        /** \brief Waveform that reads the ROM registers
         *  \returns The waveform.
         */
        vector<WaveformCommand> CommonWaveForms::readROMCommands() {
            vector<WaveformCommand> data = {
                NonrepeatingCommand::create(READ, None, 0xF7, 0),  // Read 15,7 (I in INTAN )
                NonrepeatingCommand::create(READ, None, 0xF8, 0),
                NonrepeatingCommand::create(READ, None, 0xF9, 0),
                NonrepeatingCommand::create(READ, None, 0xFA, 0),
                NonrepeatingCommand::create(READ, None, 0xFB, 0),
                NonrepeatingCommand::create(READ, None, 0xFC, 0),
                NonrepeatingCommand::create(READ, None, 0xFD, 0),
                NonrepeatingCommand::create(READ, None, 0xFE, 0),
                NonrepeatingCommand::create(READ, None, 0xFF, 0),
            };

            return data;
        }

        /** \brief Waveform that reads the RAM registers
         *  \returns The waveform.
         */
        vector<WaveformCommand> CommonWaveForms::readRAMCommands() {
            vector<WaveformCommand> data;

            for (unsigned int unit = 0; unit < MAX_NUM_CHANNELS; unit++) {
                for (unsigned int registerIndex = 0; registerIndex <= 13; registerIndex++) {
                    data.push_back(NonrepeatingCommand::create(READ, None, ((unit << 4) | registerIndex), 0));
                }
            }

            for (unsigned int registerIndex = 0; registerIndex <= 3; registerIndex++) {
                data.push_back(NonrepeatingCommand::create(READ, None, ((Unit::Global << 4) | registerIndex), 0));
            }

            return data;
        }
    }
}
