#pragma once

#include <vector>

namespace CLAMP {
    /** \brief Signal processing functionality (filtering, data analysis algorithms, etc.)
     *
     *  Doesn't really depend on any of the CLAMP functionality.
     */
    namespace SignalProcessing {
        /** \brief Base class for all filtering
         *
         *  Establishes the public interface for filtering.  Specific filter types implement this in different ways,
         *  but calling code can just refer to this interface to do its filtering.
         */
        class Filter {
        public:
            virtual ~Filter() {}

            /** \brief Filter a vector of data
             *
             *  Filtering a vector of data is implemented here in terms of pointwise filtering (filterOne()).  So any subclass
             *  will automatically inherit this ability.
             *
             *  \param[in] in The input data.
             *  \param[out] out The next filtered data.
             */
            void filter(const std::vector<double>& in, std::vector<double>& out);

            /** \brief Filter one data point.
             *
             *  One key functionality is a pointwise filter (i.e., data comes in time step by time step and is filtered
             *  point by point).  This function is the interface for that; different subclasses implement it differently.
             *
             *  \param[in] in The next input point.
             *  \returns The next filtered point.
             */
            virtual double filterOne(double in) = 0;

            /** \brief Reset filter state
             *
             *  Should return the filter to a state equivalent to if it had just been constructed.  For example, if you
             *  start filtering a new set of data, you want to return to the initial state using this function.  (Alternately,
             *  you could just create a new instance of the specific filter object.  But that would require that code that
             *  receives data to know what types of filters are being used; this function allows separation of filtering type/configuration
             *  and filter execution.)
             */
            virtual void reset() = 0;
        };

        /** \brief Implements a digital biquad filter
         *
         *  Implements a biquadratic filter with a transfer function
         *
         \code
                   a[0] + a[1] z^-1 + a[2] z^-2
           H(z)  = ----------------------------
                   b[0] + b[1] z^-1 + b[2] z^-2
         \endcode
         *
         * Coefficients are normalized so that b[0] = 1.
         *
         * Biquadratic is a second order recursive linear filter.
         *
         * The filter stores previous values of input and output to produce the next value.
         *
         * Subclasses need to set \ref a and \ref b appropriately.
         */
        class BiquadFilter : public Filter {
        public:
            /// Constructor
            BiquadFilter();

            /// \copydoc Filter::filterOne
            double filterOne(double in) override;

            /// \copydoc Filter::reset
            void reset() override;

        protected:
            /// 'a' coefficients (see transfer function above)
            double a[3];
            /// 'b' coefficients (see transfer function above)
            double b[3];

        private:
            // Previous values of input/output
            double prevIn[2];
            double prevOut[2];
            // Index lets us know if we're at time 0 or 1 (before prevIn and prevOut are valid)
            unsigned int index;
        };

        /// Second order Bessel Low Pass Filter
        class SecondOrderBesselLowPassFilter : public BiquadFilter {
        public:
            SecondOrderBesselLowPassFilter(double fc, double q, double ts);
        };

        /// Second order Bessel High Pass Filter
        class SecondOrderBesselHighPassFilter : public BiquadFilter {
        public:
            SecondOrderBesselHighPassFilter(double fc, double q, double ts);
        };

        /** \brief N-th order Bessel Low Pass Filter
         *
         *  Currently only implemented for N = 4, 6, 8.
         *
         *  Implemented as a series of SecondOrderBesselLowPassFilter instances.
         */
        class NthOrderBesselLowPassFilter : public Filter {
        public:
            NthOrderBesselLowPassFilter(unsigned int order, double fc, double ts);

            /// \copydoc Filter::filterOne
            double filterOne(double in) override;

            /// \copydoc Filter::reset
            void reset() override;

        private:
            std::vector<SecondOrderBesselLowPassFilter> filters;
        };
    }
}
