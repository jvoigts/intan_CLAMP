#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <memory>
#include "WaveformCommand.h"

namespace CLAMP {

    /** \brief This namespace consists of structures containing the internal representations of the registers of a CLAMP chip.
    *
    *    Much of the information related to these registers can be found in the Intan_CLAMP_series_datasheet.pdf, in the sections
    *    "On-Chip RAM Registers" and "On-Chip ROM Registers"
    *
    * Each CLAMP chip contains 60 nine-bit RAM configuration/control registers. Upon power-up, all RAM registers contain indeterminate data
    * and should be promptly configured by the SPI master device.
    *
    * Individual bits in a register can be changed only by rewriting the entire nine-bit contents. Therefore, it is recommended that the SPI
    * master device maintain a copy of CLAMP register contents in its memory so bitwise operations can be performed there before writing the
    * updated byte to the chip using a WRITE command on the SPI bus.
    *
    * The RAM registers present in each CLAMP chip are described in the classes of this namespace and in the datasheet.
    */
    namespace Registers {
        /** \defgroup unitRegisters Per-Channel Registers
        * \brief Per-channel registers
        *
        * The CLAMP chip contains four identical patch clamp units. Each patch clamp unit contains independent circuitry for both voltage clamp
        * and current clamp control and sensing. Each patch clamp unit contains 14 registers used for control and configuration. These registers
        * are repeated across the chip for each patch clamp unit N, where N ranges from 0 to 3.
        *
        * Each 32-bit SPI command word contains an 8-bit address field. The top four bits of this field designate to the patch clamp unit N, and
        * the bottom four bits designate the register number listed below (i.e., 0 through 13).
        *
        *   @{
        */

        /** \brief %Register N,0: Clamp Voltage DAC
        */
        struct Register0 {
            /** \brief This variable sets the voltage magnitude of a DAC used to generate the clamp voltage.
             *
             * This variable, along with clampVoltagePlusSign,
             * must be updated at regular intervals to create desired waveforms. Note that this DAC must be enabled by setting clampDACPower in
             * Register1. The step size of this DAC is controlled by the clampStepSize bit in Register1.
             */
            uint16_t clampVoltageMagnitude : 8;

            /// Setting this bit to zero selects negative clamp voltages. Setting this bit to one selects positive clamp voltages.
            uint16_t clampVoltagePlusSign : 1;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register0();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }

            /// Convert a 16-bit signed integer to this register's value
            void setValue(int16_t value);
        };

        /** \brief %Register N,1: Clamp Voltage DAC Configuration
        */
        struct Register1 {
            /** \brief Time constant for low-pass filter connected to output of clamp voltage DAC.
             *
             *  This variable sets the time constant of a low-pass filter connected to the output of the clamp
             *  voltage DAC. The step size of this variable is 0.5 &mu;s, so the time constant may be set within the range of 0 - 15.5 &mu;s. A typical
             *  value of this time constant for most general voltage-clamp applications is 10 &mu;s. For rapid voltage changes (e.g., FSCV), smaller
             *  time constants should be used.
             */
            uint16_t clampVoltageTimeConstantA : 5;

            /** \brief Sets clamp voltage DAC to 2.5 mV steps or 5 mV steps.
             *
             * Setting this bit to one sets the clamp voltage DAC step size to 5 mV with a range of &plusmn;1.275 V. Setting this bit
             *  to zero sets the clamp voltage DAC step size to 2.5 mV with a range of &plusmn;0.6375 V.
             */
            uint16_t clampStepSize : 1;

            /** \brief Power for the clamp voltage DAC.
             *
             * Setting this bit to zero shuts down the clamp voltage DAC and can be used to reduce power consumption
             * when voltage clamp mode is not used. Under normal operation this bit should be set to one.
             */
            uint16_t clampDACPower : 1;

