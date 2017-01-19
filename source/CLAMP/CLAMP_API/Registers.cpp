#include "Registers.h"
#include <stdexcept>
#include "common.h"
#include "WaveformCommand.h"
#include <cmath>

using std::wstring;
using std::runtime_error;
using std::deque;
using std::unique_ptr;
using std::invalid_argument;
using namespace CLAMP::ChipProtocol;
using namespace CLAMP::WaveformControl;

namespace CLAMP {
    namespace Registers {
        //--------------------------------------------------------------------------------------------------
        /// \cond private
        NonrepeatingCommand IndexedRegisterImpl::createWriteCommand(uint8_t channel, uint8_t index, uint16_t value) {
            uint8_t A = (channel << 4) | index;
            return NonrepeatingCommand::create(WRITE, None, A, value);
        }

        NonrepeatingCommand IndexedRegisterImpl::createWriteConvertCommand(uint8_t channel, uint8_t index, MuxSelection mux, uint16_t value) {
            uint8_t A = (channel << 4) | index;
            return NonrepeatingCommand::create(WRITE_AND_CONVERT, mux, A, value);
        }

		/// \endcond

        //--------------------------------------------------------------------------------------------------
        ChannelRegisters::ChannelRegisters()
        {
            for (unsigned int i = 0; i < 14; i++) {
                IndexedRegister<Register>& r = get(i);
                r.address = i;
            }
            setChannelIndex(0);
        }

        /** \brief Set which channel this is (0-3)
         *
         *  \param[in] value     %Channel [0-3]
         */
        void ChannelRegisters::setChannelIndex(uint8_t value) {
            for (unsigned int i = 0; i < 14; i++) {
                get(i).channel = value;
            }
        }

        /** \brief Gets a register by index
         *
         *  \param[in] index    Which register [0-13]
         *  \returns The register as the generic type IndexedRegister<Register>&
         */
        IndexedRegister<Register>& ChannelRegisters::get(uint8_t index) {
            if (index >= 14) {
                throw invalid_argument("Register index is not valid");
            }
            IndexedRegister<Register>* p = reinterpret_cast<IndexedRegister<Register>*>(this);
            return p[CheckBits(index, 4)];
        }

        //--------------------------------------------------------------------------------------------------
        GlobalRegisters::GlobalRegisters() {
            for (unsigned int i = 0; i < 16; i++) {
                IndexedRegister<Register>& r = get(i);
                r.channel = Unit::Global;
                r.address = i;
            }
            // And zero-out the ROM, for good measure
            for (unsigned int i = 7; i < 16; i++) {
                IndexedRegister<Register>& r = get(i);
                char* tmp = reinterpret_cast<char*>(&r.value);
                *reinterpret_cast<uint16_t*>(tmp) = 0;
            }
        }

        /** \brief Gets a global register by index
         *
         *  \param[in] index    Which register [0-15]
         *  \returns The register as the generic type IndexedRegister<Register>&
         */
        IndexedRegister<Register>& GlobalRegisters::get(uint8_t index) {
            IndexedRegister<Register>* p = reinterpret_cast<IndexedRegister<Register>*>(this);
            return p[CheckBits(index, 4)];
        }

        /** \brief Gets the company designation (%Registers 15,7 - 15,11) as a string
         *
         *  Note that the return type is std::wstring, as registers on the chip are 9 bits.  A std::string,
         *  which contains 8-bit characters, would be able to store the correct value, but might lose information
         *  if an incorrect value was present with the MSB set.  Since the correctness of the
         *  returned value is used to check the integrity of the communcations protocol with the chip, std::wstring is
         *  the logical choice.
         *
         *  \returns String containing the company designation.  Should be "INTAN" for valid chips.
         */
        wstring GlobalRegisters::getCompanyDesignation() const {
            wstring result;
            for (unsigned int index = 0; index < 5; index++) {
                const IndexedRegister<Register>& r = reinterpret_cast<const IndexedRegister<Register>&>(company[index]);
                result += r.value.value;
            }
            return result;
        }

        //--------------------------------------------------------------------------------------------------
        // Individual registers follow
        //--------------------------------------------------------------------------------------------------

        Register0::Register0() : clampVoltageMagnitude(0), clampVoltagePlusSign(1), padding(0) {
        }

        void Register0::setValue(int16_t value) {
            clampVoltageMagnitude = CheckBits(abs(value), 8);
            clampVoltagePlusSign = (value >= 0);
        }

        Register1::Register1() : clampVoltageTimeConstantA(20), clampStepSize(1), clampDACPower(1), unused(0), padding(0) {
        }

        Register2::Register2() : clampVoltageOffsetTrim(32), unused(0), padding(0) {
        }

        Register3::Register3() : feedbackResistance(Resistance::R200k), clampVoltageTimeConstantB(10), unused(0), padding(0) {
        }

        /** \brief Converts a Resistance enum to a resistance in Ohms.

            @param[in]  r    Resistance enum.
            @returns Resistance in ohms.
        */
        double Register3::nominalResistance(Resistance r) {
            switch (r)
            {
            case Register3::R200k:
                return 200e3;
            case Register3::R2M:
                return 2e6;
            case Register3::R20M:
                return 20e6;
            case Register3::R40M:
                return 40e6;
			case Register3::R80M:
				return 80e6;
            default:
                throw invalid_argument("Must be a legitimate resistor value");
                break;
            }
        }

