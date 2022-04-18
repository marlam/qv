/*
 * Copyright (C) 2019, 2020, 2021, 2022
 * Computer Graphics Group, University of Siegen
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

#include "version.hpp"
#include "overlay-fallback.hpp"


OverlayFallback::OverlayFallback()
{
    _fallbackText << "         qv " QV_VERSION;
    _fallbackText << "";
    _fallbackText << "  https://marlam.de/qv";
}

void OverlayFallback::update(int widthInPixels)
{
    QSize s = size();

    prepare(widthInPixels, s.height());

    float xOffset = (widthInPixels - s.width()) / 2.0f;
    for (int line = 0; line < _fallbackText.size(); line++) {
        float yOffset = (line + 2.25f) * _painter->fontInfo().pixelSize();
        _painter->drawText(xOffset, yOffset, _fallbackText[line]);
    }

    fixFormat();
    uploadImageToTexture();
}

QSize OverlayFallback::size() const
{
    int heightInPixels = _painter->fontInfo().pixelSize() * (_fallbackText.size() + 2.5f);
    int longestLineLength = -1;
    for (int i = 0; i < _fallbackText.size(); i++) {
        if (_fallbackText[i].length() > longestLineLength)
            longestLineLength = _fallbackText[i].length();
    }
    QFontMetricsF fontMetrics(_painter->font(), _painter->device());
    int widthInPixels = fontMetrics.horizontalAdvance(' ') * (longestLineLength + 1 + 0.5f);
    return QSize(widthInPixels, heightInPixels);
}
