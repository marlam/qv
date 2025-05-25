/*
 * Copyright (C) 2019, 2020 Computer Graphics Group, University of Siegen
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

#include <QGuiApplication>
#include <QClipboard>
#include <QStringList>
#include <QImage>

#include "colormap.hpp"
#include "gl.hpp"


ColorMap::ColorMap() :
    _count { 4, 4, 2, 1 },
    _type(ColorMapNone),
    _index { 0, 0, 0, 0 }
{
}

void ColorMap::reload()
{
    _sRgbData.clear();

    bool haveData = false;
    QImage img;
    if (_type == ColorMapNone) {
        /* empty */
    } else if (_type == ColorMapCustom) {
        img = QGuiApplication::clipboard()->image();
        if (img.isNull()) {
            QString csvData = QGuiApplication::clipboard()->text();
            QStringList lines = csvData.split('\n');
            bool ok = true;
            for (int i = 0; i < lines.size(); i++) {
                if (lines[i].isEmpty())
                    continue;
                unsigned short r = 0, g = 0, b = 0;
                QStringList values = lines[i].split(',');
                if (values.size() != 3) {
                    ok = false;
                    break;
                }
                bool rok, gok, bok;
                r = values[0].toUShort(&rok);
                g = values[1].toUShort(&gok);
                b = values[2].toUShort(&bok);
                if (!rok || !gok || !bok || r > 0xff || g > 0xff || b > 0xff) {
                    ok = false;
                    break;
                }
                _sRgbData.push_back(r);
                _sRgbData.push_back(g);
                _sRgbData.push_back(b);
            }
            if (_sRgbData.size() == 0)
                ok = false;
            if (ok)
                haveData = true;
            else
                _sRgbData.clear();
        }
    } else {
        QString fileName = QString(":colormaps/%1-%2.png").arg(
                _type == ColorMapSequential ? "sequential"
                : _type == ColorMapDiverging ? "diverging"
                : "qualitative").arg(_index[_type]);
        img.load(fileName);
    }

    if (!haveData && !img.isNull()) {
        img = img.convertToFormat(QImage::Format_RGB32);
        if (img.width() >= img.height()) {
            _sRgbData.resize(img.width() * 3);
            const QRgb* scanline = reinterpret_cast<const QRgb*>(img.scanLine(0));
            for (int i = 0; i < img.width(); i++) {
                QRgb color = scanline[i];
                _sRgbData[3 * i + 0] = qRed(color);
                _sRgbData[3 * i + 1] = qGreen(color);
                _sRgbData[3 * i + 2] = qBlue(color);
            }
        } else {
            _sRgbData.resize(img.height() * 3);
            for (int i = 0; i < img.height(); i++) {
                const QRgb* scanline = reinterpret_cast<const QRgb*>(img.scanLine(i));
                QRgb color = scanline[0];
                _sRgbData[3 * i + 0] = qRed(color);
                _sRgbData[3 * i + 1] = qGreen(color);
                _sRgbData[3 * i + 2] = qBlue(color);
            }
        }
        haveData = true;
    }

    if (!haveData) {
        _type = ColorMapNone;
    }
}

void ColorMap::setType(ColorMapType type)
{
    _type = type;
    reload();
}

void ColorMap::cycle()
{
    int oldI = _index[_type];
    int newI = oldI + 1;
    if (newI >= _count[_type])
        newI = 0;
    _index[_type] = newI;
    reload();
}

void ColorMap::uploadTexture(unsigned int tex) const
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, _sRgbData.size() / 3, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, _sRgbData.data());
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ASSERT_GLCHECK();
}
