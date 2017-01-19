#include "Line.h"
#include "common.h"
#include "streams.h"

using std::vector;
using std::deque;
using std::pair;

Range::Range() :
    min(std::numeric_limits<double>::infinity()),
    max(-std::numeric_limits<double>::infinity())
{

}

void Range::applyUnion(const Range& other) {
    min = std::min(min, other.min);
    max = std::max(max, other.max);
}
//------------------------------------------------------------

LineSegment::LineSegment() :
    tRange(),
    yRange()
{

}

Range LineSegment::getTRange() const {
    return tRange;
}

Range LineSegment::getYRange() const {
    return yRange;
}

unsigned int LineSegment::getFirstIndexToDraw(double tMin) {
    auto res = std::upper_bound(t.begin(), t.end(), tMin);
    unsigned int val = res - t.begin();
    if (val > 0) {
        val--;
    }
    return val;
}

void LineSegment::append(const LineSegment& other) {
    tRange.applyUnion(other.getTRange());
    yRange.applyUnion(other.getYRange());
    t.insert(t.end(), other.t.begin(), other.t.end());
    y.insert(y.end(), other.y.begin(), other.y.end());
}

void LineSegment::append(double t, double y) {
    this->t.push_back(t);
    this->y.push_back(y);
    tRange.applyUnion(Range(t, t));
    yRange.applyUnion(Range(y, y));
}

//-------------------------------------------------------------------------

// This is a class holding data for a waveform to be plotted by the
// Plot class.

// Constructor.  Clear data and set default color.
Line::Line() :
    currentIndex(0)
{
    color = Qt::blue;
}

Line::~Line()
{
}

void Line::addLineSegment() {
    data.push_back(LineSegment());
}

// Add (time, data) point to waveform.
void Line::addPoint(double t, double y)
{
    if (data.empty()) {
        addLineSegment();
    }
    data.back().append(t, y);
}

// Return length of waveform.
int Line::length() const
{
    int value = 0;
    for (const LineSegment& lineSegment : data) {
        value += lineSegment.t.size();
    }
    return value;
}

// Stream all waveform data out to binary data stream
BinaryWriter& operator<<(BinaryWriter &outStream, const Line &a)
{
    outStream << (uint32_t)a.length();

    // First write t vector...
    for (auto& piece : a.data) {
        for (double value : piece.t) {
            outStream << value;
        }
    }

    // ...then write the y vector
    for (auto& piece : a.data) {
        for (double value : piece.y) {
            outStream << value;
        }
    }

    return outStream;
}

Range Line::getTRange() const {
    Range result;
    for (const LineSegment& lineSegment : data) {
        Range tmp = lineSegment.getTRange();
        result.applyUnion(tmp);
    }
    for (const LineSegment& lineSegment : oldData) {
        Range tmp = lineSegment.getTRange();
        result.applyUnion(tmp);
    }
    return result;
}

double Line::maxT() const {
    if (data.empty() || data.back().t.empty()) {
        return 0;
    }
    return data.back().t.back();
}

Range Line::getYRange() const {
    Range result;
    for (const LineSegment& lineSegment : data) {
        Range tmp = lineSegment.getYRange();
        result.applyUnion(tmp);
    }
    return result;
}

unsigned int Line::getFirstPieceToDraw(double tMin) {
    unsigned int result = data.size() - 1;
    while (result > 0 && !data[result].t.empty() && data[result].t[0] > tMin) {
        result--;
    }
    return result;
}

void Line::append(unsigned int startIndex, const Line& other) {
    if (!other.data.empty()) {
        auto start = other.data.begin();
        if (currentIndex == startIndex) {
            if (!data.empty()) {
                // We need to append the existing piece
                data.back().append(*start);
                start++;
            }
        }

        for (auto iter = start; iter != other.data.end(); iter++) {
            if (!iter->t.empty()) {
                data.insert(data.end(), *iter);
            }
        }
        currentIndex = startIndex;
    }
    // Get rid of any pieces that are completely invalidated
    if (!data.empty()) {
        auto& currentTs = data.back().t;
        if (!currentTs.empty()) {
            double tMax = currentTs.back();
            while (!oldData.empty() && !oldData.front().t.empty() && (oldData.front().t.back() < tMax)) {
                oldData.pop_front();
            }
        }
    }
}
