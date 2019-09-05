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

#include <cstring>
#include <cmath>

#include <QImage>
#include <QPainter>

#include "overlay.hpp"
#include "gl.hpp"


Overlay::Overlay() :
    image(nullptr), painter(nullptr), heightInPixels(32)
{
    prepare(1); // to get a valid painter etc
}

Overlay::~Overlay()
{
    delete painter;
    delete image;
}

void Overlay::prepare(int widthInPixels)
{
    if (image && (image->width() != widthInPixels || image->height() != heightInPixels)) {
        delete painter;
        painter = nullptr;
        delete image;
        image = nullptr;
    }
    if (!image) {
        image = new QImage(widthInPixels, heightInPixels, QImage::Format_ARGB32_Premultiplied);
        painter = new QPainter(image);
        QFont font;
        font.setFamily("Monospace");
        font.setStyleHint(QFont::Monospace, QFont::PreferAntialias);
        font.setWeight(QFont::DemiBold);
        painter->setFont(font);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        QPen pen;
        pen.setColor(QColor(Qt::white));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
        painter->setPen(pen);
        QBrush brush;
        brush.setColor(QColor(Qt::white));
        brush.setStyle(Qt::SolidPattern);
        painter->setBrush(brush);
    }
    const unsigned char bgColor = 3 * 255 / 6;
    image->fill(QColor(bgColor, bgColor, bgColor, 255));
}

void Overlay::fixFormat(int opaqueBlockX, int opaqueBlockY, int opaqueBlockW, int opaqueBlockH)
{
    const unsigned char transparencyAlpha = 5 * 255 / 6;
    for (int l = 0; l < image->height(); l++) {
        unsigned char* lineData = image->scanLine(l);
        for (int c = 0; c < image->width(); c++) {
            float preAlpha = lineData[4 * c + 3] / 255.0f;
            float v0 = lineData[4 * c + 0] / preAlpha;
            float v1 = lineData[4 * c + 1] / preAlpha;
            float v2 = lineData[4 * c + 2] / preAlpha;
            lineData[4 * c + 0] = std::round(v2);
            lineData[4 * c + 1] = std::round(v1);
            lineData[4 * c + 2] = std::round(v0);
            unsigned char alpha = transparencyAlpha;
            if (l >= opaqueBlockY && l < opaqueBlockY + opaqueBlockH
                    && c >= opaqueBlockX && c < opaqueBlockX + opaqueBlockW) {
                alpha = 255;
            }
            lineData[4 * c + 3] = alpha;
        }
    }
}

void Overlay::uploadImageToTexture()
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    if (!_textureHolder.get())
        _textureHolder = std::make_shared<TextureHolder>();
    if (_textureHolder->size() != 1) {
        _textureHolder->create(1);
        gl->glBindTexture(GL_TEXTURE_2D, texture());
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    size_t size = image->width() * image->height() * 4;
    gl->glBindTexture(GL_TEXTURE_2D, texture());
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glGetGlobalPBO());
    gl->glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
    void* ptr = gl->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    std::memcpy(ptr, image->constBits(), size);
    gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
            image->width(), image->height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    ASSERT_GLCHECK();
}
