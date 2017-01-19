#include "ChipProtocol.h"
#include "common.h"

namespace CLAMP {
    namespace ChipProtocol {
        /** \brief Constructor
         *
         * Note that not all command types require all fields.  If a field is not needed, setting it to a default value (e.g. 0) is recommended.
         *
         * \param[in] C_    Command type (see C member)
         * \param[in] M_    %Mux (see M member)
         * \param[in] A_    Address (see A member)
         * \param[in] D_    Data (see D member).  Only the bottom 9 bits are used.
         */
        MOSICommand::MOSICommand(Commands C_, MuxSelection M_, uint8_t A_, uint16_t D_) {
            C = C_;
            M = M_;
            A = A_;
            D = CheckBits(D_, 9);
            unused = 0;
        }

        MOSICommand::MOSICommand() {
            C = 0;
            M = 0;
            A = 0;
            D = 0;
            unused = 0;
        }
    }
}
