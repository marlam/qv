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

#include "overlay-info.hpp"


OverlayInfo::OverlayInfo()
{
    heightInPixels = painter->fontInfo().pixelSize() * 1.5f;
}

OverlayInfo::~OverlayInfo()
{
}

void OverlayInfo::update(int widthInPixels, int x, int y, Set& set, Parameters& /* parameters */)
{
    Overlay::prepare(widthInPixels);

    Frame* frame = set.currentFile()->currentFrame();
    bool outside = (x < 0 || y < 0 || x >= frame->width() || y >= frame->height());

    QFontMetricsF fontMetrics(painter->font(), painter->device());
    float xOffset = 0.0f;
    float yOffset = 1.25f * painter->fontInfo().pixelSize();

    QString pos = QString(" pos=");
    if (outside) {
        pos += "outside";
    } else {
        int fieldWidth = (
                  frame->width() <= 100   && frame->height() <= 100   ? 2
                : frame->width() <= 1000  && frame->height() <= 1000  ? 3
                : frame->width() <= 10000 && frame->height() <= 10000 ? 4
                : 5);
        pos += QString("%1,%2  ").arg(x, fieldWidth).arg(y, fieldWidth);
    }
    painter->drawText(xOffset, yOffset, pos);
    xOffset += fontMetrics.horizontalAdvance(pos);

    if (!outside) {
        QString val;
        int fieldWidth = (
                  frame->type() == TAD::int8   ? 4
                : frame->type() == TAD::uint8  ? 3
                : frame->type() == TAD::int16  ? 6
                : frame->type() == TAD::uint16 ? 5
                : 11);
        for (int i = 0; i < frame->channelCount(); i++) {
            QString channelName = QString("ch%1").arg(i);
            if (frame->colorSpace() == ColorSpaceLinearRGB) {
                if (i == frame->colorChannelIndex(0))
                        channelName += "(R)";
                else if (i == frame->colorChannelIndex(1))
                    channelName += "(G)";
                else if (i == frame->colorChannelIndex(2))
                    channelName += "(B)";
            } else if (frame->colorSpace() == ColorSpaceSRGB) {
                if (i == frame->colorChannelIndex(0))
                    channelName += "(sR)";
                else if (i == frame->colorChannelIndex(1))
                    channelName += "(sG)";
                else if (i == frame->colorChannelIndex(2))
                    channelName += "(sB)";
            } else if (frame->colorSpace() == ColorSpaceXYZ) {
                if (i == frame->colorChannelIndex(0))
                    channelName += "(X)";
                else if (i == frame->colorChannelIndex(1))
                    channelName += "(Y)";
                else if (i == frame->colorChannelIndex(2))
                    channelName += "(Z)";
            }
            float v = frame->data(i).get<float>({ size_t(x), size_t(y) }, 0);
            val += channelName + QString("=%1 ").arg(v, fieldWidth, 'g');
        }
        if (frame->colorSpace() != ColorSpaceNone) {
            float v = frame->data(ColorChannelIndex).get<float>({ size_t(x), size_t(y) }, 0);
            val += QString("lum=%1").arg(v);
        }
        painter->drawText(xOffset, yOffset, val);
        xOffset += fontMetrics.horizontalAdvance(val);
    }

    setAlpha(0, 0, image->width(), image->height(), 2 * 255 / 3);
    Overlay::uploadImageToTexture();
}
