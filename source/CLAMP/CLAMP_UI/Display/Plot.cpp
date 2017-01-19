#include <QtGui>
#include <vector>
#include <queue>
#include <iostream>

#include "Plot.h"
#include "globalconstants.h"
#include "DisplayWindow.h"
#include "Line.h"
#include "common.h"
#include "SaveFile.h"
#include "GUIUtil.h"

using std::lock_guard;
using std::recursive_mutex;
using std::vector;
using std::pair;
using std::shared_ptr;

// ----------------------------------------------------------------------------------------------------------
AxisStep::AxisStep(int stepIndex_, int minStep_, int maxStep_) :
    stepIndex(stepIndex_),
    minStep(minStep_),
    maxStep(maxStep_)
{

}

// ----------------------------------------------------------------------------------------------------------
Axis::Axis(unsigned int numSteps_, int stepIndex_, const QString& unit_, const QString& name_, unsigned int startValue, int startExponent, unsigned int endValue, int endExponent, bool positiveOnly_) :
    autoscalable(true),
    zoomIndex(0),
    unit(unit_),
    name(name_),
    positiveOnly(positiveOnly_)
{
    settings.push_back(AxisStep(stepIndex_, positiveOnly_ ? 0 : -static_cast<int>(numSteps_ / 2), positiveOnly_ ? numSteps_ : numSteps_ / 2));

    unsigned int val = startValue;
    int exp = startExponent;
    for(;;) {
        // val * 10^exp
        stepValues.push_back(val * pow(10, exp));
        if (val == endValue && exp == endExponent) {
            break;
        }
        switch (val) {
            case 1:
                val = 2;
                break;
            case 2:
                val = 5;
                break;
            case 5:
                val = 1;
                exp++;
                break;
        }
    }

}

AxisStep& Axis::current() {
    return settings[zoomIndex];
}

const AxisStep& Axis::current() const {
    return settings[zoomIndex];
}

double Axis::getStep() const {
    return stepValues[current().stepIndex];
}

double Axis::getAxisMagnitude() const {
    return std::max(std::abs(minAxisValue()), std::abs(maxAxisValue()));
}

double Axis::minAxisValue() const {
    return minStepValue() * getStep();
}

// Return maximum value of currently displayed axis.
double Axis::maxAxisValue() const {
    return maxStepValue() * getStep();
}

int Axis::minStepValue() const {
    return current().minStep;
}

int Axis::maxStepValue() const {
    return current().maxStep;
}


int Axis::numberOfSteps() const {
    return current().maxStep - current().minStep;
}

void Axis::adjustStepIndex(int delta)
{
    int stepIndexNew = current().stepIndex + delta;

    stepIndexNew = std::max(stepIndexNew, 0);
    stepIndexNew = std::min(stepIndexNew, static_cast<int>(stepValues.size()) - 1);

    changeScale(stepIndexNew, current().minStep, current().maxStep);
}

QString Axis::getLabel() const {
    double step = getAxisMagnitude();
    QString prefix;
    if (step >= 0.2e9) { prefix = "G"; }
    else if (step >= 0.2e6) { prefix = "M"; }
    else if (step >= 0.2e3) { prefix = "k"; }
    else if (step >= 0.2) { prefix = ""; }
    else if (step >= 0.2e-3) { prefix = "m"; }
    else if (step >= 0.2e-6) { prefix = QSTRING_MU_SYMBOL; }
    else if (step >= 0.2e-9) { prefix = "n"; }
    else { prefix = "p"; }

    return name + " (" + prefix + unit + ")";
}

QString Axis::getTickLabel(int i) const {
    if (i == 0) {
        return "0";
    }
    else {
        double step = getAxisMagnitude();
        double scale;
        if (step >= 0.2e9) { scale = 1e-9; }
        else if (step >= 0.2e6) { scale = 1e-6; }
        else if (step >= 0.2e3) { scale = 1e-3; }
        else if (step >= 0.2) { scale = 1; }
        else if (step >= 0.2e-3) { scale = 1.0e3; }
        else if (step >= 0.2e-6) { scale = 1.0e6; }
        else if (step >= 0.2e-9) { scale = 1.0e9; }
        else { scale = 1.0e12; }

        step = getStep();
        double mark = step * scale;

        double value = mark * i;

        int numDecimals = 0;
        if (mark < 0.9) {  // Would do 1.0, but let's be safe
            double smallest = 1.0;
            while (smallest > mark) {
                numDecimals++;
                smallest /= 10;
            }
        }

        return QString::number(value, 'f', numDecimals);
    }
}

