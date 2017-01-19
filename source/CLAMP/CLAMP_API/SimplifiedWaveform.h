#pragma once

#include <vector>
#include <cstdint>

class BinaryWriter;

namespace CLAMP {
    /** \brief One piece of a SimplifiedWaveform
     *
     *  During the coarse of one WaveformSegment, the applied value (i.e., voltage or current) is constant.
     */
    struct WaveformSegment {
        /** \brief When analyzing/plotting, which waveform is this segment logically part of.
         *
         *  For example, if the waveform is: 
         \code
            holding, +10 mV, +20 mV, +30 mV, holding, +10 mV, +20 mV, +30 mV, holding
         \endcode
         *  then you might label
             - holding as waveformNumber = 0
             - +10mV as waveformNumber = 1
             - +20mV as waveformNumber = 2
             - +30mV as waveformNumber = 3
         *
         *  This information is used in overlays, for example.
         *
         *  These should start at 0 and be consecutive.
         */
        unsigned int waveformNumber;

        /** \brief When analyzing/plotting, which index within the waveform is this segment logically part of.
         *
         *   - For a simple, one-waveformNumber waveform, this would be 0, 1, 2, ...
         *   - For a more complex waveform, all the segments with waveformNumber 0 would go 0, 1, 2, ..., all the
         *     segments with waveformNumber 1 would go 0, 1, 2, ..., etc.
        */
        unsigned int indexWithinWaveform;

        /** \brief When analyzing/plotting, used to specify a 'virtual time' of overlaid waveforms.
         *
         *  For example, suppose the waveform is:
             - t = 0, holding
             - t = 100 ms, +10 mV
             - t = 200 ms, +20 mV
             - t = 300 ms, +30 mV
             - t = 400 ms, holding
         *
         * To plot, you'd like to overlay the 10 mV, 20 mV, and 30 mV at the same time.  So you'd like the times to be plotted as if they were:
             - t = 0, holding
             - t = 100 ms, +10 mV
             - t = 100 ms, +20 mV
             - t = 100 ms, +30 mV
             - t = 200 ms, holding
         *
         * To achieve that, you set tOffset appropriately:
             - t = 0, holding
             - t = 100 ms, +10 mV, tOffset = 0
             - t = 200 ms, +20 mV, tOffset = 100 ms
             - t = 300 ms, +30 mV, tOffset = 200 ms
             - t = 400 ms, holding, tOffset = 200 ms
         *
         * One minor note: the units of tOffset are timesteps, not ms, so 100 ms would be 5000 (i.e., 50,000 timestamps/second * 0.1 s)
         */
        unsigned int tOffset;

        /// The value that is applied to the chip (i.e., -255..255 for voltage clamp or -127..127 for current clamp).
        int appliedDiscreteValue;

        /// The logical value (e.g., 10 mV).  Either Volts or Amperes, depending on mode.
        double appliedValue;

        /// The timestep of the start of this segment
        unsigned int startIndex;

        /** \brief The timestep of the end of this segment
         *
         *  Note that if the segment runs from [t1...t2], this would be t2, i.e., it's the last index that is still
         *  part of this segment, not one past that.
         */
        unsigned int endIndex;

        /** \brief The number of on-chip commands corresponding to this logical command.
         *
         *  This will frequently be 1.  For long commands that require both a long time scale and a short time scale command, it will be 2.
         */
        unsigned int numCommands;

		/** \brief Marker output value.
		*
		*  This can be set to true for departures from the holding value, to produce a digital marker signal.
		*/
		bool markerOut;

		/** \brief Digital output value.
		*
		*  A second digital output value to produce other marker or control signals.
		*/
		bool digOut;

        WaveformSegment(unsigned int w, int v, unsigned int n, unsigned int t, bool m, bool d);

        unsigned int numReps() const;

        static unsigned int onDiskSize();
        friend BinaryWriter& operator<<(BinaryWriter& out, const WaveformSegment& segment);
    };
    bool operator==(const WaveformSegment& a, const WaveformSegment& b);

    /** \brief A logical description of a current or voltage waveform.
     *
     *  Consists of a number of WaveformSegments, each of which corresponds to a piecewise-constant voltage/current step.
     *
     *  For example, if the waveform is: 
        \code
           holding, +10 mV, holding
        \endcode
     *
     *  the SimplifiedWaveform would consist of 3 WaveformSegments.
     *
     *  May also include an interval; see below.
     */
    class SimplifiedWaveform {
    public:
        /// Segments constituting this waveform
        std::vector<WaveformSegment> waveform;
        /** \brief Interval between successive starts of the waveform, in seconds.
         *
         *  For example, if a 0.1 s waveform is run every 1 second (i.e., run 0.1s, wait 0.9s, run 0.1s, etc.), this value would be 1.0.
         */
        double interval;

        /// Constructor
        SimplifiedWaveform() : interval(0.0) {}

        std::vector<WaveformSegment>::size_type size() const;
        void push_back(const WaveformSegment& segment);
        void erase();
        unsigned int numWaveforms() const;
        void setStepSize(double value, double offset);
        std::vector<double> getApplied(const std::vector<uint32_t>& timestamps);
        unsigned int lastIndex(bool overlay) const;

        unsigned int onDiskSize() const;
        friend BinaryWriter& operator<<(BinaryWriter& out, const SimplifiedWaveform& simplifiedWaveform);
    };
    bool operator==(const SimplifiedWaveform& a, const SimplifiedWaveform& b);
    inline bool operator!=(const SimplifiedWaveform& a, const SimplifiedWaveform& b) {
        return !(a == b);
    }
}
