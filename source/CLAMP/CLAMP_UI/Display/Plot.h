#pragma once

#include <QWidget>
#include <QQueue>
#include <vector>
#include <mutex>
#include <memory>

class QToolButton;
struct Range;

struct AxisStep {
    int stepIndex;
    int minStep;
    int maxStep;

    AxisStep(int stepIndex_, int minStep_, int maxStep_);
};

class Axis : public QObject {
    Q_OBJECT

public:
    Axis(unsigned int numSteps_, int stepIndex_, const QString& unit_, const QString& name_, unsigned int startValue, int startExponent, unsigned int endValue, int endExponent, bool positiveOnly_);

    void adjustStepIndex(int delta);
    int numberOfSteps() const;
    QString getLabel() const;
    QString getTickLabel(int i) const;
    double valueToSteps(double value) const;
    void autoscale(const Range& actualRange, const Range& desiredRange, bool force);
    void scale(double minValue, double maxValue);
    void adjustZeroPosition(int delta);
    int minStepValue() const;
    int maxStepValue() const;
    void zoom(int minIndex, int maxIndex);
    bool canZoomIn() const;
    bool canZoomOut() const;
    int maxLabelWidth(QFont& font);
    double minAxisValue() const;
    double maxAxisValue() const;
    double getStep() const;
    bool autoscalable;

signals:
    void axisChanged();

public slots:
    void zoomIn();
    void zoomOut();

private:
    std::vector<AxisStep> settings;
    unsigned int zoomIndex;

    std::vector<double> stepValues;
    QString unit;
    QString name;

    bool positiveOnly;

    AxisStep& current();
    const AxisStep& current() const;
    double getAxisMagnitude() const;
    void resetScale(int stepIndexNew, int minStepNew, int maxStepNew);
    void changeScale(int stepIndexNew, int minStepNew, int maxStepNew);
    int findScaleForRange(double minValue, double maxValue, int minNumSteps, int maxNumSteps);
};

class Line;
struct LineSegment;
struct Range;

struct LineIncrements {
    std::vector<unsigned int> startIndices;
    std::vector<Line> lines;
};

class Lines : public QObject {
    Q_OBJECT
    typedef Range(Line::*GetRange_t)() const;

public slots:
    void clearLines();
    void cycleLines();

public:
    ~Lines();
    void addToLine(unsigned int lineIndex, double t, double y);
    void setLines(const std::vector<Line>& ls);
    void appendLines(unsigned int firstLineIndex, const LineIncrements& ls);
    void addLine(const Line& line);
    Range getTRange();
    Range getYRange();
    double maxT() const;

    double tStep;
    std::vector<Line> lines;
    std::recursive_mutex linesMutex;

signals:
    void needFullRedraw();
    void needPartialRedraw(double tMin, double tMax);

private:
    void colorLines();
    static QColor rainbow(double hue);
    static Range getRange(const std::vector<Line>& ls, GetRange_t getter);
};

class Plot : public QWidget
{
    Q_OBJECT

public:
    Lines& data;

    Plot(QWidget *parent, Lines& data_, std::shared_ptr<Axis>& tAxis_, std::shared_ptr<Axis>& yAxis_);
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setAutoScaling(bool value);
    void autoScaleForce();

private slots:
    void refreshPixmap();
    void scaleAndFullRefresh();
    void scaleAndPartialRefresh(double tMin, double tMax);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void fullRedraw();
    void partialRedraw(double tMin, double tMax);
    void setMargins();
    void drawAxes(QPainter& painter);
    void drawPlots(QPainter& painter, double tMin, double tMax);
    void drawPlotWaveform(QPainter& painter, double tMin, double tMax, Line& waveform);
    void drawPlotPiece(QPainter& painter, double tMin, double tMax, LineSegment& lineSegment);
    QRect getRect(double tMin, double tMax);
    QRect getCursorRect();

    QPixmap pixmap;
    std::recursive_mutex pixmapMutex;

    int xOffset;
    int topMargin;
    int xStepSize;
    int yStepSize;
    static const int leftMargin = 5;
    int rightMargin;
    static const int bottomMargin = 5;
    int yOffset;
    int maxXLabelWidth() const;
    int maxYLabelWidth() const;
    int fontHeight() const;

    std::shared_ptr<Axis> tAxis;
    std::shared_ptr<Axis> yAxis;

    bool autoScaling;
    void autoScale(bool force);
    void showHideButtons();

    bool showXSelection;
    int xClickStart;
    int xClickCurrent;
    bool showYSelection;
    int yClickStart;
    int yClickCurrent;
    bool inRect;
    std::pair<int, int> getTZoomRange();
    std::pair<int, int> getYZoomRange();

    QToolButton* upButton;
    QToolButton* downButton;
    QToolButton* leftButton;
    QToolButton* rightButton;

    int toXPixel(int value);
    int scaleToXPixel(double value);
    int toYPixel(int value);
    int scaleToYPixel(double value);
};
