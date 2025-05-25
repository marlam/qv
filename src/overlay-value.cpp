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

#include "overlay-value.hpp"


void OverlayValue::update(unsigned int tex, int widthInPixels, const QPoint& arrayCoordinates, Set& set)
{
    prepare(widthInPixels, _painter->fontInfo().pixelSize() * 1.5f);

    Frame* frame = set.currentFile()->currentFrame();
    bool outside = (arrayCoordinates.x() < 0 || arrayCoordinates.y() < 0
            || arrayCoordinates.x() >= frame->width() || arrayCoordinates.y() >= frame->height());

    QFontMetricsF fontMetrics(_painter->font(), _painter->device());
    float xOffset = 0.0f;
    float yOffset = 1.25f * _painter->fontInfo().pixelSize();

    QString pos = QString(" pos=");
    if (outside) {
        pos += "outside";
    } else {
        int fieldWidth = (
                  frame->width() <= 100   && frame->height() <= 100   ? 2
                : frame->width() <= 1000  && frame->height() <= 1000  ? 3
                : frame->width() <= 10000 && frame->height() <= 10000 ? 4
                : 5);
        pos += QString("%1,%2  ").arg(arrayCoordinates.x(), fieldWidth).arg(arrayCoordinates.y(), fieldWidth);
    }
    _painter->drawText(xOffset, yOffset, pos);
    xOffset += fontMetrics.horizontalAdvance(pos);

    if (!outside) {
        QString val;
        for (int i = 0; i < frame->channelCount(); i++) {
            val += QString("ch%1=").arg(frame->channelName(i).c_str());
            float v = frame->value(arrayCoordinates.x(), arrayCoordinates.y(), i);
            QString vv;
            switch (frame->type()) {
            case TGD::int8:
                vv = QString::number(v, 'f', 6);
                val += vv.rightJustified(4);
                break;
            case TGD::uint8:
                vv = QString::number(v, 'f', 6);
                val += vv.rightJustified(3);
                break;
            case TGD::int16:
                vv = QString::number(v, 'f', 6);
                val += vv.rightJustified(6);
                break;
            case TGD::uint16:
                vv = QString::number(v, 'f', 6);
                val += vv.rightJustified(5);
                break;
            default:
                vv = QString::number(v, 'g', 7);
                val += vv.rightJustified(13); // 7 significant digits + sign + dot + 'e' + sign of exponent + 2 digits for exponent
                break;
            }
            val += ' ';
        }
        if (frame->colorSpace() != ColorSpaceNone) {
            float v = frame->value(arrayCoordinates.x(), arrayCoordinates.y(), ColorChannelIndex);
            val += QString("lightness=%1").arg(v);
        }
        _painter->drawText(xOffset, yOffset, val);
        xOffset += fontMetrics.horizontalAdvance(val);
    }

    fixFormat();
    uploadImageToTexture(tex);
}
