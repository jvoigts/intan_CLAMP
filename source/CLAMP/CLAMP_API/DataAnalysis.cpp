#include "DataAnalysis.h"
#include <algorithm>
#include "common.h"

using std::vector;
using std::pair;
using std::array;

namespace CLAMP {
    namespace SignalProcessing {
        /** \brief Average (arithmetic mean) of integer data
         *
         *  For generality, this takes iterator begin and end inputs.  That means if you have a large array and
         *  want to take the average of a subset (e.g., the second half, after the transient has died out), you
         *  can do that without having to copy the data.
         *
         *  \param[in] begin   Iterator where the data begins
         *  \param[in] end     Iterator where the data ends
         *  \returns The calculated average.
         */
        double DataAnalysis::average(vector<int32_t>::const_iterator begin, vector<int32_t>::const_iterator end) {
            int32_t sum = 0;
            for (vector<int32_t>::const_iterator iter = begin; iter != end; iter++) {
                sum += *iter;
            }
            return 1.0 * sum / (end - begin);
        }

        /** \brief Average (arithmetic mean) of double precision data
         *
         *  \details \copydetails DataAnalysis::average(std::vector<int32_t>::const_iterator,std::vector<int32_t>::const_iterator)
         */
        double DataAnalysis::average(vector<double>::const_iterator begin, vector<double>::const_iterator end) {
            double sum = 0;
            for (vector<double>::const_iterator iter = begin; iter != end; iter++) {
                sum += *iter;
            }
            return 1.0 * sum / (end - begin);
        }

        /** \brief Calculates slope, given x and y vectors.
         *
         *  The slope is calculated using least squares, which gives the formula
         \code
                    sum[ (x-x_avg)*(y-yavg) ]
            slope = -------------------------
                        sum[ (x-x_avg)^2 ]
         \endcode
         *
         *  For generality, this takes iterator begin and end inputs.  That means if you have a large array and
         *  want to take the slope of a subset (e.g., the second half, after the transient has died out), you
         *  can do that without having to copy the data.
         *
         *  \param[in] xbegin   Iterator where the x data begins
         *  \param[in] xend     Iterator where the x data ends
         *  \param[in] ybegin   Iterator where the y data begins
         *  \param[in] yend     Iterator where the y data ends
         *  \returns The calculated slope.
         */
        double DataAnalysis::slope(vector<double>::const_iterator xbegin, vector<double>::const_iterator xend, vector<double>::const_iterator ybegin, vector<double>::const_iterator yend) {
            double xavg = average(xbegin, xend);
            double yavg = average(ybegin, yend);

            double num = 0, den = 0;
            for (auto xiter = xbegin, yiter = ybegin; xiter != xend && yiter != yend; xiter++, yiter++) {
                num += (*xiter - xavg) * (*yiter - yavg);
                den += (*xiter - xavg) * (*xiter - xavg);
            }

            return num / den;
        }

        /** \brief Calculates the pearson r coefficient, given x and y vectors.
         *
         *  The pearson coefficient (r) is calculated as
         \code
                      sum[ (x-x_avg)*(y-yavg) ]
            r = -------------------------------------------
                sqrt(sum[ (x-x_avg)^2 ] sum[ (y-y_avg)^2 ])
         \endcode
         *
         *  For generality, this takes iterator begin and end inputs.  That means if you have a large array and
         *  want to take the slope of a subset (e.g., the second half, after the transient has died out), you
         *  can do that without having to copy the data.
         *
         *  \param[in] xbegin   Iterator where the x data begins
         *  \param[in] xend     Iterator where the x data ends
         *  \param[in] ybegin   Iterator where the y data begins
         *  \param[in] yend     Iterator where the y data ends
         *  \returns The calculated pearson r coefficient.
         */
        double DataAnalysis::pearson(vector<double>::const_iterator xbegin, vector<double>::const_iterator xend, vector<double>::const_iterator ybegin, vector<double>::const_iterator yend) {
            double xavg = average(xbegin, xend);
            double yavg = average(ybegin, yend);

            double num = 0, xsq = 0, ysq = 0;
            for (auto xiter = xbegin, yiter = ybegin; xiter != xend && yiter != yend; xiter++, yiter++) {
                num += (*xiter - xavg) * (*yiter - yavg);
                xsq += (*xiter - xavg) * (*xiter - xavg);
                ysq += (*yiter - yavg) * (*yiter - yavg);
            }

            return num / sqrt(xsq * ysq);
        }

        /** \brief Calculated best residual of double precision data
         *
         *  It's frequently useful to find the steady state value that a vector tends toward, but ignoring the initial transient.
         *  This function finds that residual value, ignoring the first half of the data.
         *
         *  For generality, this takes iterator begin and end inputs.  That means if you have a large array and
         *  want to take the residual of a subset (e.g., the second half, after the transient has died out), you
         *  can do that without having to copy the data.
         *
         *  \param[in] begin   Iterator where the data begins
         *  \param[in] end     Iterator where the data ends
         *  \returns The calculated residual.
         */
        double DataAnalysis::calculateBestResidual(vector<double>::const_iterator begin, vector<double>::const_iterator end) {
            // Use the second half of the points
            begin += (end - begin) / 2;

            return average(begin, end);
        }

