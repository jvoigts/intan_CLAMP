#pragma once

#include <vector>
#include <deque>
#include <QColor>

struct Range {
    double min;
    double max;

    Range(double min_, double max_) : min(min_), max(max_) {}
    Range();

    void applyUnion(const Range& other);
};

class BinaryWriter;
struct LineSegment {
    std::vector<double> t;
    std::vector<double> y;

    LineSegment();
    Range getTRange() const;
    Range getYRange() const;

    unsigned int getFirstIndexToDraw(double tMin);
    void append(const LineSegment& other);
    void append(double t, double y);

private:
    Range tRange;
    Range yRange;
};

class Line
{    
    friend BinaryWriter &operator<<(BinaryWriter &outStream, const Line &a);

public:
    Line();
    ~Line();

    std::deque<LineSegment> data;
    std::deque<LineSegment> oldData;
    QColor color;

    void addPoint(double t, double y);
    Range getTRange() const;
    Range getYRange() const;
    double maxT() const;
    void addLineSegment();
    unsigned int getFirstPieceToDraw(double tMin);
    void append(unsigned int startIndex, const Line& other);

private:
    int length() const;
    unsigned int currentIndex;
};