			/// Unused
			uint16_t unused : 2;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register1();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,2: Clamp Voltage DAC Offset Trim
        */
        struct Register2 {
            /**  \brief Trims the DC level of the clamp voltage DAC
             *
             *  This variable trims the DC level of the clamp voltage DAC to compensate for imperfections in
             *  fabricated circuit components on the chip. Setting this variable to 32 sets the trim to zero. Every step above this value adds a DC
             *  shift of approximately 0.27 mV; every step below this value subtracts 0.27 mV. The total trim range is thus +8.37 mV (with this
             *  variable set to 63) to -8.64 mV (with this variable set to zero). This variable should be set during a self-calibration sequence run
             *  after the chip is powered up, before the voltage clamp circuit is used.
             */
            uint16_t clampVoltageOffsetTrim : 6;

            /// Unused
            uint16_t unused : 3;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register2();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,3: Current-to-Voltage Feedback Resistance; Clamp Voltage Time Constant B
        */
        struct Register3 {
            /// Values for Register3::feedbackResistance
            enum Resistance {
                R200k = 1,  ///< 200 k&Omega;
                R2M = 2,    ///<   2 M&Omega;
                R20M = 3,   ///<  20 M&Omega;
                R40M = 4,   ///<  40 M&Omega;
				R80M = 5	///<  80 M&Omega;
            };

            /** \brief Feedback resistor used in voltage clamp mode.
             *
             *  This variable sets the value of the feedback resistor used to convert current to voltage during voltage clamp
             *  operation. The variable should only be set to one of the Register3::Resistance values.
             *
             *  You probably should not set this directly, but rather use Channel::setFeedbackResistanceInMemory, which not only sets
             *  this register, but also updates the feedback capacitance to get as close to the desired bandwidth as possible.
             */
            uint16_t feedbackResistance : 3;

            /** \brief Time constant for low-pass filter connected to input of voltage clamp amplifier.
             *
             *  This variable sets the time constant of a low-pass filter connected to the input of the voltage
             *  clamp amplifier. The step size of this variable is 0.05 &mu;s, so the time constant may be set within the range of 0 - 1.55 &mu;s. A typical
             *  value of this time constant for most general voltage-clamp applications is 0.50 &mu;s.
             */
            uint16_t clampVoltageTimeConstantB : 5;

			/// Unused
			uint16_t unused : 1;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register3();

            double nominalResistance() const;
            Resistance resistanceEnum() const;
            static double nominalResistance(Resistance r);

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,4: Current-to-Voltage Feedback Capacitance; Voltage Clamp Power
        */
        struct Register4 {
            /** \brief Feedback capacitor in the current-to-voltage converter circuit.
             *
             *  This variable sets the value of the capacitor in parallel with the feedback resistor in the current-to-voltage
             *  converter circuit. The product of this capacitance and the feedback resistance (selected in Register3) sets a time
             *  constant that limits the bandwidth of the voltage clamp amplifier. The step size of this variable is 0.2 pF, so the feedback
             *  capacitance can be set within the range of 0 - 51 pF.
             *
             *  You probably should not set this directly, but rather use Channel::setDesiredBandwidth to set the desired cutoff
             *  frequency of the low-pass filter consisting of the feedback resistor and the feedback capacitor controlled by this register.
             */
            uint16_t feedbackCapacitance : 8;

            /** \brief Turn voltage clamp power on or off.
             *
             *  Setting this bit to zero shuts down the voltage clamp amplifier and can be used to reduce power
             *  consumption when voltage clamp mode is not used. Under normal operation this bit should be set to one.
             */
            uint16_t voltageClampPower : 1;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register4();

            double capacitance() const;
            void setFeedbackCapacitance(double c);

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,5: Voltage Clamp Difference Amplifier Configuration
        */
        struct Register5 {
            /** \brief Trims the DC level of the difference amplifier.
              *
              * This variable trims the DC level of the difference amplifier to compensate for imperfections in fabricated
              * circuit components on the chip. Setting this variable to 32 sets the trim to zero. Every step above this value adds a DC shift of
              * approximately 2.7 mV to the output of the current-to-voltage converter; every step below this value subtracts 2.7 mV. The total
              * trim range is thus +83.7 mV (with this variable set to 63) to -86.4 mV (with this variable set to zero). This variable should be set
              * during a self-calibration sequence run after the chip is powered up, before the voltage clamp circuit is used.
              */
            uint16_t diffAmpOffsetTrim : 6;