        /** \brief Calculated best residual of integer data
         *
         *  \details \copydetails DataAnalysis::calculateBestResidual(std::vector<double>::const_iterator,std::vector<double>::const_iterator)
         */
        int32_t DataAnalysis::calculateBestResidual(vector<int32_t>::const_iterator begin, vector<int32_t>::const_iterator end) {
            // Use the second half of the points
            begin += (end - begin) / 2;

            return lround(average(begin, end));
        }

        /** \brief Returns best trim index and the residual when you use that index
         *
         *  For various calibration routines, we have a trim setting and we'll measure the error value at
         *  trim = 0, trim = 1, trim = 2, etc.
         *
         *  This function finds the least error value, and returns both that value and the trim that produced it.
         *
         *  \param[in] values   Values of error/residual at trim = 0, 1, 2, etc.
         *  \returns A pair of the form (best trim index, best residual value).
         */
        pair<unsigned int, int32_t> DataAnalysis::calculateBestTrim(vector<int32_t>& values) {
            // Find the best one
            auto pBestResidual = std::min_element(values.begin(), values.end(),
                [](const int32_t& a, const int32_t& b) {
                return std::abs(a) < std::abs(b);
            });

            pair<unsigned int, int32_t> result;
            result.first = pBestResidual - values.begin();
            result.second = *pBestResidual;
            return result;
        }

        //----------------------------------------------------------------------------------------------------------------

        /// \cond private
        double ExponentialFit::determinant2(const double A[3][3], const unsigned int xs[2], const unsigned int ys[2]) {
            // Matrix of the form
            //   a  b
            //   c  d
            double a = A[ys[0]][xs[0]];
            double b = A[ys[0]][xs[1]];
            double c = A[ys[1]][xs[0]];
            double d = A[ys[1]][xs[1]];

            return a * d - b * c;
        }

        double ExponentialFit::determinant3(const double A[3][3]) {
            unsigned int i12[2] = { 1, 2 }, i02[2] = { 0, 2 }, i01[2] = { 0, 1 };

            double d00 = determinant2(A, i12, i12);
            double d01 = determinant2(A, i02, i12);
            double d02 = determinant2(A, i01, i12);
            return A[0][0] * d00 - A[0][1] * d01 + A[0][2] * d02;
        }

        void ExponentialFit::inverse(const double A[3][3], double Ainv[3][3]) {
            double d = determinant3(A);

            unsigned int others[3][2] = {
                { 1, 2 },
                { 0, 2 },
                { 0, 1 }
            };

            for (unsigned int y = 0; y < 3; y++) {
                for (unsigned int x = 0; x < 3; x++) {
                    int sign = ((x - y) % 2 == 0) ? 1 : -1;
                    Ainv[y][x] = sign * determinant2(A, others[y], others[x]) / d;
                }
            }

        }

        vector<array<double, 3>> ExponentialFit::getJ(const vector<double>& xs, double beta[3]) {
            vector<array<double, 3>> J(xs.size());

            for (unsigned int i = 0; i < xs.size(); i++) {
                gradientOfF(xs[i], beta, J[i]);
            }

            return J;
        }

        void ExponentialFit::gradientOfF(double x, double beta[3], array<double, 3>& grad) {
            /*  f(x) = C * exp(B * x) + A
                df / dA = 1
                df / dB = C * x * exp(B * x)
                df / dC = exp(B * x)

                grad = { df/dA, df/dB, df/dC }
                */

            // double A = beta[0]; -not used below
            double B = beta[1];
            double C = beta[2];

            grad[0] = 1;
            grad[1] = C * x * exp(B * x);
            grad[2] = exp(B * x);
        }

        vector<double> ExponentialFit::getDifference(const vector<double>& xs, const vector<double>& ys, double beta[3]) {
            vector<double> diff(xs.size());
            for (unsigned int i = 0; i < xs.size(); i++) {
                diff[i] = (ys[i] - f(xs[i], beta));
            }
            return diff;
        }
        /// \endcond

        /** \brief Returns f(x, beta)
         *
         * Where f is an exponential of the form
         \code
            f(x) = beta[2] * exp( beta[1] * x ) + beta[0]
         \endcode
         *
         * \param[in] x     X value (in our application, this is time, adjusted with t = 0 the time of the step)
         * \param[in] beta Fitted parameters (see above)
         * \returns f(x, beta)
         */
        double ExponentialFit::f(double x, double beta[3]) {
            //  f(x) = C * exp(B * x) + A

            double A = beta[0];
            double B = beta[1];
            double C = beta[2];

            return C * exp(B * x) + A;
        }