        /// Feedback resistance, in Ohms.
        double Register3::nominalResistance() const {
            Register3::Resistance r = resistanceEnum();
            return nominalResistance(r);
        }

        /// Feedback resistance, as a Resistance enum.
        Register3::Resistance Register3::resistanceEnum() const {
            return static_cast<Register3::Resistance>(feedbackResistance);
        }

        Register4::Register4() : feedbackCapacitance(0xff), voltageClampPower(1), padding(0) {
        }

        /// Feedback capacitance, in Farads.
        double Register4::capacitance() const {
            return 0.2e-12 * feedbackCapacitance;
        }

        /** \brief Sets the feedback capacitance

            @param[in]  c    Desired feedback capacitance, in Farads.  The actual value is set to be the closest 
                             achievable value, and can be queried via the capacitance() function.  Range 0..51e-12.
        */
        void Register4::setFeedbackCapacitance(double c) {
            uint8_t value;
            double desiredValue = round(c / 0.2e-12);
            if (desiredValue > 255) {
                value = 255;
            }
            else if (desiredValue < 1) {
                value = 1;
            }
            else {
                value = static_cast<uint8_t>(desiredValue);
            }
            feedbackCapacitance = CheckBits(value, 8);
        }


        Register5::Register5() : diffAmpOffsetTrim(32), diffAmpInMinusSelect(0), diffAmpInPlusSelect(0), unused(0), padding(0) {
        }

        Register6::Register6() : fastTransCapCompensation(55), fastTransInSelect(0), padding(0) {
        }

        /** \brief Sets the magnitude of fast transient compensation.
         *
         *  \param[in] magnitude        Amount of capacitive compensation in F.  Granularity is set by fastTransRange in Register N,8.
         */
        void Register6::setMagnitude(double magnitude, double stepSize) {
            // value = magnitude/stepSize + 55
            // so a value of 55 corresponds to 0 pF compensation, and 255 corresponds to maximum compensation.
            long lvalue = lround(magnitude / stepSize) + 55;

            // Invalid argument.  Check it here, rather than on the raw magnitude, because comparisons of floating point numbers are dangerous
            if (lvalue < 55 || lvalue > 255) {
                throw invalid_argument("Invalid capacitive compensation register value.");
            }


            uint8_t value;
            if (lvalue < 0) {
                value = 0;
            }
            else if (lvalue > 255) {
                value = 255;
            }
            else {
                value = static_cast<uint8_t>(lvalue);
            }
            fastTransCapCompensation = value;
        }

        /** \brief Gets the magnitude of fast transient compensation.
         *
         *  \returns Amount of capacitive compensation in step sizes [0..200].  Granularity is set by fastTransRange in Register8.
         */
        int Register6::getMagnitude() {
            return (fastTransCapCompensation - 55);
        }

        Register7::Register7() : fastTransTimeConstant(10), fastTransConnect(0), voltageClampConnect(0), clampCurrentEnable(0), fastTransPower(1), padding(0) {
        }

        Register8::Register8() : inputSelect(InputSelect::Open0), buzzSwitch(BuzzState::BuzzOff), fastTransRange(CompensationRange::Range20pF), voltageAmpPower(1), unused(0), padding(0) {
        }

		double Register8::getStepSize() {
			switch (fastTransRange)
			{
			case Register8::Range10pF:
				return 0.05e-12;
			case Register8::Range14pF:
				return 0.07e-12;
			case Register8::Range16pF:
				return 0.08e-12;
			case Register8::Range20pF:
				return 0.10e-12;
			default:
				throw invalid_argument("Must be a legitimate compensation range.");
				break;
			}
		}

        Register9::Register9() : clampCurrentMagnitude(0), clampCurrentSign(0), unused(0), padding(0) {
        }

        void Register9::setValue(int16_t value) {
            clampCurrentMagnitude = CheckBits(abs(value), 7);
            clampCurrentSign = (value >= 0);
        }

        Register10::Register10() : negativeCurrentScaleCoarse(0), unused(0), padding(0) {
        }

        Register11::Register11() : negativeCurrentScaleFine(67), unused(0), padding(0) {
        }

        Register12::Register12() : positiveCurrentScaleCoarse(0), unused(0), padding(0) {
        }

        Register13::Register13() : positiveCurrentScaleFine(67), unused(0), padding(0) {
        }

        GlobalRegister0::GlobalRegister0() : tempen(0), tempS1(0), tempS2(0), tempS3(0), unused(0), padding(0) {
        }

        GlobalRegister1::GlobalRegister1() : globalBiasA(86), unused(0), padding(0) {
        }

        GlobalRegister2::GlobalRegister2() : globalBiasB(29), unused(0), padding(0) {
        }

        GlobalRegister3::GlobalRegister3() : is18bitADC(1), adcTiming(ConvertTiming::Falling30), weakMISO(1), digout(0), digoutHiZ(1), unused(0), padding(0) {
        }
    }
}