            /** \brief Connects the negative input of the difference amplifier to the normal input or to ground.
              *
              * Setting this bit to zero connects the negative input of the gain-of-10 difference amplifier to the clamp voltage;
              * this is the normal mode of operation. Setting this bit to one connects the negative input of the difference amplifier to ground. This
              * can be used by self-calibration algorithms to measure and trim any DC offset in the difference amplifier.
              */
            uint16_t diffAmpInMinusSelect : 1;

            /**  \brief Connects the positive input of the difference amplifier to the normal input or to ground.
              *
              * Setting this bit to zero connects the positive input of the gain-of-10 difference amplifier to the output of the
              * voltage clamp amplifier; this is the normal mode of operation. Setting this bit to one connects the positive input of the difference
              * amplifier to ground. This can be used by self-calibration algorithms to measure and trim any DC offset in the difference amplifier.
              */
            uint16_t diffAmpInPlusSelect : 1;

            /// Unused
            uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register5();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,6: Fast Transient Capacitive Compensation Magnitude
        */
        struct Register6 {
            /** \brief Magnitude of fast transient capacitive compensation.
              *
              * This variable sets the magnitude of capacitance to be compensated by the fast transient
              * capacitive compensation circuitry. This variable should always have a value in the range of 55 - 255. Setting this variable to 55
              * selects zero capacitance compensation. The step size of this variable is 0.05 pF, so the range of capacitive compensation is 0 -
              * 10 pF.
              */
            uint16_t fastTransCapCompensation : 8;

			/** \brief Sets fast transient capacitive compensation to voltage clamp or current clamp mode.
			*
			* Setting this bit to zero connects the input of the fast transient capacitive compensation circuitry to the clamp
			* voltage. This is the normal mode of operation for voltage clamp operation. Setting this bit to one connects the input to the main
			* input from the off-chip electrode. This enables capacitive compensation in current clamp mode.
			*/
			uint16_t fastTransInSelect : 1;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register6();

            void setMagnitude(double magnitude, double stepSize);
            int getMagnitude();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,7: Fast Transient Capacitive Compensation Configuration; Clamp Configuration
        */
        struct Register7 {
            /** \brief Time constant of low-pass filter connected to the input of the fast transient capacitive compensation circuitry.
              *
              * This variable sets the time constant of a low-pass filter connected to the input of the fast transient
              * capacitive compensation circuitry. The step size of this variable is 0.05 &mu;s, so the time constant may be set within the range of
              * 0 - 1.55 &mu;s. A typical value of this time constant for most applications is 0.50 &mu;s, but this is typically adjusted by the user to fine-tune
              * capacitive compensation.
              */
            uint16_t fastTransTimeConstant : 5;

			/** \brief Connects the fast transient capacitive compensation circuitry to the main input.
			*
			* Setting this bit to one connects the fast transient capacitive compensation circuitry to the main input from the
			* off-chip electrode. Setting this bit to zero disconnects this circuitry, which can be done if capacitive compensation is not enabled.
			*/
			uint16_t fastTransConnect : 1;

			/** \brief Connect or disconnect the voltage clamp from the input.
			*
			* Setting this bit to one connects the input of the voltage clamp amplifier to the main input from the off-chip
			* electrode. Setting this bit to zero disconnects the voltage clamp amplifier, as should be done in current clamp mode.
			*/
			uint16_t voltageClampConnect : 1;

