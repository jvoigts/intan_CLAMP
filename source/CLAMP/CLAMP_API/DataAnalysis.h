#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace CLAMP {
    namespace SignalProcessing {
        /// Miscellaneous data analysis/numerical routines
        class DataAnalysis {
        public:
            static double average(std::vector<int32_t>::const_iterator begin, std::vector<int32_t>::const_iterator end);
            static double average(std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end);
            static double slope(std::vector<double>::const_iterator xbegin, std::vector<double>::const_iterator xend, std::vector<double>::const_iterator ybegin, std::vector<double>::const_iterator yend);
            static double pearson(std::vector<double>::const_iterator xbegin, std::vector<double>::const_iterator xend, std::vector<double>::const_iterator ybegin, std::vector<double>::const_iterator yend);
            static double calculateBestResidual(std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end);
            static int32_t calculateBestResidual(std::vector<int32_t>::const_iterator begin, std::vector<int32_t>::const_iterator end);
            static std::pair<unsigned int, int32_t> calculateBestTrim(std::vector<int32_t>& values);
        };

        /** \brief Numerical routines used in fitting an exponential distribution to data.
         *
         *  This uses the Levenberg-Marquardt algorithm.  It takes inspiration from Numerical Recipes in C, and from the
         *  Wikipedia entry on the algorithm.  The variable names largely match the latter.
         *
         *  A general implementation of the LM algorithm would be much more complicated - it would involve general matrix
         *  inverse, for example, and be parameterized to pass in a different function to fit.  This is *not* a general
         *  implementation - it assumes you're only fitting 3 parameters, and so the inverse function is hardwired for
         *  a 3x3 matrix, and various internal variables are hardwired to arrays of length 3.  Moreover, the inverse of
         *  the 3x3 matrix is implemented using determinants (which themselves are hardwired).  A more general
         *  implementation beyond 3x3 would use the appropriate numerical methods.
         */
        class ExponentialFit {
        public:
            static void lm(const std::vector<double>& xs, const std::vector<double>& ys, /* out: */ double beta[3], double& chi2);
            static double f(double x, double beta[3]);

            /* These functions are essentially private.  But, because the logic of lm is sufficiently complicated, I made
             * them public so I could write unit tests against them.
             */
            /// \cond private

            // Determinant of 2x2 sub-matrix of a 3x3 matrix
            static double determinant2(const double A[3][3], const unsigned int xs[2], const unsigned int ys[2]);

            // Determinant of 3x3 matrix
            static double determinant3(const double A[3][3]);

            // Inverse of a 3x3 matrix
            static void inverse(const double A[3][3], double Ainv[3][3]);

            // Returns the Jacobian (gradient of f with respect to beta), evaluated at each x[i]
            static std::vector<std::array<double,3>> getJ(const std::vector<double>& xs, double beta[3]);
            // Returns a vector where v[i] = { y[i] - predicted_y[i] }
            static std::vector<double> getDifference(const std::vector<double>& xs, const std::vector<double>& ys, double beta[3]);
            // One step of the Levenberg-Marquardt algorithm
            static bool lmOneStep(double& lambda, double beta[3], const std::vector<double>& xs, const std::vector<double>& ys, double& chi2);

            /// \endcond

        private:
            static void gradientOfF(double x, double beta[3], std::array<double, 3>& result);
        };
    }
}
