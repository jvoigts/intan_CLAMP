#pragma once

#include <cstdint>

namespace CLAMP {
    const unsigned int KILO = 1024;
    const unsigned int MEGA = KILO * KILO;

    const unsigned int MAX_NUM_CHANNELS = 4;  // Channels per chip

    const unsigned int MAX_NUM_CHIPS = 8;     // Chips per evaluation board

    const unsigned int MAX_NUM_DELAYS = 16;  // Cable delay = 0-15

    const double PI = 3.141592653;

    const double STEP16 = 2.56 / (1 << 15); // Full range is +/-, so we want 1/2 that for 2.56 V
    const double STEP18 = 2.56 / (1 << 17); // Full range is +/-, so we want 1/2 that for 2.56 V
    const double STEPADC = 3.3 / (1 << 16); // Full range is 0 .. 3.3V, and they are 16-bit
    const int32_t INVALID_MUX_VALUE = 0x7FFFFFFF; // Out of the range of 18-bit values

    const unsigned int FIFO_CAPACITY_WORDS = 0x4000000;

    const double MAX_FEEDBACK_CAPACITANCE = 51.0e-12;
    const double FEEDBACK_CAPACITANCE_STEP = 0.2e-12;

	enum WireIn {
		LedDisplay = 0x00,
		RunControl = 0x01,
		DCMFreq = 0x02,
		RAMWriteStart = 0x03,
		RAMWriteEnd = 0x04,
		Channels = 0x05,
		MaxTimestepLow = 0x06,
		MaxTimestepHigh = 0x07,
		AdcConfigWord = 0x08,
		SerialDigitalInCntl = 0x09,
		AdcSelectEnChip0 = 0x0a,
		AdcSelectEnChip1 = 0x0b,
		AdcSelectEnChip2 = 0x0c,
		AdcSelectEnChip3 = 0x0d,
		AdcSelectEnChip4 = 0x0e,
		AdcSelectEnChip5 = 0x0f,
		AdcSelectEnChip6 = 0x10,
		AdcSelectEnChip7 = 0x11,
		DacConfigWord = 0x12,
		SpiPortLeds = 0x13,
		StatusLeds = 0x14,
		DigOutEnable = 0x15,
		DigOutSelectA = 0x16,
		DigOutSelectB = 0x17,
		DisablePorts = 0x18
	};

	enum Triggers {
		DCMProgram = 0x40,
		RAMWrite = 0x40,
		Start = 0x41,
		DacConfigLoad = 0x42,
		DacVoltageMultiplierLoad = 0x43,
		DacCurrentMultiplierLoad = 0x44,
		DacVoltageOffsetLoad = 0x45,
		DacCurrentOffsetLoad = 0x46,
		AdcVoltageMultiplierLoad = 0x47,
		AdcCurrentMultiplierLoad = 0x48
	};

	// For Triggers
	enum Bit {
		StartBit = 0x0,
		RamWriteBit = 0x1,
		DCMProgramBit = 0x0,
		Dac1 = 0x0,
		Dac2 = 0x1,
		Dac3 = 0x2,
		Dac4 = 0x3,
		Dac5 = 0x4,
		Dac6 = 0x5,
		Dac7 = 0x6,
		Dac8 = 0x7
	};

	// For WiresIn
	enum BitMask {
		ResetBitMask = 0x1,
		RunContinuouslyBitMask = 0x2,
		DataClkLockedBitMask = 0x1,
		DcmProgDoneBitMask = 0x2,
		TtlInSerial = 0x1,
		TtlInSerialExp = 0x2,
		ExpanderDetectBitMask = 0x4,
		ExpanderIdBitMask = 0x8
	};

	enum WireOut {
		Programming = 0x20,
		NumWordsFIFOLow = 0x21,
		NumWordsFIFOHigh = 0x22,
		SerialDigitalIn = 0x23,
		BoardMode = 0x24,
		DigOffset = 0x25, // TEMP
		DigEnable = 0x26, // TEMP
		DigCommands = 0x27, // TEMP
		Address0 = 0x28, // TEMP
		AddressDigOut = 0x29, // TEMP

		BoardId = 0x3e,
		BoardVersion = 0x3f
	};

	enum PipeIn {
		Waveform = 0x80
	};

	enum PipeOut {
		Data = 0xA0
	};

	const int CLAMP_BOARD_MODE = 15;
	const int CLAMP_BOARD_ID = 600;

}