			/** \brief Enables current clamp
			*
			* Setting this bit to one enables the current source to drive a specified current onto the electrode. Setting
			* this bit to zero sets the current source output to zero. The current source can be enabled to establish a current clamp or to subtract
			* a large background current in voltage clamp mode.
			*/
			uint16_t clampCurrentEnable : 1;

            /** \brief Turns power to fast transient capacitive compensation circuitry on or off
              *
              * Setting this bit to zero shuts down the fast transient capacitive compensation circuitry and can be used to reduce
              * power consumption when fast transient compensation is not used. Under normal operation this bit should be set to one.
              */
            uint16_t fastTransPower : 1;

            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register7();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,8: Input Select; Fast Transient Configuration; Voltage Amplifier Power
        */
        struct Register8 {
            /** \brief Enum used to set main input to electrode, calibration resistor(s), ground, or open.
             *
             *  Note that all four channels share the same connections to the two calibration resistors,
             *  so at most one channel should be connected to each of those at a time, or else the results
             *  will not be what you expect.  All other inputs are replicated per-channel.
             */
            enum InputSelect {
                Open0 = 0,        ///< Open Circuit
                Open1 = 1,        ///< Open Circuit
                Open2 = 2,        ///< Open Circuit
                Open3 = 3,        ///< Open Circuit
                ElectrodePin = 4, ///< Electrode pin (in0, in1, in2, or in3 for channel 0-3, respectively)
                RCal1 = 5,        ///< Calibration resistor 1 pin (Rcal1)
                RCal2 = 6,        ///< Calibration resistor 2 pin (Rcal2)
                Ground = 7        ///< Short to ground
            };

            /** \brief Sets the input for this channel to electrode, calibration resistor(s), ground, or open.
              *
              * This variable selects the input signal that is routed to the patch clamp circuit assembly. Various settings of this
              * variable are useful in self calibration routines. This should be set to one of the Register8::InputSelect values.
              * Under normal operation, this variable should be set to ElectrodePin.
              */
            uint16_t inputSelect : 3;

			/// Values for Register8::buzzSwitch
			enum BuzzState {
				BuzzOff = 0,  ///< fast transient compensation amplifier driving CC;
				BuzzGnd = 1,    ///< GND (0 V) driving CC;
				BuzzPos = 2,   ///< VREF (+1.28 V) driving CC;
				BuzzNeg = 3   ///< VSS (-1.28 V) driving CC;
			};

            /** This variable controls a 4-way switch at the bottom of fast transient compensation capacitor CC that can be
              * used to switch between different voltages to create a "buzz" function.  This should be set to one of the
			  * Register8::BuzzState values.  Under normal operation, this variable should be set to BuzzOff.
              */
            uint16_t buzzSwitch : 2;

			/// Values for Register8::fastTransRange
			enum CompensationRange {
				Range10pF = 0,  ///< CC = 2.75 pF, fast transient compensation step size = 0.05 pF, fast transient compensation range = 0 - 10 pF;
				Range14pF = 1,  ///< CC = 3.85 pF, fast transient compensation step size = 0.07 pF, fast transient compensation range = 0 - 14 pF;
				Range16pF = 2,  ///< CC = 4.40 pF, fast transient compensation step size = 0.08 pF, fast transient compensation range = 0 - 16 pF;
				Range20pF = 3   ///< CC = 5.50 pF, fast transient compensation step size = 0.10 pF, fast transient compensation range = 0 - 20 pF;
			};

			/** This variable selects the magnitude of CC, the capacitor used to implement fast transient capacitive compensation.
			* This should be set to one of the Register8::CompensationRange values.  The step size selected by this value is multiplied by the
			* fastTransCapCompensation variable in Register6 to determine the value of capacitance to be compensated.
			*/
			uint16_t fastTransRange : 2;

            /** Setting this bit to zero shuts down the voltage amplifier and can be used to reduce power consumption when
              * current clamp mode is not used. Under normal operation this bit should be set to one.
              */
            uint16_t voltageAmpPower : 1;

            /// Unused
            uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register8();