void Axis::autoscale(const Range& actualRange, const Range& desiredRange, bool force) {
    if (!autoscalable) {
        return;
    }

    // The rule is that if there's data that doesn't fit into the window, we always autoscale.
    // If there isn't (i.e., plot is entirely contained in the window), we only autoscale if force=true

    // When checking whether there's data that doesn't fit in the window, use the actual data range...
    double minValue = positiveOnly ? 0 : actualRange.min;
    double maxValue = actualRange.max;

    bool doAutoscale = force;
    if (!doAutoscale) {
        // Always autoscale if the data won't fit on the screen.
        doAutoscale = (minValue < minAxisValue()) || (maxValue > maxAxisValue());
    }

    if (doAutoscale) {
        // ... but when we go to autoscale, use the desired range (which may contain 10% or 20% more size)
        minValue = positiveOnly ? 0 : desiredRange.min;
        maxValue = desiredRange.max;

        int stepIndexNew = findScaleForRange(minValue, maxValue, 5, 10);
        if (stepIndexNew != -1) {
            int minStepNew = floor(minValue / stepValues[stepIndexNew]);
            int maxStepNew = minStepNew + 10;
            resetScale(stepIndexNew, minStepNew, maxStepNew);
        }
    }
}

void Axis::scale(double minValue, double maxValue) {
    int stepIndexNew = findScaleForRange(minValue, maxValue, 5, 20);
    if (stepIndexNew != -1) {
        int minStepNew = floor(minValue / stepValues[stepIndexNew]);
        int maxStepNew = ceil(maxValue / stepValues[stepIndexNew]);
        resetScale(stepIndexNew, minStepNew, maxStepNew);
    }
}

void Axis::adjustZeroPosition(int delta)
{
    int minStepNew = current().minStep + delta;
    int maxStepNew = current().maxStep + delta;

    if (positiveOnly && (minStepNew < 0)) {
        minStepNew++;
        maxStepNew++;
    }

    changeScale(current().stepIndex, minStepNew, maxStepNew);
}

double Axis::valueToSteps(double value) const {
    return value / getStep() - minStepValue();
}

void Axis::zoom(int minIndex, int maxIndex) {
    double minValue = getStep() * (minIndex + minStepValue());
    double maxValue = getStep() * (maxIndex + minStepValue());

    int stepIndexNew = findScaleForRange(minValue, maxValue, 5, 20);
    if (stepIndexNew != -1) {
        int minStepNew = floor(minValue / stepValues[stepIndexNew]);
        int maxStepNew = ceil(maxValue / stepValues[stepIndexNew]);

        changeScale(stepIndexNew, minStepNew, maxStepNew);
    }
}

int Axis::findScaleForRange(double minValue, double maxValue, int minNumSteps, int maxNumSteps) {
    double diff = maxValue - minValue;
    int stepIndexNew = -1;
    for (unsigned int i = 0; i < stepValues.size(); i++) {
        double step = stepValues[i];
        int numSteps = ceil(diff / step);
        if (numSteps >= minNumSteps && numSteps <= maxNumSteps) {
            stepIndexNew = i;
            break;
        }
    }
    return stepIndexNew;
}

void Axis::resetScale(int stepIndexNew, int minStepNew, int maxStepNew) {
    bool changed = zoomIndex != 0;

    zoomIndex = 0;
    settings.erase(settings.begin() + zoomIndex + 1, settings.end());

    if (current().stepIndex != stepIndexNew || current().minStep != minStepNew || current().maxStep != maxStepNew) {
        settings[0] = AxisStep(stepIndexNew, minStepNew, maxStepNew);
        changed = true;
    }

    if (changed) {
        emit axisChanged();
    }
}

