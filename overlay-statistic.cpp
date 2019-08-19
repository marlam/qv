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

#include <QImage>
#include <QPainter>

#include "overlay-statistic.hpp"


OverlayStatistic::OverlayStatistic()
{
    heightInPixels = painter->fontInfo().pixelSize() * 1.5f;
}

OverlayStatistic::~OverlayStatistic()
{
}

void OverlayStatistic::update(int widthInPixels, Set& set, Parameters& /* parameters */)
{
    Overlay::prepare(widthInPixels);

    Frame* frame = set.currentFile()->currentFrame();
    QString s = " channel=";
    if (frame->channelIndex() == ColorChannelIndex) {
        s += "luminance ";
    } else {
        s += QString("%1").arg(frame->channelIndex());
        if (frame->colorSpace() == ColorSpaceLinearRGB) {
            if (frame->channelIndex() == frame->colorChannelIndex(0))
                s += "(R)";
            else if (frame->channelIndex() == frame->colorChannelIndex(1))
                s += "(G)";
            else if (frame->channelIndex() == frame->colorChannelIndex(2))
                s += "(B)";
        } else if (frame->colorSpace() == ColorSpaceSRGB) {
            if (frame->channelIndex() == frame->colorChannelIndex(0))
                s += "(sR)";
            else if (frame->channelIndex() == frame->colorChannelIndex(1))
                s += "(sG)";
            else if (frame->channelIndex() == frame->colorChannelIndex(2))
                s += "(sB)";
        } else if (frame->colorSpace() == ColorSpaceXYZ) {
            if (frame->channelIndex() == frame->colorChannelIndex(0))
                s += "(X)";
            else if (frame->channelIndex() == frame->colorChannelIndex(1))
                s += "(Y)";
            else if (frame->channelIndex() == frame->colorChannelIndex(2))
                s += "(Z)";
        }
        s += ' ';
    }
    const Statistic& S = frame->currentStatistic();
    s += QString("min=%1 max=%2 mean=%3 var=%4 dev=%5").
        arg(S.minVal()).arg(S.maxVal()).arg(S.sampleMean()).arg(S.sampleVariance()).arg(S.sampleDeviation());
    float xOffset = 0.0f;
    float yOffset = 1.25f * painter->fontInfo().pixelSize();
    painter->drawText(xOffset, yOffset, s);

    setAlpha(0, 0, image->width(), image->height(), 2 * 255 / 3);
    Overlay::uploadImageToTexture();
}