			double getStepSize();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,9: Clamp Current Source
         *
         *  Be sure to set the current clamp enable bit in Register N,7 if you intend to use current clamp.
        */
        struct Register9 {
            /** \brief Magnitude of the current source.
              *
              * This variable sets the magnitude of the current source. This variable, along with clampCurrentSign,
              * must be updated at regular intervals to create desired waveforms. The step size of the current source is controlled by the
              * Registers N,10 through N,13.
              */
            uint16_t clampCurrentMagnitude : 7;

            /** \brief Positive or negative current.
              *
              * By convention, current is the flow of positive charges out of the chip.
              *
              * Setting this bit to one drives current (i.e., positive charges) out of the chip. This is considered positive current.
              * Setting this bit to zero pulls current into the chip. This is considered negative current.
              */
            uint16_t clampCurrentSign : 1;

			/// Unused
			uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register9();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }

            /// Convert a 16-bit integer to this register's value
            void setValue(int16_t value);
        };

        /** \brief %Register N,10: Clamp Current Source Scale (Negative Current, Coarse adjustment)
         *
         * %Registers N,10 - N,13 set the step size of the current source that is controlled by Register9. The positive and negative current scale
         * are set independently to allow for corrections of small current imbalances caused by device mismatch during chip fabrication.
         * Every coarse step is equal to approximately 65 fine steps.
         *
         * During the startup calibration routine, optimal values of the positive and negative coarse and fine parameters are measured for several
         * different magnitudes of current; subsequently these values are used depending on the current magnitude.
         */
        struct Register10 {
            /** \brief Coarse adjustment of the current source scale for negative currents
             *
             *  See Register10 for more information
             */
            uint16_t negativeCurrentScaleCoarse : 7;

            /// Unused
            uint16_t unused : 2;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register10();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,11: Clamp Current Source Scale (Negative Current, Fine adjustment)
        */
        struct Register11 {
            /** \brief Fine adjustment of the current source scale for negative currents
             *
             *  See Register10 for more information
             */
            uint16_t negativeCurrentScaleFine : 8;

            /// Unused
            uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register11();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,12: Clamp Current Source Scale (Positive Current, Coarse adjustment)
        */
        struct Register12 {
            /** \brief Coarse adjustment of the current source scale for positive currents
             *
             *  See Register10 for more information
             */
            uint16_t positiveCurrentScaleCoarse : 7;

            /// Unused
            uint16_t unused : 2;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register12();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register N,13: Clamp Current Source Scale (Positive Current, Fine adjustment)
        */
        struct Register13 {
            /** \brief Fine adjustment of the current source scale for positive currents
             *
             *  See Register10 for more information
             */
            uint16_t positiveCurrentScaleFine : 8;

            /// Unused
            uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            Register13();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };
        /** @} */

        /** \defgroup globalRegisters Global RAM Registers
        * \brief Per-chip registers
        *
        * In addition to the registers contained in each patch clamp unit, the CLAMP chip also contains four global RAM registers that set
        * various chip-wide parameters. These global registers are addressed by setting the top four bits of the address field to 15.
        *
        *   @{
        */

        /** \brief %Register 15,0: Temperature Sensor
         *
         *  The operation of the temperature sensor is detailed in the datasheet, and that logic is implemented in 
         *  TemperatureSensor::createTemperatureSenseCommands.
        */
        struct GlobalRegister0 {
            /** \brief Temperature sensor enable.
             *
             *  Setting this bit to one enables the on-chip temperature sensor. Power consumption may be reduced by setting this bit to
             *  zero to disable the sensor.
             */
            uint16_t tempen : 1;

            /** \brief Temperature sensor switch S1.
             *
             *  These bits control switches in the on-chip temperature sensor, whose output may be sampled by
             *  the ADC on channel 63. The detailed operation of the temperature sensor is described in the "Temperature Sensor" section
             *  in the datasheet. When the temperature sensor is not in use, these bits should each be set to zero.
             */
            uint16_t tempS1 : 1;

