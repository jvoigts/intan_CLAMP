#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "ChipProtocol.h"

namespace CLAMP {
    /** \brief Classes for configuring waveforms to run on CLAMP chips
     *
     *  A waveform consists of a list of WaveformCommand instances.  Each WaveformCommand can be either:
     *  \li a NonrepeatingCommand, the (Command, Mux, Address, Data) form similar to ChipProtocol::MOSICommand
     *  \li a RepeatingCommand, which can only control the current or voltage generator, but the same
     *      command is repeated more than once.
     *
     *  See the "Per-channel Command Sequences" section of the CLAMP Programmer's Guide for more information.
     */
    namespace WaveformControl {
        /** \brief A non-repeating command, the (Command, Mux, Address, Data) form similar to ChipProtocol::MOSICommand
         *
         *  Note that the four components (Command, Mux, Address, Data) are stored differently than in
         *  ChipProtocol::MOSICommand instances - this class is used for a command on the FPGA, which is subsequently
         *  decoded/reordered to form the MISO command.
         */
        struct NonrepeatingCommand {
            // IMPORTANT: order of the bit fields matters - matches on-FPGA order

            /// Register address.  Used by command types READ, WRITE, and WRITE_AND_CONVERT.
            uint32_t A : 8;
            /// %Mux source to convert (of type ChipProtocol::MuxSelection).  Used by command types CONVERT and WRITE_AND_CONVERT.
            uint32_t M : 6;
            /// Command type.  Should be filled in by an enum of type ChipProtocol::Commands.
            uint32_t C : 2;
            /// Data to write to register.  Used by command types WRITE and WRITE_AND_CONVERT.
            uint32_t D : 9;
			/// Second digital marker
			uint32_t digOut : 1;
			/// Digital marker
			uint32_t markerOut : 1;
            /// Unused
            uint32_t unused : 4;
            /// Specifies that this is not a repeating command (used to decode a uint32_t that is either a NonrepeatingCommand or a RepeatingCommand)
            uint32_t repeating : 1;

            static NonrepeatingCommand create(ChipProtocol::Commands C_, ChipProtocol::MuxSelection M_, uint8_t A_, uint16_t D_);
            /// Convert value to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };

        /** \brief A repeating command, which can only control the current or voltage generator, but the same
         *      command is repeated more than once.
         *
         *  The Waveform RAM supports repeated commands consisting of writes to either the Clamp Current Source 
         *  or Clamp Voltage DAC registers.  The command is executed repeatedly a number of times, before moving 
         *  on to the next command.
         */
        struct RepeatingCommand {
            /// Enum for controlling which register to write to
            enum WriteType {
                WRITE_VOLTAGE = 0, ///< Write to the Clamp Voltage DAC register (Register N,0)
                WRITE_CURRENT      ///< Write to the Clamp Current Source register (Register N,9)
            };

            /// Enum for controlling which mux to read from
            enum ReadType {
                READ_VOLTAGE = 0,  ///< Read from Voltage Sense on the given channel
                READ_CURRENT       ///< Read from Current Sense on the given channel
            };

            /// Enum for controlling which value to write to the register
            enum ValueSourceType {
                LITERAL = 0,  ///< Write the literal L value
                ADC           ///< Write a scaled version of one of the FPGA board's ADCs
            };

            /// Controls number of times to repeat the command.  See also \ref longTimescale.
            uint32_t T : 16;

            /// Data to write to register if \ref valueSource is set to \ref LITERAL.
            uint32_t L : 9;

			/// Second digital marker
			uint32_t digOut : 1;

			/// Digital marker
			uint32_t markerOut : 1;

            /** \brief Together with the \ref T member above, controls how many times the command will be repeated.  
             *
             *  \li Setting this bit to true (i.e., long time scale) executes the command
             *      \code TTTT TTTT TTTT TTTT 0000 0000 0000 0000 \endcode
             *      times.
             *  \li Setting this bit to false (i.e., short time scale) executes the command
             *      \code 0000 0000 0000 0000 TTTT TTTT TTTT TTTT \endcode
             *      times
             *
             * Note that any arbitrary number of repetitions can be constructed from two adjacent commands, 
             * one with long timescale set to true and one with it set to false.
             * 
             * Also note that all repeating commands are executed at least one time, so setting T to 
             * all 0s will execute the command one time.
             */
            uint32_t longTimescale : 1;

