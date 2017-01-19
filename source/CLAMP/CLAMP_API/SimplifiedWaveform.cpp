#include "SimplifiedWaveform.h"
#include "common.h"
#include <algorithm>
#include "streams.h"

using std::vector;

namespace CLAMP {
    //------------------------------------------------------------------------------------------
    /** \brief Constructor
     *
     *  Several of the members are initialized to dummy values, and then corrected when the
     *  WaveformSegment is added to a SimplifiedWaveform.
     *
     *  \param[in] w  Waveform Number (see members)
     *  \param[in] v  Applied discrete Value (see members)
     *  \param[in] n  Number of timesteps (startIndex and endIndex)
     *  \param[in] t  Time Offset (see members)
     */
    WaveformSegment::WaveformSegment(unsigned int w, int v, unsigned int n, unsigned int t, bool m, bool d) :
        waveformNumber(w),
        tOffset(t),
        appliedDiscreteValue(v),
		markerOut(m),
		digOut(d),
        numCommands(0)
    {
        startIndex = 0;
        endIndex = n - 1;
    }

    /// Number of repetitions/timesteps that this logical command takes
    unsigned int WaveformSegment::numReps() const {
        return endIndex - startIndex + 1;
    }

    /** Size of this object on disk
     *  \returns The size in bytes
     */
    unsigned int WaveformSegment::onDiskSize() {
        return sizeof(uint8_t) + 3 * sizeof(uint32_t) + sizeof(float);
    }

    /** \brief Write the segment to a BinaryWriter stream
     *
     *  Note that only certain parts are written, corresponding to the interesting things.
     *  The other member variables are considered internal.
     *
     *  \param[in] out   Output stream
     *  \param[in] segment  Segment to write
     *  \returns Updated output stream
     */
    BinaryWriter& operator<<(BinaryWriter& out, const WaveformSegment& segment) {
        out << (uint8_t)segment.waveformNumber;
        out << (uint32_t)segment.tOffset;
        out << (uint32_t)segment.startIndex;
        out << (uint32_t)segment.endIndex;
        out << segment.appliedValue;
        return out;
    }

    bool operator==(const WaveformSegment& a, const WaveformSegment& b) {
        if (a.waveformNumber != b.waveformNumber) {
            return false;
        }
        if (a.tOffset != b.tOffset) {
            return false;
        }
        if (a.appliedDiscreteValue != b.appliedDiscreteValue) {
            return false;
        }
        if (a.appliedValue != b.appliedValue) {
            return false;
        }
        if (a.startIndex != b.startIndex) {
            return false;
        }
        if (a.endIndex != b.endIndex) {
            return false;
        }
        return true;
    }

    //------------------------------------------------------------------------------------------
    /// Size/length of the waveform
    vector<WaveformSegment>::size_type SimplifiedWaveform::size() const {
        return waveform.size();
    }

    /** \brief Add a segment to the waveform
     *
     *  Adjusts start and end indices.
     *
     *  \param[in] segment  Segment to add
     */
    void SimplifiedWaveform::push_back(const WaveformSegment& segment) {
    	WaveformSegment segment2(segment);
        if (!waveform.empty()) {
            segment2.startIndex += waveform.back().endIndex + 1;
            segment2.endIndex += waveform.back().endIndex + 1;
        }
        vector<WaveformSegment>::reverse_iterator it;
        for (it = waveform.rbegin(); it != waveform.rend(); it++) {
            if (it->waveformNumber == segment2.waveformNumber) {
                break;
            }
        }
        if (it == waveform.rend()) {
            segment2.indexWithinWaveform = 0;
        }
        else {
            segment2.indexWithinWaveform = it->indexWithinWaveform + 1;
        }
        waveform.push_back(segment2);
    }

    /// Removes all segments from the waveforms
    void SimplifiedWaveform::erase() {
        waveform.erase(waveform.begin(), waveform.end());
    }

    /// Number of different WaveformSegment::waveformNumber values
    unsigned int SimplifiedWaveform::numWaveforms() const {
        unsigned int numSteps = 1;
        for (unsigned int index = 0; index < size(); index++) {
            numSteps = std::max(waveform[index].waveformNumber + 1, numSteps);
        }
        return numSteps;
    }

    /** \brief Sets the step size, which controls the appliedDiscreteValue to appliedValue scaling.
     *
     *  For example, in voltage clamp, this is frequently 2.5e-3 (i.e., 2.5 mV / step).
     *
     *  \param[in] value   The step size
     *  \param[in] offset  The value of the pipette offset (which *is* included in appliedDiscreteValue, but should *not* be included in appliedValue)
     */
    void SimplifiedWaveform::setStepSize(double value, double offset) {
        for (unsigned int i = 0; i < size(); i++) {
            waveform[i].appliedValue = waveform[i].appliedDiscreteValue * value - offset;
        }
    }

    /** \brief Returns a vector containing the applied voltage or current
     *
     *  \param[in] timestamps  The timestamps for which to reconstruct the applied voltage or current
     *  \returns The (partial) applied waveform
     */
    vector<double> SimplifiedWaveform::getApplied(const vector<uint32_t>& timestamps) {
        vector<double> result;
        result.reserve(timestamps.size());

        if (!waveform.empty()) {
            uint32_t maxTimestamp = waveform.back().endIndex;
            auto iter = waveform.begin();
            for (uint32_t timestamp : timestamps) {
                timestamp = timestamp % (maxTimestamp + 1);
                while (iter->endIndex < timestamp) {
                    iter++;
                }
                result.push_back(iter->appliedValue);
            }
        }

        return result;
    }

    /** Size of this object on disk
     *  \returns The size in bytes
     */
    unsigned int SimplifiedWaveform::onDiskSize() const {
        return sizeof(float) + sizeof(uint16_t) + waveform.size() * WaveformSegment::onDiskSize();
    }

    /** \brief Write the simplified waveform to a BinaryWriter stream
     *
     *  \param[in] out   Output stream
     *  \param[in] simplifiedWaveform  Simplified Waveform to write
     *  \returns Updated output stream
     */
    BinaryWriter& operator<<(BinaryWriter& out, const SimplifiedWaveform& simplifiedWaveform) {
        out << simplifiedWaveform.interval;
        out << (uint16_t)simplifiedWaveform.size();
        for (const WaveformSegment& segment : simplifiedWaveform.waveform) {
            out << segment;
        }
        return out;
    }

    /** \brief Returns the last index of the simplified waveform
     *
     *  \param[in] overlay  True if waveforms are overlaid on each other; false if not
     *  \returns the index.
     */
    unsigned int SimplifiedWaveform::lastIndex(bool overlay) const {
        unsigned int result = 0;
        for (auto& segment : waveform) {
            unsigned int value = overlay ? (segment.endIndex - segment.tOffset) : segment.endIndex;
            result = std::max(result, value);
        }
        return result;
    }

    bool operator==(const SimplifiedWaveform& a, const SimplifiedWaveform& b) {
        if (a.interval != b.interval) {
            return false;
        }
        return a.waveform == b.waveform;
    }

}