            /** \brief Temperature sensor switch S2.
             *
             * These bits control switches in the on-chip temperature sensor, whose output may be sampled by
             *  the ADC on channel 63. The detailed operation of the temperature sensor is described in the "Temperature Sensor" section
             *  in the datasheet. When the temperature sensor is not in use, these bits should each be set to zero.
             */
            uint16_t tempS2 : 1;

            /** \brief Temperature sensor switch S3.
             *
             * These bits control switches in the on-chip temperature sensor, whose output may be sampled by
             *  the ADC on channel 63. The detailed operation of the temperature sensor is described in the "Temperature Sensor" section
             *  in the datasheet. When the temperature sensor is not in use, these bits should each be set to zero.
             */
            uint16_t tempS3 : 1;

            /// Unused
            uint16_t unused : 5;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            GlobalRegister0();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register 15,1: Global Bias A
        */
        struct GlobalRegister1 {
            /** \brief Global Bias A.
             *
             *  This variable sets a set of bias currents that are used by circuit components across the chip. This variable
             *  should always be set to 86. This variable should be set before any self-calibration routines are performed on the patch clamp
             *  units.
             */
            uint16_t globalBiasA : 8;

            /// Unused
            uint16_t unused : 1;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            GlobalRegister1();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register 15,2: Global Bias B
        */
        struct GlobalRegister2 {
            /** \brief Global Bias B.
             *
             * This variable sets a set of bias currents that are used by circuit components across the chip. This variable
             *  should always be set to 29. This variable should be set before any self-calibration routines are performed on the patch clamp
             *  units.
             */
            uint16_t globalBiasB : 6;

            /// Unused
            uint16_t unused : 3;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            GlobalRegister2();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /** \brief %Register 15,3: ADC Configuration; Auxiliary Digital Output
        */
        struct GlobalRegister3 {
            /// Enum that controls when convert pulse is sent to the off-chip ADC.  See GlobalRegister3::adcTiming for information.
            enum ConvertTiming {
                Falling25 = 0, ///< falling edge of 25th SCLK pulse
                Falling26,     ///< falling edge of 26th SCLK pulse
                Falling27,     ///< falling edge of 27th SCLK pulse
                Falling28,     ///< falling edge of 28th SCLK pulse
                Falling29,     ///< falling edge of 29th SCLK pulse
                Falling30,     ///< falling edge of 30th SCLK pulse
                Falling31,     ///< falling edge of 31st SCLK pulse
                Falling32      ///< falling edge of 32nd SCLK pulse
            };

            /** \brief Is a 16-bit ADC or an 18-bit ADC connected to the CLAMP chip?
             *
             *  If an 18-bit ADC is connected to the CLAMP chip, this bit must be set to one for proper operation. If a 16-bit ADC is
             *  connected, this bit must be set to zero.
             */
            uint16_t is18bitADC : 1;

            /** \brief Controls when convert pulse is sent to the off-chip ADC
             *
             *  This variable adjusts the timing of the rising edge of the ADC_CNV pulse sent to the off-chip ADC. The input
             *  to the ADC is sampled at this time. (The falling edge of the ADC_CNV pulse always occurs on the falling edge of the 7th SCLK
             *  pulse; the analog MUX switches on the falling edge of the 8th SCLK pulse.)
             *
             *  The user must ensure that the ADC_CNV pulse is held low for a sufficiently long period of time to satisfy the timing specifications
             *  of the ADC used with the CLAMP chip.
             *
             *  Note that during a WRITE/CONVERT command, the new register value is updated on the falling edge of the 30th SCLK pulse. By
             *  adjusting the value of the ADC timing variable, the ADC can be made to sample just before, just after, or coincident with the
             *  register update.
             *
             *  This variable should be set to one of the GlobalRegister3::ConvertTiming enum values, typically Falling30.
             */
            uint16_t adcTiming : 3;