void Axis::changeScale(int stepIndexNew, int minStepNew, int maxStepNew) {
    if (current().stepIndex != stepIndexNew || current().minStep != minStepNew || current().maxStep != maxStepNew) {
        settings.erase(settings.begin() + zoomIndex + 1, settings.end());
        zoomIndex++;
        settings.push_back(AxisStep(stepIndexNew, minStepNew, maxStepNew));

        emit axisChanged();
    }
}

bool Axis::canZoomIn() const {
    return zoomIndex < settings.size() - 1;
}

bool Axis::canZoomOut() const {
    return zoomIndex > 0;
}

void Axis::zoomIn() {
    if (canZoomIn()) {
        zoomIndex++;
        emit axisChanged();
    }
}

void Axis::zoomOut() {
    if (canZoomOut()) {
        zoomIndex--;
        emit axisChanged();
    }
}

int Axis::maxLabelWidth(QFont& font) {
    QFontMetrics fm(font);

    int result = 0;
    for (int i = minStepValue(); i <= maxStepValue(); ++i) {
        result = std::max(result, fm.width(getTickLabel(i)));
    }

    return result;
}

// ----------------------------------------------------------------------------------------------------------

// Contructor.
Plot::Plot(QWidget *parent, Lines& data_, shared_ptr<Axis>& tAxis_, shared_ptr<Axis>& yAxis_) :
    QWidget(parent),
    data(data_),
    tAxis(tAxis_),
    yAxis(yAxis_),
    showXSelection(false),
    xClickStart(0),
    xClickCurrent(0),
    showYSelection(false),
    yClickStart(0),
    yClickCurrent(0),
    inRect(false)
{
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    // Connect signals
    connect(tAxis.get(), SIGNAL(axisChanged()), this, SLOT(refreshPixmap()));
    connect(yAxis.get(), SIGNAL(axisChanged()), this, SLOT(refreshPixmap()));

    // Tie signals to us
    connect(&data, SIGNAL(needPartialRedraw(double, double)), this, SLOT(scaleAndPartialRefresh(double, double)));
    connect(&data, SIGNAL(needFullRedraw()), this, SLOT(scaleAndFullRefresh()));

    upButton = new QToolButton(this);
    upButton->setIcon(QIcon(":/images/Zoom_back.png"));
    upButton->adjustSize();
    downButton = new QToolButton(this);
    downButton->setIcon(QIcon(":/images/Zoom_fwd.png"));
    downButton->adjustSize();
    leftButton = new QToolButton(this);
    leftButton->setIcon(QIcon(":/images/Zoom_back.png"));
    leftButton->adjustSize();
    rightButton = new QToolButton(this);
    rightButton->setIcon(QIcon(":/images/Zoom_fwd.png"));
    rightButton->adjustSize();

    connect(upButton, SIGNAL(clicked()), yAxis.get(), SLOT(zoomOut()));
    connect(downButton, SIGNAL(clicked()), yAxis.get(), SLOT(zoomIn()));
    connect(leftButton, SIGNAL(clicked()), tAxis.get(), SLOT(zoomOut()));
    connect(rightButton, SIGNAL(clicked()), tAxis.get(), SLOT(zoomIn()));
}

