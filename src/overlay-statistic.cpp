/*
 * Copyright (C) 2019, 2020, 2021, 2022
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * Copyright (C) 2023, 2024, 2025
 * Martin Lambers <marlam@marlam.de>
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


void OverlayStatistic::update(unsigned int tex, int widthInPixels, Set& set)
{
    prepare(widthInPixels, _painter->fontInfo().pixelSize() * 1.5f);

    Frame* frame = set.currentFile()->currentFrame();
    QString s = " channel=";
    if (frame->channelIndex() == ColorChannelIndex)
        s += "lightness";
    else
        s += frame->currentChannelName().c_str();
    const Statistic& S = frame->currentStatistic();
    s += QString(" min=%1 max=%2 mean=%3 var=%4 dev=%5 invalid=%6")
        .arg(S.minVal())
        .arg(S.maxVal())
        .arg(S.sampleMean())
        .arg(S.sampleVariance())
        .arg(S.sampleDeviation())
        .arg(frame->width() * frame->height() - S.finiteValues());
    float xOffset = 0.0f;
    float yOffset = 1.25f * _painter->fontInfo().pixelSize();
    _painter->drawText(xOffset, yOffset, s);

    fixFormat();
    uploadImageToTexture(tex);
}
