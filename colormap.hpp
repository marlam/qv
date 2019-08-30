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

#ifndef QV_COLORMAP_HPP
#define QV_COLORMAP_HPP

#include <vector>

typedef enum {
    ColorMapSequential = 0,
    ColorMapDiverging = 1,
    ColorMapQualitative = 2,
    ColorMapCustom = 3,
    ColorMapNone = 4
} ColorMapType;

class ColorMap {
private:
    int _count[5];
    ColorMapType _type;
    int _index[5];
    std::vector<unsigned char> _sRgbData;
    unsigned int _texture;

    void reload();

public:
    ColorMap();
    ~ColorMap();

    ColorMapType type() const { return _type; }
    void setType(ColorMapType type);
    void cycle();

    const std::vector<unsigned char> sRgbData() const { return _sRgbData; }
    unsigned int texture();
    void clearTexture();
};

#endif