void Plot::drawAxes(QPainter& painter)
{
    // Draw vertical lines
    for (int i = tAxis->minStepValue(); i <= tAxis->maxStepValue(); ++i) {
        if (i == tAxis->minStepValue() || i == tAxis->maxStepValue()) {
            painter.setPen(Qt::black);
        } else {
            painter.setPen(Qt::lightGray);
        }
        int x = scaleToXPixel(i - tAxis->minStepValue());
        painter.drawLine(x, topMargin, x, toYPixel(-4));
    }

    // Draw horizontal lines
    for (int i = yAxis->minStepValue(); i <= yAxis->maxStepValue(); ++i) {
        if (i == yAxis->minStepValue() || i == 0 || i == yAxis->maxStepValue()) {
            painter.setPen(Qt::black);
        } else {
            painter.setPen(Qt::lightGray);
        }
        int y = scaleToYPixel(i - yAxis->minStepValue());
        painter.drawLine(toXPixel(-4), y, width() - rightMargin - 1, y);
    }

    // Redraw leftmost vertical line
    painter.setPen(Qt::black);
    painter.drawLine(toXPixel(0), topMargin, toXPixel(0), toYPixel(-4));

    // Increase font size for labels
    QFont font, originalFont;
    font = painter.font();
    originalFont = font;
    font.setPointSize(font.pointSize() + 2);
    painter.setFont(font);

    // Label time axis
    for (int i = tAxis->minStepValue(); i <= tAxis->maxStepValue(); ++i) {
        QString text = tAxis->getTickLabel(i);
        GUIUtil::drawText(painter, scaleToXPixel(i - tAxis->minStepValue()), toYPixel(-8), Qt::AlignHCenter | Qt::AlignTop, text);
    }

    // Add time axis label
    int xCenterXLabel = toXPixel((width() - xOffset - rightMargin) / 2);
    int yTopXLabel = toYPixel(-8 - fontHeight() - 1);
    GUIUtil::drawText(painter, xCenterXLabel, yTopXLabel, Qt::AlignHCenter | Qt::AlignTop, tAxis->getLabel());

    // And place zoom buttons
    int yTopXButton = yTopXLabel - (leftButton->height() - fontHeight()) / 2;
    int xLabelWidth = fontMetrics().width(tAxis->getLabel());
    leftButton->move(xCenterXLabel - xLabelWidth / 2 - leftButton->width() - 10, yTopXButton);
    rightButton->move(xCenterXLabel + xLabelWidth / 2 + 10, yTopXButton);

    // Label y axis
    for (int i = yAxis->minStepValue(); i <= yAxis->maxStepValue(); ++i) {
        QString text = yAxis->getTickLabel(i);
        GUIUtil::drawText(painter, toXPixel(-8), scaleToYPixel(i - yAxis->minStepValue()), Qt::AlignRight | Qt::AlignVCenter, text);
    }

    int yCenterYLabel = toYPixel((height() - topMargin - yOffset) / 2);
    painter.save();
    painter.rotate(-90);
    // Flip x and y here because we're flipped 90 degrees
    GUIUtil::drawText(painter, -yCenterYLabel, leftMargin, Qt::AlignHCenter | Qt::AlignTop, yAxis->getLabel());
    painter.restore();

    int yLabelWidth = fontMetrics().width(yAxis->getLabel());
    int xLeftYButton = leftMargin - (leftButton->height() - fontHeight()) / 2;;
    upButton->move(xLeftYButton, yCenterYLabel - yLabelWidth / 2 - upButton->height() - 20);
    downButton->move(xLeftYButton, yCenterYLabel + yLabelWidth / 2 + 20);

    // Restore original font size
    painter.setFont(originalFont);
}

template <typename T>
unsigned int lastIndex(const vector<T>& data) {
    if (data.empty()) {
        return 0;
    }
    return data.size() - 1;
}

void Plot::drawPlotPiece(QPainter& painter, double tMin, double tMax, LineSegment& lineSegment) {
    QVector<QPointF> p;
    p.reserve(lineSegment.t.size());

    const vector<double>& y = lineSegment.y;
    const vector<double>& t = lineSegment.t;
    auto iter = std::upper_bound(t.begin(), t.end(), tMax);
    int maxIteration = std::min(static_cast<std::vector<double>::size_type>(iter - t.begin()), t.size() - 1);
    for (int i = lineSegment.getFirstIndexToDraw(tMin); i <= maxIteration; ++i) {
        double normalizedT = tAxis->valueToSteps(t[i]);
        double normalizedY = yAxis->valueToSteps(y[i]);
        p.push_back(QPointF(scaleToXPixel(normalizedT), scaleToYPixel(normalizedY)));
    }

    if (!p.empty()) {
        painter.drawPoint(p.first());
        painter.drawPolyline(p);
    }
}

