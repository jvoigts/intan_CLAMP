#pragma once

#include "OpalKellyBoard.h"
#include <vector>
#include <deque>
#include <memory>
#include "Waveform.h"
#include "Channel.h"
#include "Chip.h"
#include "Constants.h"
#include "ReadQueue.h"

namespace CLAMP {
    /** \brief In-memory representation of a CLAMP evaluation board.
     *
     *  This class contains functionality for controlling the chips attached to the board, the ADCS and digital I/O, the
     *  on-FPGA waveform RAM, etc.
     *
     *  See also ClampConfig::ClampController (the *controller* member) - many per-chip and per-channel configuration options 
     *  are much easier to accomplish using that interface than doing them by hand.
     *
     *  See below for details.
     */
    class Board {
    public:
        Board(bool is18bit=true);
        ~Board();

        /// \name Initialization
        //@{
        bool open(const std::string& dllPath = "", const std::string& bitfilePath = "");
        void scanForChips();
        //@}

        /** Alternate interface for accessing chip- and channel-level functionality.
         *
         *  The Board and its Chip and Chip->Channel members are structured the way the chips are structured.
         *  The *controller* member is structured subsystem-by-subsystem, and has logic to produce higher-level
         *  operations (e.g., calibrate) that may require write/read/interpret cycles and may require a mixture
         *  of per-channel, per-chip, and per-board actions.
         */
        ClampConfig::ClampController controller;

        /** \brief In-memory representation of the CLAMP chips attached to this board.
         *
         *  Note that there may be more objects in memory than actually attached to the board; check the *present* member
         *  of Chip for whether the given chip is attached.
         */
        Chip* chip[MAX_NUM_CHIPS];

		bool spiPortPresent[8];
		bool userId[3];
		bool serialId[4];
		bool digOutVoltageLevel;

        /// Read queue that buffers and interprets USB data coming back from the FPGA
        ReadQueue readQueue;

        /** \name Channels
         */
        //@{
        void enableChannels(const ClampConfig::ChipChannelList& channelList, bool allowMulti = false);
		void addEnabledChannels(const ClampConfig::ChipChannelList& channelList);
        /// Number of repetitions of each channel.  See enableChannels() for more details.
        unsigned int channelRepetition;
        ClampConfig::ChipChannelList getPresentChannels() const;
        unsigned int numActiveChannels() const;
        //@}

        /** \name Board-level I/Os
         *
         *  Controls for the FPGA-board I/Os: ADCs, digital inputs, digital outputs, LEDs.
         */
        //@{
        void setFpgaLeds(uint8_t value);
		void setSpiPortLeds(uint8_t value);
		void setStatusLeds(bool digitalInControl, uint8_t value);
        void setDataTransfer(bool adcs[8], bool digin, bool digout);
//        void writeDigitalOutputRAM(int port, const std::vector<uint16_t>& data);
//        void enableDigitalOutputs(bool enable[16]);
//        void enableCommandControlOfDigitalOutputs(bool enable);
//		void setDigitalOutputControlPort(int port);
		void enableDigitalMarker(int port, bool enabled);
		void setDigitalMarkerDestination(int port, int digOut);
		void updateDigOutConfig();
		bool digitalMarkerEnabled(int port);
		int digitalMarkerDestination(int port);
		void configureDac(int dac, bool enable, int port, int channel, bool outputClamp);
		void setDacVoltageMultiplier(int dac, double gain);
		void setDacCurrentMultiplier(int dac, double gain);
		void setDacVoltageOffset(int dac, int16_t value);
		void setDacCurrentOffset(int dac, int16_t value);
		void setVoltageMultiplier(int port, double gain);
		void setCurrentMultiplier(int port, uint16_t value);
		void enableAdcControl(bool enable, int chip, int channel);
		void selectAdcControl(int adc, int chip, int channel);
		//@}

        /// \name Run control
        //@{
        void runContinuously();
        void stop();
        void flush();
        void runFixed(uint32_t numTimesteps);
        void runAndReadOneCycle(int chipIndex);
        void runOneCycle(int chipIndex, unsigned int extraTimesteps = 0);
        void readOneCycle(int chipIndex);
        unsigned int getNumTimesteps(int chipIndex) const;
        //@}

        /// \name Reading
        //@{
        unsigned int read(unsigned int numPackets);
        void blockingRead(unsigned int numPackets);
        uint32_t numWordsInFifo();
        void readBackAll(); // Looping version of readBack

        /** \brief How full the FIFO is, as a percentage [0-100]
         *
         *  Only updated whenever data is read.
         */
        double fifoPercentageFull;
        /** \brief The latency of the FPGA's FIFO queue, in ms.
         *
         * This can be interpreted as amount of data (in milliseconds worth) that is in
         * the board's FIFO and has not yet been transferred to the computer.
         *
         *  Only updated whenever data is read.
         */
        double latency;
        //@}


        double getSamplingRateHz() const;

        /// \name Commands
        //@{
        void configurePerChipCommands(const std::vector<WaveformControl::WaveformCommand>& commands);
        void clearCommands();
		void clearSelectedCommands(const ClampConfig::ChipChannelList& channelList);
        void commandsToFPGA();
		void commandsToFPGA(const ClampConfig::ChipChannelList& channelList);
		void commandsToFPGASinglePort(int port);
        //@}

		bool isDacInUse(int dac, ClampConfig::ChipChannel& chipChannel, bool& outputClamp);
		bool expanderBoardPresent();
		int expanderBoardId();

		void enableAllPorts();
		void enableOnePortOnly(int port);

    private:
        OpalKellyBoard okb;

        // Functions in this class are designed to be thread-safe.  This variable is used to ensure that.
        // Note that the OpalKellyBoard class is also thread-safe; this variable is used for commands that
        // require more than one OpalKellyBoard call.
        std::mutex commandMutex;

        std::vector<ChannelNumber> channels;

        bool is18bitADC;
		bool expanderBoardDetected;
		int expanderBoardIdNumber;

		bool dacInUse[8];
		bool dacOutputClamp[8];
		bool digOutEnabled[8];
		int digOutDestination[8];
		std::vector<ClampConfig::ChipChannel> usingDac;

        // Buffer for reading bytes from USB interface
        unsigned char* usbBuffer;
        unsigned int usbBufferSize;

        std::shared_ptr<WaveformControl::WaveformExtent> digitalOutputExtent;

        void setChannelLoopOrder(const std::vector<ChannelNumber>& channels_);
        void setUSBBufferSize(unsigned int size);
        void setClockFreq_internal(uint8_t M, uint8_t D);
        void updateFIFOStats(unsigned int perPacketSizeWords);
        void writeGlobalVirtualRegister(uint8_t address, uint16_t value);
        void setSamplingRate();
        ClampConfig::ChipChannelList getAllChannels() const;
		void readDigitalInManual();
        void initialize();
		void updateAdcClampControl();

		bool adcClampControlEnable[MAX_NUM_CHIPS][MAX_NUM_CHANNELS];
		int adcClampControlSelect[MAX_NUM_CHIPS][MAX_NUM_CHANNELS];

        bool adcTransfer[8];
        bool diginTransfer;
        bool digoutTransfer;
        void setDigitalCommandOffset(uint16_t offset);
        friend class USBPacket;
        friend class ReadQueue;

        WaveformControl::WaveformRAM waveformRAM;
        void writeRAM(uint16_t start_addr, const std::vector<uint32_t>& data);
        friend class Chip;
        friend class Channel;
        friend class WaveformControl::WaveformRAM;

        void reset();
    };
}