            /** \brief Controls which value will be sent to the chip as its D.  
             * 
             * \li Setting this value to \ref ADC uses the scaled value of one of the evaluation board's ADCs.  
             * \li Setting it to \ref LITERAL sends the literal L from the member above.  
             * 
             * The ADC value (if used) will be recalculated every time though the loop.  
             * So if you repeat the command, the exact command sent to the board may vary 
             * (the exact command is captured in the USB response, i.e., ChipProtocol::MISOReturn).
             */
            uint32_t valueSource : 1;

            /** \brief Controls which mux will be used when reading
             *
             *  Repeating commands always read from the channel they write to.
             *
             * \li Setting this value to \ref READ_CURRENT reads from the current sense mux for the given channel (MUX address Unit0Current, Unit1Current, Unit2Current, or Unit3Current).
             * \li Setting it to \ref READ_VOLTAGE reads from the voltage sense mux for the given channel (MUX address Unit0Voltage, Unit1Voltage, Unit2Voltage, Unit3Voltage).
             *
             * It is not possible to read from the temperature MUX with a repeating command.
             */
            uint32_t muxToRead : 1;

            /** \brief Controls which register will be written.
             *
             *  \li Setting this value to \ref WRITE_CURRENT writes to the Clamp Current Source register (Register N,9).  
             *  \li Setting it to \ref WRITE_VOLTAGE writes to the Clamp Voltage DAC register (Register N,0).
             *
             * It is not possible to write to any other register with a repeating command.
             */
            uint32_t registerToWrite : 1;

            /// Specifies that this is a repeating command (used to decode a uint32_t that is either a NonrepeatingCommand or a RepeatingCommand)
            uint32_t repeating : 1;

            static RepeatingCommand create(WriteType write, ReadType read, ValueSourceType valueSource, bool longTimescale_, bool markerOut_, bool digOut_, uint16_t L_, uint16_t T_);
            uint32_t numRepetitions() const;

            /// Convert value to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };

        /** \brief One command in a waveform to play to one channel on one chip.
         *
         *  A waveform consists of a list of WaveformCommand instances.  Each WaveformCommand can be either:
         *  \li a NonrepeatingCommand, the (Command, Mux, Address, Data) form similar to ChipProtocol::MOSICommand
         *  \li a RepeatingCommand, which can only control the current or voltage generator, but the same
         *      command is repeated more than once.
         *
         *  This union contains a NonrepeatingCommand, a RepeatingCommand, and some functions that work on either.
         */
        union WaveformCommand {
            /// If the command is a NonrepeatingCommand, use this element
            NonrepeatingCommand nonrepeating;
            /// If the command is a RepeatingCommand, use this element
            RepeatingCommand repeating;

            uint32_t numRepetitions() const;

            /// Constructor
            WaveformCommand() {}
            /// Constructor
            WaveformCommand(const NonrepeatingCommand& nr) : nonrepeating(nr) {}
            /// Constructor
            WaveformCommand(const RepeatingCommand& r) : repeating(r) {}
            /// Constructor (used for testing)
            WaveformCommand(uint32_t value) { *reinterpret_cast<uint32_t*>(this) = value; }

            /// Convert value to a 32-bit integer
            operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        };
        static_assert(sizeof(WaveformCommand) == sizeof(uint32_t), "Waveform command has the wrong size");

        uint32_t numRepetitions(std::vector<WaveformCommand>& commands);

        /** \brief Augmented version of WaveformCommand.
        *
        *  For interpreting the results of a waveform command, it's frequently useful to have the commands that execute
        *  indexed by how many timesteps they take.  For example, if there are three repeating commands with (8, 6, 4) repetitions,
        *  respectively, it is useful to know that the first command starts at time 0, the second at time 8, and the third at time 14.
        *  This structure stores those cumulative timesteps, so that calling code doesn't have to calculate them.
        */
        struct IndexedWaveformCommand {
            /// Command
            WaveformCommand command;
            /// Timestep index when the command starts (time 0 has index 0, etc.)
            uint32_t        index;

            /// Constructor
            IndexedWaveformCommand(const WaveformCommand& cmd) : command(cmd), index(0) {}
        };
    }
}