void Plot::drawPlotWaveform(QPainter& painter, double tMin, double tMax, Line& waveform) {
    painter.setPen(waveform.color);

    double tMax_newData = tMin;
    auto piece = waveform.data.begin() + waveform.getFirstPieceToDraw(tMin);
    for (; piece != waveform.data.end() && (piece->t[0] <= tMax); piece++) {
        drawPlotPiece(painter, tMin, tMax, *piece);
        if (!piece->t.empty()) {
            tMax_newData = std::max(tMax_newData, piece->t.back());
        }
    }

    // Draw any residual data
    piece = waveform.oldData.begin();
    for (; piece != waveform.oldData.end() && (piece->t[0] <= tMax); piece++) {
        drawPlotPiece(painter, tMax_newData, tMax, *piece);
    }
}

void Plot::drawPlots(QPainter& painter, double tMin, double tMax)
{
    for (Line& waveform : data.lines) {
        if (!waveform.data.empty()) {
            drawPlotWaveform(painter, tMin, tMax, waveform);
        }
    }
}

int Plot::maxXLabelWidth() const {
    QFont font = this->font();
    font.setPointSize(font.pointSize() + 2);

    return tAxis->maxLabelWidth(font);
}

int Plot::maxYLabelWidth() const {
    QFont font = this->font();
    font.setPointSize(font.pointSize() + 2);
    QFontMetrics fm(font);

    return yAxis->maxLabelWidth(font);
}

int Plot::fontHeight() const {
    QFont font = this->font();
    font.setPointSize(font.pointSize() + 2);
    QFontMetrics fm(font);
    return fm.height();
}

QSize Plot::minimumSizeHint() const
{
    int h = fontHeight();
    int w = maxXLabelWidth();

    return QSize(xOffset + tAxis->numberOfSteps() * (w + 1) + rightMargin, yOffset + yAxis->numberOfSteps() * (h + 1) + topMargin);
}

QSize Plot::sizeHint() const
{
    return QSize(860, 690);
}

void Plot::setMargins() {
    // Initialize graphics size parameters   
    rightMargin = std::max(20, maxXLabelWidth() / 2 + 5);
    xOffset = leftMargin + fontHeight() + 1 + maxYLabelWidth() + upButton->width() / 2 + 2;
    xStepSize = (width() - (xOffset + rightMargin)) / tAxis->numberOfSteps();

    topMargin = std::max(20, fontHeight() / 2 + 5);
    yOffset = 8 + 2 * fontHeight() + 1 + bottomMargin;
    yStepSize = (height() - (topMargin + yOffset)) / yAxis->numberOfSteps();
}

void Plot::resizeEvent(QResizeEvent*) {
    lock_guard<recursive_mutex> lockw(data.linesMutex);
    lock_guard<recursive_mutex> lockp(pixmapMutex);

    // Pixel map used for double buffering.
    if (pixmap.size() != size()) {
        pixmap = QPixmap(size());
        pixmap.fill(this, 0, 0);
    }

    fullRedraw();
}

void Plot::paintEvent(QPaintEvent *)
{
    lock_guard<recursive_mutex> lockp(pixmapMutex);

    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);

    if (inRect) {
        if (showXSelection) {
            pair<int, int> range = getTZoomRange();
            int minIndex = range.first;
            int maxIndex = range.second;
            for (int i = minIndex; i < maxIndex; i++) {
                QRect r(scaleToXPixel(i) + 2, toYPixel(-2), xStepSize - 3, 5);
                stylePainter.fillRect(r, Qt::blue);
            }
            stylePainter.fillRect(scaleToXPixel(minIndex), topMargin, (maxIndex - minIndex) * xStepSize, height() - (topMargin + yOffset), QColor(0, 0, 255, 100));
        }

        if (showYSelection) {
            pair<int, int> range = getYZoomRange();
            int minIndex = range.first;
            int maxIndex = range.second;
            for (int i = minIndex; i < maxIndex; i++) {
                QRect r(toXPixel(-7), scaleToYPixel(i + 1) + 2, 5, yStepSize - 3);
                stylePainter.fillRect(r, Qt::blue);
            }
            stylePainter.fillRect(toXPixel(0), scaleToYPixel(maxIndex), width() - (xOffset + rightMargin), (maxIndex - minIndex) * yStepSize, QColor(0, 0, 255, 50));
        }
    }
}

