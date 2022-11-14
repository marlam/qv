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

#include <QImage>
#include <QPainter>

#include "overlay-colormap.hpp"


void OverlayColorMap::update(int widthInPixels, Parameters& parameters)
{
    prepare(widthInPixels, 32);

    // Border
    const int borderSize = 5;
    const QColor borderColor = QColor(QColor(0, 0, 0));
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

    // Color map
    if (parameters.colorMap().type() != ColorMapNone) {
        int availableWidth = widthInPixels - 2 * borderSize;
        const std::vector<unsigned char>& sRgbData = parameters.colorMap().sRgbData();
        int entries = sRgbData.size() / 3;
        for (int i = 0; i < availableWidth; i++) {
            float normalizedI = float(i) / (availableWidth - 1);
            int sRgbIndex = std::round(normalizedI * entries);
            if (sRgbIndex >= entries)
                sRgbIndex = entries - 1;
            QColor color = QColor(
                    sRgbData[3 * sRgbIndex + 0],
                    sRgbData[3 * sRgbIndex + 1],
                    sRgbData[3 * sRgbIndex + 2]);
            _painter->fillRect(borderSize + i, borderSize, 1, heightInPixels() - 2 * borderSize, color);
        }
        fixFormat(borderSize, borderSize, _image->width() - 2 * borderSize, _image->height() - 2 * borderSize);
    } else {
        fixFormat();
    }

    uploadImageToTexture();
}
