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

#include <cstring>
#include <cmath>

#include <QImage>
#include <QPainter>

#include "overlay.hpp"
#include "gl.hpp"


Overlay::Overlay() : _scaleFactor(1.0f), _image(nullptr), _painter(nullptr)
{
}

Overlay::~Overlay()
{
    delete _painter;
    delete _image;
}

void Overlay::initialize(float scaleFactor)
{
    _scaleFactor = scaleFactor;
    prepare(1, 1); // to get an initial valid _painter
}

void Overlay::prepare(int widthInPixels, int heightInPixels)
{
    if (_image && (_image->width() != widthInPixels || _image->height() != heightInPixels)) {
        delete _painter;
        _painter = nullptr;
        delete _image;
        _image = nullptr;
    }
    if (!_image) {
        _image = new QImage(widthInPixels, heightInPixels, QImage::Format_ARGB32_Premultiplied);
        _painter = new QPainter(_image);
        QFont font;
        font.setFamily("Monospace");
        font.setStyleHint(QFont::TypeWriter, QFont::PreferAntialias);
        font.setWeight(QFont::DemiBold);
        font.setPointSizeF(font.pointSizeF() * _scaleFactor);
        _painter->setFont(font);
        _painter->setRenderHint(QPainter::Antialiasing);
        _painter->setRenderHint(QPainter::TextAntialiasing);
        QPen pen;
        pen.setColor(QColor(Qt::white));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
        _painter->setPen(pen);
        QBrush brush;
        brush.setColor(QColor(Qt::white));
        brush.setStyle(Qt::SolidPattern);
        _painter->setBrush(brush);
    }
    const unsigned char bgColor = 32;
    _image->fill(QColor(bgColor, bgColor, bgColor, 255));
}

void Overlay::fixFormat(int opaqueBlockX, int opaqueBlockY, int opaqueBlockW, int opaqueBlockH)
{
    const unsigned char transparencyAlpha = 192;
    for (int l = 0; l < _image->height(); l++) {
        unsigned char* lineData = _image->scanLine(l);
        for (int c = 0; c < _image->width(); c++) {
            unsigned char v0 = lineData[4 * c + 0];
            unsigned char v1 = lineData[4 * c + 1];
            unsigned char v2 = lineData[4 * c + 2];
            unsigned char v3 = lineData[4 * c + 3];
            float preAlpha = v3 / 255.0f;
            float vv0 = v0 / preAlpha;
            float vv1 = v1 / preAlpha;
            float vv2 = v2 / preAlpha;
            unsigned char alpha = transparencyAlpha;
            if (l >= opaqueBlockY && l < opaqueBlockY + opaqueBlockH
                    && c >= opaqueBlockX && c < opaqueBlockX + opaqueBlockW) {
                alpha = 255;
            }
            lineData[4 * c + 0] = std::round(vv2);
            lineData[4 * c + 1] = std::round(vv1);
            lineData[4 * c + 2] = std::round(vv0);
            lineData[4 * c + 3] = alpha;
        }
    }
}

void Overlay::uploadImageToTexture(unsigned int tex) const
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
            _image->width(), _image->height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, _image->constBits());
    ASSERT_GLCHECK();
}