// Refresh pixel map used in double buffered graphics.
void Plot::refreshPixmap() {
    if (isVisible()) {
        fullRedraw();
    }
}

void Plot::fullRedraw()
{
    lock_guard<recursive_mutex> lockw(data.linesMutex);
    lock_guard<recursive_mutex> lockp(pixmapMutex);

    showHideButtons();
    setMargins();

    if (pixmap.isNull()) {
        return;
    }

    QPainter painter(&pixmap);
    painter.initFrom(this);

    // Clear old display.
    painter.eraseRect(rect());

    // Draw box around entire display.
    QRect rect(this->rect());

    rect.adjust(0, 0, -1, -1);
    painter.setPen(Qt::darkGray);
    painter.drawRect(rect);

    rect.adjust(1, 1, -1, -1);
    painter.fillRect(rect, Qt::white);

    drawAxes(painter);
    
    QRect clipRect;
    clipRect.setCoords(toXPixel(1), topMargin + 1, width() - rightMargin - 1, toYPixel(1));
    painter.setClipRect(clipRect);
    drawPlots(painter, tAxis->minAxisValue(), tAxis->maxAxisValue());

    update();
}

QRect Plot::getRect(double tMin, double tMax) {
    int minX = scaleToXPixel(tAxis->valueToSteps(tMin));
    int maxX = scaleToXPixel(tAxis->valueToSteps(tMax));
    maxX = std::min(maxX, width() - rightMargin);
    QRect clipRect;
    clipRect.setCoords(minX, topMargin + 1, maxX, toYPixel(1));
    return clipRect;
}

const int CURSOR_WIDTH = 10;

// Clear an area of CURSOR_WIDTH pixels past the end of the data, so that cycling looks good
QRect Plot::getCursorRect() {
    int minX = scaleToXPixel(tAxis->valueToSteps(data.maxT()));
    int maxX = std::min(minX + CURSOR_WIDTH, width() - rightMargin);
    QRect clearRect;
    clearRect.setCoords(minX, topMargin + 1, maxX, toYPixel(1));

    Range tRange = data.getTRange();
    QRect clipRect = getRect(tRange.min, tRange.max);
    return clearRect.intersect(clipRect);
}

void Plot::partialRedraw(double tMin, double tMax)
{
    lock_guard<recursive_mutex> lockw(data.linesMutex);
    lock_guard<recursive_mutex> lockp(pixmapMutex);

    if (pixmap.isNull()) {
        return;
    }

    QPainter painter(&pixmap);
    painter.initFrom(this);

    // Clear the cursor area
    QRect clearRect = getCursorRect();
    painter.eraseRect(clearRect);
    painter.fillRect(clearRect, Qt::white);
    painter.setClipRect(clearRect);
    drawAxes(painter);

    // Redraw a little extra
    tMin -= tAxis->getStep() / xStepSize;
    tMin = std::max(tMin, tAxis->minAxisValue());
    tMax = std::min(tMax, tAxis->maxAxisValue());

    QRect clipRect = getRect(tMin, tMax);
    painter.setClipRect(clipRect);
    painter.eraseRect(clipRect);
    painter.fillRect(clipRect, Qt::white);

    drawAxes(painter);
    drawPlots(painter, tMin, tMax);

    update();
}

void Plot::autoScaleForce() {
    autoScale(true);
}

void Plot::scaleAndFullRefresh() {
    autoScale(false);
    fullRedraw();
}

void Plot::scaleAndPartialRefresh(double tMin, double tMax) {
    autoScale(false);
    partialRedraw(tMin, tMax);
}

// Parse keypress commands.
void Plot::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_PageUp:
        yAxis->adjustZeroPosition(+1);
        break;
    case Qt::Key_PageDown:
        yAxis->adjustZeroPosition(-1);
        break;
    case Qt::Key_Left:
	case Qt::Key_Less:
	case Qt::Key_Comma:
        tAxis->adjustStepIndex(1);
        break;
    case Qt::Key_Right:
	case Qt::Key_Greater:
	case Qt::Key_Period:
        tAxis->adjustStepIndex(-1);
        break;
    case Qt::Key_Up:
	case Qt::Key_Plus:
	case Qt::Key_Equal:
        yAxis->adjustStepIndex(-1);
        break;
    case Qt::Key_Down:
	case Qt::Key_Minus:
	case Qt::Key_Underscore:
        yAxis->adjustStepIndex(+1);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void Plot::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

