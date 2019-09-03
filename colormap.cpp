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

#include <QGuiApplication>
#include <QClipboard>
#include <QStringList>

#include "colormap.hpp"
#include "utils.hpp"
#include "gl.hpp"


ColorMap::ColorMap() :
    _count { 4, 4, 1, 1 },
    _type(ColorMapNone),
    _index { 0, 0, 0, 0 },
    _texture(0)
{
}

ColorMap::~ColorMap()
{
    clearTexture();
}

void ColorMap::reload()
{
    clearTexture();
    _sRgbData.clear();

    QString csvData;
    if (_type == ColorMapNone) {
        /* empty */
    } else if (_type == ColorMapCustom) {
        csvData = QGuiApplication::clipboard()->text();
    } else {
        QString fileName = QString(":colormap-%1-%2.csv").arg(
                _type == ColorMapSequential ? "sequential"
                : _type == ColorMapDiverging ? "diverging"
                : "qualitative").arg(_index[_type]);
        csvData = readFile(fileName);
    }    

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
    if (!ok)
        _type = ColorMapNone;
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

unsigned int ColorMap::texture()
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    if (_texture == 0 && type() != ColorMapNone) {
        gl->glGenTextures(1, &_texture);
        gl->glBindTexture(GL_TEXTURE_2D, _texture);
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glGetGlobalPBO());
        gl->glBufferData(GL_PIXEL_UNPACK_BUFFER, _sRgbData.size(), nullptr, GL_STREAM_DRAW);
        void* ptr = gl->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, _sRgbData.size(),
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        std::memcpy(ptr, _sRgbData.data(), _sRgbData.size());
        gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, _sRgbData.size() / 3, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (isOpenGLES()) {
            // mipmap generation does not seem to work reliably!?
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            gl->glGenerateMipmap(GL_TEXTURE_2D);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ASSERT_GLCHECK();
    }
    return _texture;
}

void ColorMap::clearTexture()
{
    auto gl = getGlFunctionsFromCurrentContext();
    if (gl) // at program termination, the GL context might already be gone
        gl->glDeleteTextures(1, &_texture);
    _texture = 0;
}
