#pragma once

#include <cstdint>

namespace CLAMP {
    /** \brief This namespace consists of structures used in the low-level protocol between the FPGA and the CLAMP chips.
    *
    * Information related to this protocol can be found in the Intan_CLAMP_series_datasheet.pdf.
    *
    * Much of the CLAMP API code exists to interpret and transform these low-level protocol details into meaninful higher
    * level abstractions.  The structures in this namespace can be viewed as low-level implementation details, which are
    * used internally, and probably do not need to be studied to use the API in its normal modes, only when delving into
    * the guts of the internals.
    */
    namespace ChipProtocol {
        // MOSI and related

        /// Value of the C (command type) member of a MOSICommand
        enum Commands {
            READ = 0,              ///< Read from an on-chip register
            WRITE,                 ///< Write to an on-chip register
            CONVERT,               ///< Convert (analog-to-digital) the currently selected mux value, and return the result
            WRITE_AND_CONVERT      ///< Write to an on-chip register, convert (analog-to-digital) the currently selected mux value, and return the result
        };

        /// Value of the M (mux) member of a MOSICommand
        enum MuxSelection {
            /** Used for non-convert commands.  The chip doesn't really care about the value of the mux for non-convert commands, 
                but it's nice to have a defined, repeatable value, rather than random bits, especially for debugging purposes.
            */
            None = 0,
            Unit0Current = 0,       ///< %Channel 0, Current Sense
            Unit0Voltage,           ///< %Channel 0, Voltage Sense
            Unit1Current,           ///< %Channel 1, Current Sense
            Unit1Voltage,           ///< %Channel 1, Voltage Sense
            Unit2Current,           ///< %Channel 2, Current Sense
            Unit2Voltage,           ///< %Channel 2, Voltage Sense
            Unit3Current,           ///< %Channel 3, Current Sense
            Unit3Voltage,           ///< %Channel 3, Voltage Sense
            Temperature = 63        ///< Temperature
        };

        /** \brief A MOSI command from FPGA to CLAMP chip.
         *
         *  The SPI protocol, used by CLAMP chips, consists of :
         *  \li a bit being sent from the master (i.e., the FPGA board) to the slave (i.e., the CLAMP board) - this is called Master Output Slave Input (MOSI)
         *  \li a bit being sent from the slave (i.e., the CLAMP board) to the master (i.e., the FPGA board) - this is called Master Input Slave Output (MISO)
         *
         *  This structure contains a full 32-bit MOSI command.  The 32-bit MISO return value is found in the MISOReturn structure.
         *
         *  See the individual fields for details, as well as the datasheet.
         */
        struct MOSICommand {
            /// Unused
            uint32_t unused : 7;

            /// Data to write to register.  Used by command types WRITE and WRITE_AND_CONVERT.
            uint32_t D : 9;

            /// Register address.  Used by command types READ, WRITE, and WRITE_AND_CONVERT.
            uint32_t A : 8;

            /// %Mux source to convert (of type ::MuxSelection).  Used by command types CONVERT and WRITE_AND_CONVERT.
            uint32_t M : 6;

            /// Command type.  Should be filled in by an enum of type ::Commands.
            uint32_t C : 2;

            MOSICommand(Commands C_, MuxSelection M_, uint8_t A_, uint16_t D_);
            MOSICommand();

            /// Convert command to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };
        static_assert(sizeof(MOSICommand) == sizeof(uint32_t), "MOSI Command is the wrong size");


        // MISO and related

        /** \brief MISO return value corresponding to a MOSI READ command
         *
         * See also MISOReturn.
         */
        struct ReadValue {
            /// Register value
            uint32_t value : 9;
            /// Unused
            uint32_t unused : 23;

            /// Convert this structure to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };

        /** \brief MISO return value corresponding to a MOSI CONVERT or WRITE_AND_CONVERT command, when you have a 16-bit ADC
         *
         * See also MISOReturn.
         */
        struct ConvertValue16 {
            /// Signed 16-bit ADC value
            int32_t value : 16;
            /// Unused
            int32_t unused : 16;

            /// Convert this structure to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };

        /** \brief MISO return value corresponding to a MOSI CONVERT or WRITE_AND_CONVERT command, when you have an 18-bit ADC
         *
         * See also MISOReturn.
         */
        struct ConvertValue18 {
            /// Signed 18-bit ADC value
            int32_t value : 18;
            /// Unused
            int32_t unused : 14;

            /// Convert this structure to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };

        /** \brief A union that contains the MISO return value of a command.
         * 
         *  How to interpret it depends on the kind of command sent.  See the members for details.
         *
         *  See MOSICommand and the data sheet for details on the protocol in general.
         */
        union MISOReturn {
            /// Return from a READ command
            ReadValue read;
            /// Return from a CONVERT or WRITE_AND_CONVERT command, if a 16-bit ADC is used
            ConvertValue16 convert16;
            /// Return from a CONVERT or WRITE_AND_CONVERT command, if an 18-bit ADC is used
            ConvertValue18 convert18;

            /// Convert this structure to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };
        static_assert(sizeof(MISOReturn) == sizeof(uint32_t), "MISO Return is the wrong size");
    }
}
