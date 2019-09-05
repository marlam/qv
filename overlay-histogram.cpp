/*
 * Copyright (C) 2019 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>
#include <limits>

#include <QImage>
#include <QPainter>

#include "overlay-histogram.hpp"


static float clamp(float v, float lo, float hi)
{
    if (v < lo)
        v = lo;
    else if (v > hi)
        v = hi;
    return v;
}

static float logtransf(float x)
{
    const float base = 100.0f;
    return clamp(std::log(1.0f + x * (base - 1.0f)) / std::log(base), 0.0f, 1.0f);
}

void OverlayHistogram::update(int widthInPixels, int x, int y, Set& set, Parameters& parameters)
{
    prepare(widthInPixels, 64);

    Frame* frame = set.currentFile()->currentFrame();
    const Histogram& H = frame->currentHistogram();

    // Border
    const int borderSize = 5;
    const QColor borderColor = QColor(QColor(64, 64, 64));
    int borderX0 = borderSize - 1;
    int borderY0 = borderSize - 1;
    int borderX1 = widthInPixels - 1 - borderX0;
    int borderY1 = heightInPixels() - 1 - borderY0;
    int borderWidth = borderX1 - borderX0;
    int borderHeight = borderY1 - borderY0;
    _painter->fillRect(borderX0, borderY0, borderWidth, 1, borderColor);
    _painter->fillRect(borderX0, borderY1, borderWidth, 1, borderColor);
    _painter->fillRect(borderX0, borderY0, 1, borderHeight, borderColor);
    _painter->fillRect(borderX1, borderY0, 1, borderHeight + 1 /* Huh?? */, borderColor);

    // Vis Interval
    float visMin = parameters.visMinVal(frame->channelIndex());
    float visMax = parameters.visMaxVal(frame->channelIndex());
    float normalizedVisMin = (visMin - H.minVal()) / (H.maxVal() - H.minVal());
    float normalizedVisMax = (visMax - H.minVal()) / (H.maxVal() - H.minVal());
    int visX0 = borderSize + normalizedVisMin * (widthInPixels - 2 * borderSize);
    int visX1 = borderSize + normalizedVisMax * (widthInPixels - 2 * borderSize);
    int visY0 = borderSize;
    int visY1 = heightInPixels() - 1 - borderSize;
    QColor visIntervalColor = QColor(Qt::gray);
    _painter->fillRect(visX0, visY0, visX1 - visX0, visY1 - visY0 + 1, visIntervalColor);

    // Histogram
    bool outside = (x < 0 || y < 0 || x >= frame->width() || y >= frame->height());
    float value = (outside ? 0.0f : frame->value(x, y, frame->channelIndex()));
    int availableWidth = widthInPixels - 2 * borderSize;
    float binWidth = float(availableWidth) / H.binCount();
    int availableHeight = heightInPixels() - 2 * borderSize;
    int binY = heightInPixels() - borderSize;
    for (int bin = 0; bin < H.binCount(); bin++) {
        int binX = borderSize + std::round(bin * binWidth);
        if (binX >= borderX1)
            binX = borderX1 - 1;
        int nextBinX = borderSize + std::round((bin + 1) * binWidth);
        int thisBinWidth = nextBinX - binX;
        if (thisBinWidth < 1 || binX == borderX1 - 1)
            thisBinWidth = 1;
        float normalizedBinHeight = float(H.binVal(bin)) / H.maxBinVal();
        if (frame->type() != TAD::int8 && frame->type() != TAD::uint8) {
            normalizedBinHeight = logtransf(normalizedBinHeight);
        }
        int binHeight = std::round(normalizedBinHeight * availableHeight);
        if (!outside && H.binIndex(value) == bin) {
            _painter->fillRect(binX, borderSize, thisBinWidth, availableHeight, QColor(Qt::green));
            _painter->fillRect(binX, binY, thisBinWidth, -binHeight, QColor(Qt::green));
        } else {
            _painter->fillRect(binX, binY, thisBinWidth, -binHeight, QColor(Qt::white));
        }
    }

    fixFormat();
    uploadImageToTexture();
}