            /** \brief Weak MISO
             *
             *  If this bit is set to zero, the MISO line goes to high impedance mode (HiZ) when CS is pulled high, allowing multiple
             *  chips to share the same MISO line, so long as only one of their chip select lines is activated at any time. 
             *
             *  If only one CLAMP chip will be using a MISO line, this bit may be set to one, and when CS is pulled high the MISO line will be 
             *  driven weakly by the chip.  This can prevent the line from drifting to indeterminate values between logic high and logic low. 
             *  This pin has no effect in LVDS communication mode.
             */
            uint16_t weakMISO : 1;

            /** \brief Auxiliary Digital Output value
             *
             *  This bit is driven out of the auxiliary CMOS digital output pin auxout, provided that the digout HiZ bit is set to zero. See
             *  the "Auxiliary Digital Output" section of the datasheet for details.
             *
             *  This value must be used together with GlobalRegister3::digoutHiZ to control the output.
             */
            uint16_t digout : 1;

            /** \brief Enable/disable Auxiliary Digital Output
             *
             *  The CLAMP chips have an auxiliary digital output pin auxout that may be used to activate off-chip circuitry (e.g.,
             *  MOSFET switches, LEDs, stimulation circuits). Setting this bit to one puts the digital output into high impedance (HiZ) mode.
             *
             *  This value must be used together with GlobalRegister3::digout to control the output.
             */
            uint16_t digoutHiZ : 1;

            /// Unused
            uint16_t unused : 2;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            GlobalRegister3();

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };
        /** @} */

        /** \brief A generic 9-bit register on the CLAMP chip
         *
         * This struct is used by code that deals with all different register types.  A given register (e.g., Register3) will
         * be cast to this type, and this type is used in the generic code.  For actually setting registers, you should use the
         * more specific types above.
         *
         * Example:
         * \code{.cpp}
             Register3 r3;
             Register& r = reinterpret_cast<Register&>(r3);
             return createWriteCommand(channel, address, r.value);
         * \endcode
         */
        struct Register {
            /// The 9-bit register value
            uint16_t value : 9;
            /// Padding - CLAMP uses 9-bit registers, but we can only use 8 or 16 in software.
            uint16_t padding : 7;

            /// Convert register value to a 16-bit integer
            operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
        };

        /// Enum used in addressing registers
        enum Unit {
            Unit0 = 0,    ///< %Register values for %Channel 0
            Unit1,        ///< %Register values for %Channel 1
            Unit2,        ///< %Register values for %Channel 2
            Unit3,        ///< %Register values for %Channel 3
            Global = 15   ///< Global %Register values
        };

        /// \cond private
        // These two methods would just be private methods in IndexedRegister, but that class is templated, and we don't want
        // to *define* them here, just *declare* them here, hence the weirdness.
        class IndexedRegisterImpl {
        protected:
            WaveformControl::NonrepeatingCommand createWriteCommand(uint8_t channel, uint8_t index, uint16_t value);
            WaveformControl::NonrepeatingCommand createWriteConvertCommand(uint8_t channel, uint8_t index, ChipProtocol::MuxSelection mux, uint16_t value);
        };
        /// \endcond

        /** \brief A register that knows its own address.
         *
         *  After setting these up initially, you simply change the value of the underlying register (IndexedRegister::value),
         *  then call writeCommand() or writeConvertCommand() to generate a command with the appropriate bits to write to the
         *  given register with the given value.
         */
        template <class T>
        struct IndexedRegister : private IndexedRegisterImpl {
            /// %Channel index (should correspond to one of the values of ::Unit)
            uint8_t channel;
            /// %Register index.  E.g., register 5 would have 5 here.
            uint8_t address;
            /** \brief Register value
             *
             *  The type is one of:
             *   - a per-channel register (Register0 - Register13)
             *   - a global register (GlobalRegister0 - GlobalRegister3)
             *   - a Register (for ROM registers with no other internal structure)
             */
            T value;

