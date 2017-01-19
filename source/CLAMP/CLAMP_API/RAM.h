#pragma once

#include <cstdint>

namespace CLAMP {
    /// \cond private
    // Simple structs for addresses in the Global RAM on the FPGA.  Used internally.
    struct VirtualRegisterAddress {
        uint16_t address : 4;
        uint16_t unit : 4;
        uint16_t chip : 3;
        uint16_t unused : 2;
        uint16_t type : 3;

        VirtualRegisterAddress(uint8_t chip_, uint8_t unit_, uint8_t address_);
        operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
    };

    struct GlobalVirtualRegisterAddress {
        uint16_t address : 4;
        uint16_t unused : 9;
        uint16_t type : 3;

        GlobalVirtualRegisterAddress(uint8_t address_);
        operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
    };

    struct WaveformRAMAddress {
        uint16_t address : 15;
        uint16_t type : 1;

        WaveformRAMAddress(uint16_t address_);
        operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
    };
    /// \endcond
}
