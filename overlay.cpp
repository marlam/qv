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

#include <QImage>
#include <QPainter>

#include "overlay.hpp"
#include "gl.hpp"


Overlay::Overlay() :
    image(nullptr), painter(nullptr), heightInPixels(32), texture(0)
{
    prepare(1); // to get a valid painter etc
}

Overlay::~Overlay()
{
    delete painter;
    delete image;
    auto gl = getGlFunctionsFromCurrentContext();
    if (gl)
        gl->glDeleteTextures(1, &texture);
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
        painter->setRenderHints(painter->renderHints() | QPainter::TextAntialiasing);
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
    image->fill(QColor(Qt::black));
}

void Overlay::setAlpha(int x, int y, int w, int h, int alpha)
{
    for (int l = y; l < y + h; l++) {
        unsigned char* lineData = image->scanLine(l);
        for (int c = x; c < x + w; c++) {
            lineData[4 * c + 3] = alpha;
        }
    }
}

void Overlay::uploadImageToTexture()
{
    auto gl = getGlFunctionsFromCurrentContext();
    if (texture == 0) {
        gl->glGenTextures(1, &texture);
        gl->glBindTexture(GL_TEXTURE_2D, texture);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    size_t size = image->width() * image->height() * 4;
    gl->glBindTexture(GL_TEXTURE_2D, texture);
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glGetGlobalPBO());
    gl->glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
    void* ptr = gl->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size,
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    std::memcpy(ptr, image->constBits(), size);
    gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
            image->width(), image->height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