        /// \cond private
        bool ExponentialFit::lmOneStep(double& lambda, double beta[3], const vector<double>& xs, const vector<double>& ys, double& chi2) {
            vector<array<double, 3>> J = getJ(xs, beta);
            vector<double> diff = getDifference(xs, ys, beta);

            // J: n x 3
            // JtJ = J'*J;
            double JtJ[3][3] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0 };
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int j = 0; j < 3; j++) {
                    for (unsigned int k = 0; k < J.size(); k++) {
                        JtJ[i][j] += J[k][i] * J[k][j];
                    }
                }
            }

            // A = JtJ + lambda * diag(diag(JtJ));
            double A[3][3];
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int j = 0; j < 3; j++) {
                    A[i][j] = JtJ[i][j];
                }
                A[i][i] += lambda * JtJ[i][i];
            }

            // b = J'*diff;
            double b[3] = { 0, 0, 0 };
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int k = 0; k < J.size(); k++) {
                    b[i] += J[k][i] * diff[k];
                }
            }

            // Now, A * delta = b
            // So let delta = inverse(A) * b
            double Ainv[3][3];
            inverse(A, Ainv);

            double delta[3] = { 0, 0, 0 };
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int j = 0; j < 3; j++) {
                    delta[i] += Ainv[i][j] * b[j];
                }
            }


            // new_beta = beta + delta
            double new_beta[3];
            for (unsigned int i = 0; i < 3; i++) {
                new_beta[i] = beta[i] + delta[i];
            }

            vector<double> new_diff = getDifference(xs, ys, new_beta);

            // chi2 = diff'*diff / length(x)
            chi2 = 0;
            for (unsigned int i = 0; i < diff.size(); i++) {
                chi2 += diff[i] * diff[i];
            }
            chi2 /= diff.size();

            // new_chi2 = new_diff' * new_diff / length(x)
            double new_chi2 = 0;
            for (unsigned int i = 0; i < new_diff.size(); i++) {
                new_chi2 += new_diff[i] * new_diff[i];
            }
            new_chi2 /= new_diff.size();

            if (new_chi2 >= chi2) {
                if (lambda < 10) {
                    lambda = 10 * lambda;
                }
                return false;
            }
            else {
                if (lambda / 10 > 1e-10) {
                    lambda = lambda / 10;
                }
                for (unsigned int i = 0; i < 3; i++) {
                    beta[i] = new_beta[i];
                }
                return true;
            }
        }
        /// \endcond

        /** \brief Fits xs and ys with an exponential using the Levenberg-Marquardt optimization algorithm
         *
         *  This function applies the Levenberg-Marquardt algorithm to find *beta* values that minimize
         \code
            chi^2 = sum [ { y[i] - f(x[i], beta) } ^2 ]
         \endcode
         *
         * where f is an exponential of the form
         \code
            f(x) = beta[2] * exp( beta[1] * x ) + beta[0]
         \endcode
         *
         * \param[in] xs    X values (in our application, these are time, adjusted with t = 0 the time of the step)
         * \param[in] ys    Y values (in our application, these are measured voltages or currents)
         * \param[out] beta Fitted parameters (see above)
         * \param[out] chi2 chi^2 value
         */
        void ExponentialFit::lm(const std::vector<double>& xs, const std::vector<double>& ys, /* out: */ double beta[3], double& chi2) {
            // Initial guess
            beta[0] = ys.back();
            beta[1] = -3000;   // Corresponds to model cell parameters: Ra = 10 M, Rm = 500 M => Rp ~= 10M.  Cm = 33pF.  Tau = -1/RpCm ~= -3000
            beta[2] = ys.front() - ys.back();

            double lambda = 0.001;

            unsigned int consecutiveNonSteps = 0;

            unsigned int numSteps = 0;
            bool first = true;
            for (;;) {
                double old_chi2 = chi2;
                double old_beta[3] = { beta[0], beta[1], beta[2] };

                bool tookAStep = lmOneStep(lambda, beta, xs, ys, chi2);
                if (lambda < 1e-9) {
                    break;
                }
                if (tookAStep) {
                    consecutiveNonSteps = 0;
                    if (!first) {
                        double fracChangeChi2 = abs((chi2 - old_chi2) / old_chi2);
                        double fracChangeBeta = abs((beta[0] - old_beta[0]) / old_beta[0]) + abs((beta[1] - old_beta[1]) / old_beta[1]) + abs((beta[2] - old_beta[2]) / old_beta[2]);
                        if (fracChangeChi2 < 1e-6 && fracChangeBeta < 1e-6) {
                            break;
                        }
                    }
                }
                else {
                    consecutiveNonSteps++;
                    if (consecutiveNonSteps > 10) {
                        break;
                    }
                }
                numSteps++;
                if (numSteps > 200) {
                    break;
                }
                first = false;
            }
        }
    }
}


