#include "RAM.h"
#include "common.h"

namespace CLAMP {
    /// \cond private
    VirtualRegisterAddress::VirtualRegisterAddress(uint8_t chip_, uint8_t unit_, uint8_t address_) {
        address = CheckBits(address_, 4);
        unit = CheckBits(unit_, 4);
        chip = CheckBits(chip_, 3);
        unused = CheckBits(0, 2);
        type = CheckBits(6, 3); // binary 110
    }

    GlobalVirtualRegisterAddress::GlobalVirtualRegisterAddress(uint8_t address_) {
        address = CheckBits(address_, 4);
        unused = CheckBits(0, 9);
        type = CheckBits(7, 3); // binary 111
    }

    WaveformRAMAddress::WaveformRAMAddress(uint16_t address_) {
        address = CheckBits(address_, 15);
        type = CheckBits(0, 1);
    }
    /// \endcond
}
