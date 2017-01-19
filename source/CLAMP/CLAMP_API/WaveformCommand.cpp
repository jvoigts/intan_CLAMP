#include "WaveformCommand.h"
#include "common.h"
#include "Channel.h"
#include "ReadQueue.h"
#include <stdexcept>

using std::vector;
using std::pair;
using std::unique_ptr;
using std::runtime_error;
using CLAMP::Registers::Register9;
using namespace CLAMP::ChipProtocol;

namespace CLAMP {
    namespace WaveformControl {
        /** \brief Create a NonrepeatingCommand with the appropriate parts.
         *
         * \param[in] C_    Command type (see C member)
         * \param[in] M_    %Mux (see M member)
         * \param[in] A_    Address (see A member)
         * \param[in] D_    Data (see D member).  Only the bottom 9 bits are used.
         * \returns The filled-in NonrepeatingCommand.
         */
        NonrepeatingCommand NonrepeatingCommand::create(ChipProtocol::Commands C_, MuxSelection M_, uint8_t A_, uint16_t D_) {
            NonrepeatingCommand result;
            result.A = CheckBits(A_, 8);
            result.M = CheckBits(static_cast<uint8_t>(M_), 6);
            result.C = CheckBits(static_cast<uint8_t>(C_), 2);
            result.D = CheckBits(D_, 9);
			result.digOut = false;
			result.markerOut = false;
            result.unused = CheckBits(0, 4);
            result.repeating = false;

            return result;
        }

        //----------------------------------------------------------------------------------------

        /** \brief Create a RepeatingCommand with the appropriate parts.
         *
         * \param[in] write             Write type (see \ref registerToWrite member)
         * \param[in] read              Read type (see \ref muxToRead member)
         * \param[in] valueSource       Write literal or ADC (see \ref valueSource member)
         * \param[in] longTimescale     True to use long time scale.  See \ref longTimescale member.
         * \param[in] L                 Literal; used if valueSource is \ref LITERAL.
         * \param[in] T                 Time constant.  See \ref longTimescale and \ref T members.
         * \returns The filled-in RepeatingCommand.
         */
        RepeatingCommand RepeatingCommand::create(WriteType write, ReadType read, ValueSourceType valueSource, bool longTimescale, bool markerOut, bool digOut, uint16_t L, uint16_t T) {
            RepeatingCommand result;
            result.T = CheckBits(T, 16);
            result.L = CheckBits(L, 9);
			result.digOut = digOut;
			result.markerOut = markerOut;
            result.longTimescale = longTimescale;
            result.valueSource = valueSource;
            result.muxToRead = read;
            result.registerToWrite = write;
            result.repeating = true;
            return result;
        }

        /** \brief Number of time steps that this command will take to execute.
         *
         *  NonrepeatingCommand instances always take one time step to execute.  RepeatingCommand may take more, and how many
         *  depends on the combination of the RepeatingCommand::longTimescale and RepeatingCommand::T members.  This function calculates the
         *  number of repetitions.
         *
         * \returns The number of time steps.
         */
        uint32_t RepeatingCommand::numRepetitions() const {
            uint32_t result;
            result = longTimescale ? (T << 16) : T;
            if (result == 0) {
                result = 1;
            }
            return result;
        }

        /** \copydoc RepeatingCommand::numRepetitions
         */
        uint32_t WaveformCommand::numRepetitions() const {
            if (repeating.repeating) {
                return repeating.numRepetitions();
            }
            else {
                return 1;
            }
        }

        /** \brief Number of time steps that a command list will take to execute.
         *
         * \param[in] commands  The command list.
         * \returns The number of time steps.
         */
        uint32_t numRepetitions(vector<WaveformCommand>& commands) {
            uint32_t index = 0;

            for (const WaveformCommand& command : commands)
            {
                index += command.numRepetitions();
            }

            return index;
        }
    }
}
