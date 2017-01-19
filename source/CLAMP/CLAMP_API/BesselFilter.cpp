#include "BesselFilter.h"
#include <stdexcept>
#include "Constants.h"
#include "common.h"
#include <cmath>

using std::invalid_argument;
using std::vector;
using CLAMP::PI;

namespace CLAMP {
    namespace SignalProcessing {
        void Filter::filter(const vector<double>& in, vector<double>& out) {
            out.resize(in.size());
            for (unsigned int i = 0; i < in.size(); ++i) {
                out[i] = filterOne(in[i]);
            }
        }

        //----------------------------------------------------------------------------------------------------------------------------

        BiquadFilter::BiquadFilter() {
            reset();
        }

        double BiquadFilter::filterOne(double in) {
            double out;
            if (index == 0 || index == 1) {
                out = in;
                index++;  // We'll be a little blase' about index - we only care if it's 0, 1, or > 1.  So we only increment here, and don't worry about overflowing it
            }
            else {
                out = a[0] * in + a[1] * prevIn[0] + a[2] * prevIn[1]
                    - b[1] * prevOut[0] - b[2] * prevOut[1];
            }

            // If we get NaNs, don't pollute the filter
            if (!isnan(in) && !isnan(out)) {
                prevIn[1] = prevIn[0];
                prevIn[0] = in;
                prevOut[1] = prevOut[0];
                prevOut[0] = out;
            }

            return out;
        }

        void BiquadFilter::reset() {
            index = 0;
        }

        //----------------------------------------------------------------------------------------------------------------------------
        /** \brief Constructor
         *
         * \param[in] fc  Cutoff frequency, in Hz.
         * \param[in] q   Quality factor
         * \param[in] ts  Sampling time/period.
         */
        SecondOrderBesselLowPassFilter::SecondOrderBesselLowPassFilter(double fc, double q, double ts) {
            double fs = 1.0 / ts;
            double k = tan(PI * fc / fs);
            double norm = 1.0 / (1.0 + k / q + k * k);

            a[0] = k * k * norm;
            a[1] = 2.0 * a[0];
            a[2] = a[0];
            // b[0] is not used.  (It's always 1.)
            b[1] = 2.0 * (k * k - 1.0) * norm;
            b[2] = (1.0 - k / q + k * k) * norm;
        }

        //----------------------------------------------------------------------------------------------------------------------------
        /** \brief Constructor
         *
         * \param[in] fc  Cutoff frequency, in Hz.
         * \param[in] q   Quality factor
         * \param[in] ts  Sampling time/period.
         */
        SecondOrderBesselHighPassFilter::SecondOrderBesselHighPassFilter(double fc, double q, double ts) {
            double fs = 1.0 / ts;
            double k = tan(PI * fc / fs);
            double norm = 1.0 / (1.0 + k / q + k * k);

            a[0] = norm;
            a[1] = -2.0 * a[0];
            a[2] = a[0];
            // b[0] is not used.  (It's always 1.)
            b[1] = 2.0 * (k * k - 1.0) * norm;
            b[2] = (1.0 - k / q + k * k) * norm;
        }

        //----------------------------------------------------------------------------------------------------------------------------
        /** \brief Constructor
         *
         * \param[in] order Order of the filter (4, 6, 8)
         * \param[in] fc    Cutoff frequency, in Hz.
         * \param[in] ts    Sampling time/period.
         */
        NthOrderBesselLowPassFilter::NthOrderBesselLowPassFilter(unsigned int order, double fc, double ts) {
            /* The n-th order Bessel function can be factored into a n/2 second order functions of the form:
             *    s^2 + p * s + g
             *
             * Then, w0 = sqrt(g) and Q = w0/p.
             *
             * Finally, fc = w0/C, where C is the same for all the second order functions of a given order Bessel filter.
             * Specifically, for N = 4, C = 2.1138; for N = 6, C = 2.7034; for N = 8, C = 3.1796
             *
             * C is a correction factor for the fact that multiple filters will be applied.  If you find the value of w such
             * that H^2(w) = 0.5, that w is C.
             */
            switch (order) {
            case 4:
                filters = {
                    SecondOrderBesselLowPassFilter(1.6034 * fc, 0.8055, ts),
                    SecondOrderBesselLowPassFilter(1.4302 * fc, 0.5219, ts)
                };
                break;
            case 6:
                filters = {
                    SecondOrderBesselLowPassFilter(1.9047 * fc, 1.0233, ts),
                    SecondOrderBesselLowPassFilter(1.6892 * fc, 0.6112, ts),
                    SecondOrderBesselLowPassFilter(1.6039 * fc, 0.5103, ts)
                };
                break;
            case 8:
                filters = {
                    SecondOrderBesselLowPassFilter(2.1887 * fc, 1.2257, ts),
                    SecondOrderBesselLowPassFilter(1.9532 * fc, 0.7109, ts),
                    SecondOrderBesselLowPassFilter(1.8321 * fc, 0.5596, ts),
                    SecondOrderBesselLowPassFilter(1.7785 * fc, 0.5060, ts)
                };
                break;
            default:
                throw invalid_argument("Order is not valid");
            }
        }

        // This implementation and reset() could be pulled out into a "CompositeFilter" or "NthOrderFilter" class, with only
        // the constructor having Bessel-specific behavior.
        double NthOrderBesselLowPassFilter::filterOne(double in) {
            double out = in;
            for (SecondOrderBesselLowPassFilter& f : filters) {
                out = f.filterOne(out);
            }
            return out;
        }

        void NthOrderBesselLowPassFilter::reset() {
            for (SecondOrderBesselLowPassFilter& f : filters) {
                f.reset();
            }
        }
    }
}