            /// Creates a WRITE command to the given register with the given value
            /// \returns A NonrepeatingCommand object that contains the constructed command.
            WaveformControl::NonrepeatingCommand writeCommand() {
                char* tmp = reinterpret_cast<char*>(&value);
                Register r = *reinterpret_cast<Register*>(tmp);
                return createWriteCommand(channel, address, r.value);
            }

            /** \brief Creates a WRITE/CONVERT command to the given register with the given value
             *  \param[in]   mux     %Mux to convert
             *  \returns A NonrepeatingCommand object that contains the constructed command.
             */
            WaveformControl::NonrepeatingCommand writeConvertCommand(ChipProtocol::MuxSelection mux) {
                char* tmp = reinterpret_cast<char*>(&value);
                Register r = *reinterpret_cast<Register*>(tmp);
                return createWriteConvertCommand(channel, address, mux, r.value);
            }

            static_assert(sizeof(T) == sizeof(Register), "Invalid type T");
        };

        /// Struct storing \ref unitRegisters (Register0 - Register13).
        struct ChannelRegisters {
            /// %Register 0
            IndexedRegister<Register0> r0;
            /// %Register 1
            IndexedRegister<Register1> r1;
            /// %Register 2
            IndexedRegister<Register2> r2;
            /// %Register 3
            IndexedRegister<Register3> r3;
            /// %Register 4
            IndexedRegister<Register4> r4;
            /// %Register 5
            IndexedRegister<Register5> r5;
            /// %Register 6
            IndexedRegister<Register6> r6;
            /// %Register 7
            IndexedRegister<Register7> r7;
            /// %Register 8
            IndexedRegister<Register8> r8;
            /// %Register 9
            IndexedRegister<Register9> r9;
            /// %Register 10
            IndexedRegister<Register10> r10;
            /// %Register 11
            IndexedRegister<Register11> r11;
            /// %Register 12
            IndexedRegister<Register12> r12;
            /// %Register 13
            IndexedRegister<Register13> r13;

            ChannelRegisters();

            void setChannelIndex(uint8_t value);

            IndexedRegister<Register>& get(uint8_t index);
        };

        /// Struct storing per-chip global registers (\ref globalRegisters and ROM registers)
        struct GlobalRegisters {
            /// %Register 0
            IndexedRegister<GlobalRegister0> r0;
            /// %Register 1
            IndexedRegister<GlobalRegister1> r1;
            /// %Register 2
            IndexedRegister<GlobalRegister2> r2;
            /// %Register 3
            IndexedRegister<GlobalRegister3> r3;
            /// Unused
            IndexedRegister<Register> unused[3];
            /** \brief %Registers 15,7 - 15,11: Company Designation
             *
             *  The read-only registers 15,7 through 15,11 contain the characters INTAN in ASCII. 
             *  The contents of these registers can be read to verify the fidelity of the SPI interface.
             */
            IndexedRegister<Register> company[5];
            /** \brief %Register 15,12: Future Expansion
             *
             *  This register is reserved for future expansion. Its current value is zero.
             */
            IndexedRegister<Register> futureExpansion;
            /** \brief %Register 15,13: Die Revision
             *
             *  This read-only variable encodes a die revision number which is set by Intan Technologies to encode various versions of a chip.
             */
            IndexedRegister<Register> dieRevision;
            /** \brief %Register 15,14: Number of Patch Clamp Units
             *
             *  This read-only variable encodes the total number of patch clamp units on the chip. This register is set to 4 for the CLAMP chip.
             */
            IndexedRegister<Register> numUnits;
            /** \brief %Register 15,15: Intan Technologies %Chip ID
             *
             *  This read-only variable encodes a unique Intan Technologies ID number indicating the type of chip. The chip ID for the CLAMP chip is 128.
             */
            IndexedRegister<Register> chipId;

            GlobalRegisters();
            IndexedRegister<Register>& get(uint8_t index);
            std::wstring getCompanyDesignation() const;
        };
    }
}