Lines::~Lines() {

}

void Lines::colorLines() {
    lock_guard<recursive_mutex> lockw(linesMutex);

    for (unsigned int i = 0; i < lines.size(); i++) {
        lines[i].color = rainbow(1.0 * i / (lines.size() - 1));
    }
}

void Lines::addLine(const Line& line)
{
    lock_guard<recursive_mutex> lockw(linesMutex);

    lines.push_back(line);
    colorLines();
    emit needFullRedraw();
}

void Lines::setLines(const std::vector<Line>& ls) {
    lock_guard<recursive_mutex> lockw(linesMutex);

    lines.clear();
    for (const Line& line : ls) {
        lines.push_back(line);
    }
    colorLines();

    emit needFullRedraw();
}

void Lines::appendLines(unsigned int firstLineIndex, const LineIncrements& ls) {
    lock_guard<recursive_mutex> lockw(linesMutex);

    if (lines.size() < firstLineIndex + ls.lines.size()) {
        lines.resize(firstLineIndex + ls.lines.size());
    }
    Range range = getRange(ls.lines, &Line::getTRange);

    for (unsigned int i = 0; i < ls.lines.size(); i++) {
        const Line& input = ls.lines[i];
        Line& line = lines[i + firstLineIndex];
        line.append(ls.startIndices[i], input);
    }

    colorLines();

    emit needPartialRedraw(range.min, range.max);
}

void Lines::addToLine(unsigned int lineIndex, double t, double y) {
    lock_guard<recursive_mutex> lockw(linesMutex);

    if (lines.size() <= lineIndex) {
        lines.resize(lineIndex + 1);
    }
    colorLines();

    Line& w = lines[lineIndex];
    w.addPoint(t, y);

    double tMin = t;
    auto& piece = w.data.back();
    if (piece.t.size() > 1) {
        tMin = piece.t[piece.t.size() - 2];
    }
    emit needPartialRedraw(tMin, t);
}

void Lines::cycleLines()
{
    lock_guard<recursive_mutex> lockw(linesMutex);

    for (auto& line : lines) {
        line.oldData.erase(line.oldData.begin(), line.oldData.end());
        line.oldData.swap(line.data);
    }
    emit needPartialRedraw(0, 0);
}

void Lines::clearLines()
{
    lock_guard<recursive_mutex> lockw(linesMutex);

    lines.clear();
    emit needFullRedraw();
}

Range Lines::getRange(const vector<Line>& ls, GetRange_t getter) {
    Range result;
    for (const Line& line : ls) {
        Range tmp = CALL_MEMBER_FN(line, getter)();
        result.applyUnion(tmp);
    }
    return result;
}

Range Lines::getTRange() {
    lock_guard<recursive_mutex> lockw(linesMutex);
    return getRange(lines, &Line::getTRange);
}

Range Lines::getYRange() {
    lock_guard<recursive_mutex> lockw(linesMutex);

    return getRange(lines, &Line::getYRange);
}

double Lines::maxT() const {
    double result = 0;
    for (const Line& line : lines) {
        result = std::max(result, line.maxT());
    }
    return result;
}

// Returns a QColor from a rainbox color spectrum, where hue is between 0 and 1.
QColor Lines::rainbow(double hue)
{
    QColor c;

    hue = std::max(0.0, hue);
    hue = std::min(1.0, hue);

    c.setHsv(round(hue * 255), 255, 200);
    return c;
}

void Plot::setAutoScaling(bool value) {
    autoScaling = value;

    autoScale(true);
}

