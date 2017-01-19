#pragma once

// Trigonometric constants
#define TWO_PI  6.28318530718
#define DEGREES_TO_RADIANS  0.0174532925199
#define RADIANS_TO_DEGREES  57.2957795132

// Special Unicode characters, as QString data type
#define QSTRING_MU_SYMBOL  ((QString)((QChar)0x03bc))
#define QSTRING_OMEGA_SYMBOL  ((QString)((QChar)0x03a9))
#define QSTRING_ANGLE_SYMBOL  ((QString)((QChar)0x2220))
#define QSTRING_DEGREE_SYMBOL  ((QString)((QChar)0x00b0))
#define QSTRING_PLUSMINUS_SYMBOL  ((QString)((QChar)0x00b1))

// Interface board FPGA constants
#define MAX_COMMAND_SEQUENCE_LENGTH 16384
