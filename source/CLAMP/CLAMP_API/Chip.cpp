#include "Chip.h"
#include "Board.h"
#include "common.h"
#include <sstream>
#include <exception>
#include "RAM.h"

using std::string;
using std::ostringstream;
using std::runtime_error;
using std::invalid_argument;
using std::endl;
using std::vector;
using CLAMP::Registers::Register;
using CLAMP::Registers::Unit;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::WaveformControl;

namespace CLAMP {
    //----------------------------------------------------------------------------------------
    /// Constructor
    Chip::Chip(Board& board_, unsigned int index_) :
        index(index_),
        board(board_),
        RCal1(10e6), 
        RCal2(1e6),
		numChannels(0)
    {
        for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
            channel[i] = new Channel(*this, static_cast<ChannelNumber>(i));
        }
    }

    Chip::~Chip()
    {
        for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
            delete channel[i];
        }
    }

    /** \brief Sets the MISO-sampling delay used to compensate for long cables
     *
     *   The SPI protocol, used by CLAMP chips, consists of :
     *   \li a bit being sent from the master (i.e., the FPGA board) to the slave (i.e., the CLAMP board) - this is called Master Output Slave Input (MOSI)
     *   \li a bit being sent from the slave (i.e., the CLAMP board) to the master (i.e., the FPGA board) - this is called Master Input Slave Output (MISO)
     *
     *   Typically, both bits are read at the same time.  However, with the FPGA and the CLAMP chip physically separated by a significant distance
     *   (several feet or meters), there may be a noticeable lag before the MISO input to the FPGA stabilizes.  This parameter lets you to set the delay
     *   between when the FPGA writes the MOSI output and reads the MISO input to compensate for that.
     *
     *   The parameter is not as useful as the corresponding parameter on RHD2000 chips, because the sampling rate for CLAMP is much lower.
     *
     *   \param[in] value Delay [0-15]
     */
    void Chip::setCableDelay(uint8_t value) {
        writeVirtualRegister(0, CheckBits(value, 4));
    }

    void Chip::writeVirtualRegister(uint8_t address, uint16_t value) {
        VirtualRegisterAddress addr(index, 15, address);

        vector<uint32_t> data(1, value);

        board.writeRAM(addr, data);
    }

    /** \brief Fills the in-memory value of a register with the value returned by a chip's READ command
     *
     *  If you want to read the registers from the chip (not just from memory), you execute READ commands
     *  with the registers addresses.  This function takes the return of one of those commands and stores
     *  the register value in memory.
     *
     *  This is used at startup to read ROM and determine chip type, for example.
     *
     *  \param[in] mosi  The MOSI command that was sent to the chip.  It's okay to call this function with a non-READ
     *                   MOSI command; the function simply doesn't do anything in that case.
     *  \param[in] miso  The returned MISO value.
     */
    void Chip::readBackRegister(const MOSICommand& mosi, const MISOReturn& miso) {
        if (mosi.C == READ) {
            Register& reg = getRegister(mosi.A);
            reg.value = miso.read.value;
        }
    }

    Register& Chip::getRegister(uint8_t address) {
        Unit unit = static_cast<Unit>(address >> 4);
        uint8_t index = address & 0xF;

        switch (unit)
        {
        case Unit::Unit0:
        case Unit::Unit1:
        case Unit::Unit2:
        case Unit::Unit3:
            return channel[unit]->registers.get(index).value;
            break;
        case Unit::Global:
            return chipRegisters.get(index).value;
            break;
        default:
            throw invalid_argument("Unit is not valid");
            break;
        }
    }

    /** \brief Creates a waveform command list to send all in-RAM commands to the chips.
     *
     *  To set all registers on a chip, you set the register values in memory, call this
     *  function to get a list of commands, and execute those commands on the chip.
     *  This is used at startup to establish initial values of all registers, for instance.
     *  Subsequently, most register updates affect a handful of registers only, not all.
     *
     *  \returns The command list.
     */
    vector<WaveformCommand> Chip::writeAllRegistersCommands() {
        vector<WaveformCommand> data;

        for (unsigned int unit = 0; unit < MAX_NUM_CHANNELS; unit++) {
            for (unsigned int registerIndex = 0; registerIndex <= 13; registerIndex++) {
                data.push_back(channel[unit]->registers.get(registerIndex).writeCommand());
            }
        }

        for (unsigned int registerIndex = 0; registerIndex <= 3; registerIndex++) {
            data.push_back(chipRegisters.get(registerIndex).writeCommand());
        }

        return data;
    }
}