void Plot::showHideButtons() {
    upButton->setEnabled(yAxis->canZoomOut());
    downButton->setEnabled(yAxis->canZoomIn());
    bool zoomY = upButton->isEnabled() || downButton->isEnabled();
    upButton->setVisible(zoomY);
    downButton->setVisible(zoomY);

    leftButton->setEnabled(tAxis->canZoomOut());
    rightButton->setEnabled(tAxis->canZoomIn());
    bool zoomX = leftButton->isEnabled() || rightButton->isEnabled();
    leftButton->setVisible(zoomX);
    rightButton->setVisible(zoomX);
}

// Deal with scaling
void Plot::autoScale(bool force) {
    lock_guard<recursive_mutex> lockw(data.linesMutex);
    lock_guard<recursive_mutex> lockp(pixmapMutex);

    if (autoScaling) {
        tAxis->autoscale(data.getTRange(), data.getTRange(), force);

        const Range& yRange = data.getYRange();
        double length = yRange.max - yRange.min;
        if (length <= std::abs(yRange.max) * 0.001) {
            length = std::abs(yRange.max) * 0.1;
        }
        if (length <= 0) {
            length = yAxis->getStep();
        }
        yAxis->autoscale(yRange, Range(yRange.min - 0.1*length, yRange.max + 0.1*length), force);
    }
}

void Plot::mousePressEvent(QMouseEvent* event) {
    if (!autoScaling && event->button() == Qt::LeftButton) {
        QPoint p = event->pos();

        QRect xRect(toXPixel(0), toYPixel(0), width() - xOffset - rightMargin, yOffset);
        QRect yRect(0, topMargin, xOffset, height() - yOffset - topMargin);
        if (xRect.contains(p)) {
            inRect = true;
            showXSelection = true;
            xClickStart = (p.x() - xOffset) / xStepSize;
            xClickCurrent = xClickStart;
            update();
        }
        else if (yRect.contains(p)) {
            inRect = true;
            showYSelection = true;
            yClickStart = (height() - p.y() - yOffset) / yStepSize;
            yClickCurrent = yClickStart;
            update();
        }
    }
}

void Plot::mouseMoveEvent(QMouseEvent* event) {
    QPoint p = event->pos();
    inRect = rect().contains(p);
    if (inRect) {
        if (showXSelection) {
            xClickCurrent = (p.x() - xOffset) / xStepSize;
        }
        else if (showYSelection) {
            yClickCurrent = (height() - p.y() - yOffset) / yStepSize;
        }
    }
    if (showXSelection || showYSelection) {
        update();
    }
}

void Plot::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (event->button() == Qt::LeftButton) {
            QPoint p = event->pos();
            inRect = rect().contains(p);
            if (showXSelection && inRect) {
                pair<int, int> range = getTZoomRange();
                tAxis->zoom(range.first, range.second);
            }
            else if (showYSelection && inRect) {
                pair<int, int> range = getYZoomRange();
                yAxis->zoom(range.first, range.second);
            }
            showXSelection = false;
            xClickStart = 0;
            xClickCurrent = 0;
            showYSelection = false;
            yClickStart = 0;
            yClickCurrent = 0;
            update();
        }
    }
}

pair<int, int> Plot::getTZoomRange() {
    int minIndex = std::min(xClickStart, xClickCurrent);
    int maxIndex = std::max(xClickStart, xClickCurrent) + 1;

    minIndex = std::max(minIndex, 0);
    maxIndex = std::min(maxIndex, tAxis->numberOfSteps());

    return pair<int, int>(minIndex, maxIndex);
}

pair<int, int> Plot::getYZoomRange() {
    int minIndex = std::min(yClickStart, yClickCurrent);
    int maxIndex = std::max(yClickStart, yClickCurrent) + 1;
    
    minIndex = std::max(minIndex, 0);
    maxIndex = std::min(maxIndex, yAxis->numberOfSteps());

    return pair<int, int>(minIndex, maxIndex);
}

int Plot::toXPixel(int value) {
    return xOffset + value;
}

int Plot::scaleToXPixel(double value) {
    return toXPixel(value * xStepSize);
}

int Plot::toYPixel(int value) {
    return height() - (yOffset + value);
}

int Plot::scaleToYPixel(double value) {
    return toYPixel(value * yStepSize);
}
