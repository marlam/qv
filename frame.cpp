/*
 * Copyright (C) 2019, 2020, 2021  Computer Graphics Group, University of Siegen
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

#include <limits>
#include <type_traits>
#include <cmath>

#include "frame.hpp"
#include "gl.hpp"


Frame::Frame() :
    _colorSpace(ColorSpaceNone), _colorChannels { -1, -1, -1 }, _alphaChannel(-1),
    _channelIndex(-1)
{
}

static int componentIndex(const TAD::ArrayContainer& a, const std::string& interpretationValue)
{
    int ret = -1;
    for (size_t i = 0; i < a.componentCount(); i++) {
        if (a.componentTagList(i).value("INTERPRETATION") == interpretationValue) {
            ret = i;
            break;
        }
    }
    return ret;
}

void Frame::init(const TAD::ArrayContainer& a)
{
    reset();
    _originalArray = a;
    // Make room for min/max etc
    _minVals.resize(channelCount(), std::numeric_limits<float>::quiet_NaN());
    _maxVals.resize(channelCount(), std::numeric_limits<float>::quiet_NaN());
    _statistics.resize(channelCount());
    _histograms.resize(channelCount());
    // Determine color space, if any
    _colorSpace = ColorSpaceNone;
    _colorChannels[0] = -1;
    _colorChannels[1] = -1;
    _colorChannels[2] = -1;
    _alphaChannel = componentIndex(_originalArray, "ALPHA");
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(_originalArray, "GRAY");
        if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceLinearGray;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(_originalArray, "RED");
        _colorChannels[1] = componentIndex(_originalArray, "GREEN");
        _colorChannels[2] = componentIndex(_originalArray, "BLUE");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0) {
            _colorSpace = ColorSpaceLinearRGB;
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(_originalArray, "SRGB/GRAY");
        if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceSGray;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(_originalArray, "SRGB/R");
        _colorChannels[1] = componentIndex(_originalArray, "SRGB/G");
        _colorChannels[2] = componentIndex(_originalArray, "SRGB/B");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0
                && (_alphaChannel < 0 || _alphaChannel == 3)) {
            _colorSpace = ColorSpaceSRGB;
        }
        if (_colorSpace == ColorSpaceNone) {
            _colorChannels[0] = componentIndex(_originalArray, "SRGB/RED");
            _colorChannels[1] = componentIndex(_originalArray, "SRGB/GREEN");
            _colorChannels[2] = componentIndex(_originalArray, "SRGB/BLUE");
            if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0
                    && (_alphaChannel < 0 || _alphaChannel == 3)) {
                _colorSpace = ColorSpaceSRGB;
            }
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(_originalArray, "XYZ/X");
        _colorChannels[1] = componentIndex(_originalArray, "XYZ/Y");
        _colorChannels[2] = componentIndex(_originalArray, "XYZ/Z");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0) {
            _colorSpace = ColorSpaceXYZ;
        } else if (_colorChannels[1] >= 0) {
            _colorSpace = ColorSpaceY;
            _colorChannels[0] = _colorChannels[1];
            _colorChannels[2] = _colorChannels[1];
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _alphaChannel = -1;
    }
    if (_colorSpace == ColorSpaceLinearGray) {
        _colorMinVal = minVal(colorChannelIndex(0));
        _colorMaxVal = maxVal(colorChannelIndex(0));
        _colorVisMinVal = statistic(ColorChannelIndex).minVal();
        _colorVisMaxVal = statistic(ColorChannelIndex).maxVal();
    } else if (_colorSpace == ColorSpaceLinearRGB) {
        _colorMinVal = std::min(std::min(minVal(colorChannelIndex(0)), minVal(colorChannelIndex(1))), minVal(colorChannelIndex(2)));
        _colorMaxVal = std::max(std::max(maxVal(colorChannelIndex(0)), maxVal(colorChannelIndex(1))), maxVal(colorChannelIndex(2)));
        _colorVisMinVal = statistic(ColorChannelIndex).minVal();
        _colorVisMaxVal = statistic(ColorChannelIndex).maxVal();
    } else if (_colorSpace == ColorSpaceSGray || _colorSpace == ColorSpaceSRGB) {
        if (type() == TAD::uint8) {
            _colorMinVal = 0.0f;
            _colorMaxVal = 255.0f;
            _colorVisMinVal = 0.0f;
            _colorVisMaxVal = 100.0f;
        } else if (type() == TAD::uint16) {
            _colorMinVal = 0.0f;
            _colorMaxVal = 65535.0f;
            _colorVisMinVal = 0.0f;
            _colorVisMaxVal = 100.0f;
        } else {
            if (_colorSpace == ColorSpaceSGray) {
                _colorMinVal = minVal(colorChannelIndex(0));
                _colorMaxVal = maxVal(colorChannelIndex(0));
            } else {
                _colorMinVal = std::min(std::min(minVal(colorChannelIndex(0)), minVal(colorChannelIndex(1))), minVal(colorChannelIndex(2)));
                _colorMaxVal = std::max(std::max(maxVal(colorChannelIndex(0)), maxVal(colorChannelIndex(1))), maxVal(colorChannelIndex(2)));
            }
            _colorVisMinVal = statistic(ColorChannelIndex).minVal();
            _colorVisMaxVal = statistic(ColorChannelIndex).maxVal();
        }
    } else if (_colorSpace == ColorSpaceY) {
        _colorMinVal = minVal(colorChannelIndex(0));
        _colorMaxVal = maxVal(colorChannelIndex(0));
        _colorVisMinVal = _colorMinVal;
        _colorVisMaxVal = _colorMaxVal;
    } else if (_colorSpace == ColorSpaceXYZ) {
        _colorMinVal = minVal(colorChannelIndex(1));
        _colorMaxVal = maxVal(colorChannelIndex(1));
        _colorVisMinVal = _colorMinVal;
        _colorVisMaxVal = _colorMaxVal;
    }
    // Set initial channel
    _channelIndex = (_colorSpace != ColorSpaceNone ? ColorChannelIndex : 0);
    // Initialize quadtree representation. Quads in level 0 are never explicitly
    // stored in order to not duplicate the original data in memory
    TAD::Type quadType = TAD::float32;
    if (channelCount() <= 4) {
        // single texture
        GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
        _texFormat = formats[channelCount() - 1];
        if (type() == TAD::uint8 && (colorSpace() == ColorSpaceSGray || colorSpace() == ColorSpaceSRGB)) {
            _texInternalFormat = (colorSpace() == ColorSpaceSGray ? GL_SRGB8
                    : colorSpace() == ColorSpaceSRGB && !hasAlpha() ? GL_SRGB8
                    : GL_SRGB8_ALPHA8);
            _texType = GL_UNSIGNED_BYTE;
            quadType = TAD::uint8;
        } else {
            GLint internalFormats[4] = { GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
            _texInternalFormat = internalFormats[channelCount() - 1];
            _texType = GL_FLOAT;
        }
    } else {
        // one texture per channel
        _texInternalFormat = GL_R32F;
        _texFormat = GL_RED;
        _texType = GL_FLOAT;
    }
    _quadLevel0BorderSize = 1;
    std::vector<size_t> quadDims(2, 1022 + 2 * quadBorderSize(0));
    if (width() <= requiredMaxTextureSize && height() <= requiredMaxTextureSize) {
        // optimization for frames that fit into a single texture (covers 4K resolution)
        _quadLevel0BorderSize = 0;
        quadDims[0] = width();
        quadDims[1] = height();
    }
    _quadLevel0Description = TAD::ArrayDescription(quadDims, channelCount(), quadType);
    int level = 0;
    int quadsX = std::max(width() / quadWidth() + (width() % quadWidth() ? 1 : 0), 1);
    int quadsY = std::max(height() / quadHeight() + (height() % quadHeight() ? 1 : 0), 1);
    _quadTreeWidths.push_back(quadsX);
    _quadTreeHeights.push_back(quadsY);
    while (quadsX > 1 || quadsY > 1) {
        level++;
        quadsX = quadsX / 2 + quadsX % 2;
        quadsY = quadsY / 2 + quadsY % 2;
        _quadTreeWidths.push_back(quadsX);
        _quadTreeHeights.push_back(quadsY);
    }
}

void Frame::reset()
{
    if (_textureHolder.get())
        _textureHolder->clear();
    *this = Frame();
}

std::string Frame::channelName(int channelIndex) const
{
    std::string channelName;
    if (channelIndex == ColorChannelIndex) {
        channelName = "color";
    } else {
        channelName = std::to_string(channelIndex);
        switch (colorSpace()) {
        case ColorSpaceNone:
            break;
        case ColorSpaceLinearGray:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(gray)";
            break;
        case ColorSpaceLinearRGB:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(R)";
            else if (channelIndex == colorChannelIndex(1))
                channelName += "(G)";
            else if (channelIndex == colorChannelIndex(2))
                channelName += "(B)";
            break;
        case ColorSpaceSGray:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(sGray)";
            break;
        case ColorSpaceSRGB:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(sR)";
            else if (channelIndex == colorChannelIndex(1))
                channelName += "(sG)";
            else if (channelIndex == colorChannelIndex(2))
                channelName += "(sB)";
            break;
        case ColorSpaceY:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(Y)";
            break;
        case ColorSpaceXYZ:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(X)";
            else if (channelIndex == colorChannelIndex(1))
                channelName += "(Y)";
            else if (channelIndex == colorChannelIndex(2))
                channelName += "(Z)";
            break;
        }
        if (channelIndex == alphaChannelIndex())
            channelName += "(A)";
    }
    return channelName;
}

template<typename T>
float normalize(T value)
{
    // for integer types, convert original value range to [0,1]
    // for floating point types, do nothing
    float normalizedValue = value;
    if (std::is_integral<T>::value) {
        float minVal = std::numeric_limits<T>::min();
        float maxVal = std::numeric_limits<T>::max();
        normalizedValue = (value - minVal) / (maxVal - minVal);
    }
    return normalizedValue;
}

template<typename T>
static void lightnessArrayHelperLinearGray(float* lightness, size_t n, const T* src, int cc, int c)
{
    #pragma omp parallel for
    for (size_t e = 0; e < n; e++) {
        float v = normalize(src[e * cc + c]);
        lightness[e] = rgbToL(v, v, v);
    }
}

template<typename T>
static void lightnessArrayHelperLinearRGB(float* lightness, size_t n, const T* src, int cc, int cr, int cg, int cb)
{
    #pragma omp parallel for
    for (size_t e = 0; e < n; e++) {
        float r = normalize(src[e * cc + cr]);
        float g = normalize(src[e * cc + cg]);
        float b = normalize(src[e * cc + cb]);
        lightness[e] = rgbToL(r, g, b);
    }
}

template<typename T>
static void lightnessArrayHelperSGray(float* lightness, size_t n, const T* src, int cc, int c)
{
    #pragma omp parallel for
    for (size_t e = 0; e < n; e++) {
        float v = toLinear(normalize(src[e * cc + c]));
        lightness[e] = rgbToL(v, v, v);
    }
}

template<typename T>
static void lightnessArrayHelperSRGB(float* lightness, size_t n, const T* src, int cc, int cr, int cg, int cb)
{
    #pragma omp parallel for
    for (size_t e = 0; e < n; e++) {
        float r = toLinear(normalize(src[e * cc + cr]));
        float g = toLinear(normalize(src[e * cc + cg]));
        float b = toLinear(normalize(src[e * cc + cb]));
        lightness[e] = rgbToL(r, g, b);
    }
}

template<typename T>
static void lightnessArrayHelperY(float* lightness, size_t n, const T* src, int cc, int c)
{
    #pragma omp parallel for
    for (size_t e = 0; e < n; e++) {
        float v = normalize(src[e * cc + c]);
        if (std::is_integral<T>::value)
            v *= 100.0f;
        lightness[e] = YToL(v);
    }
}

const TAD::Array<float>& Frame::lightnessArray()
{
    if (_lightnessArray.elementCount() == 0) {
        _lightnessArray = TAD::Array<float>(_originalArray.dimensions(), 1);
        float* lightness = static_cast<float*>(_lightnessArray.data());
        size_t n = _lightnessArray.elementCount();
        int cc = _originalArray.componentCount();
        if (colorSpace() == ColorSpaceLinearGray) {
            int c = colorChannelIndex(0);
            switch (type()) {
            case TAD::int8:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint8:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int16:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint16:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::float32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TAD::float64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const double*>(_originalArray.data()), cc, c);
                break;
            }
        } else if (colorSpace() == ColorSpaceLinearRGB) {
            int cr = colorChannelIndex(0);
            int cg = colorChannelIndex(1);
            int cb = colorChannelIndex(2);
            switch (type()) {
            case TAD::int8:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint8:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int16:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint16:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::float32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const float*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::float64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const double*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            }
        } else if (colorSpace() == ColorSpaceSGray) {
            int c = colorChannelIndex(0);
            switch (type()) {
            case TAD::int8:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint8:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int16:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint16:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::float32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TAD::float64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const double*>(_originalArray.data()), cc, c);
                break;
            }
        } else if (colorSpace() == ColorSpaceSRGB) {
            int cr = colorChannelIndex(0);
            int cg = colorChannelIndex(1);
            int cb = colorChannelIndex(2);
            switch (type()) {
            case TAD::int8:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint8:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int16:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint16:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::int64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::uint64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::float32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const float*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TAD::float64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const double*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            }
        } else if (colorSpace() == ColorSpaceY || colorSpace() == ColorSpaceXYZ) {
            int c = colorChannelIndex(colorSpace() == ColorSpaceY ? 0 : 1);
            switch (type()) {
            case TAD::int8:
                lightnessArrayHelperY(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint8:
                lightnessArrayHelperY(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int16:
                lightnessArrayHelperY(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint16:
                lightnessArrayHelperY(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int32:
                lightnessArrayHelperY(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint32:
                lightnessArrayHelperY(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::int64:
                lightnessArrayHelperY(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::uint64:
                lightnessArrayHelperY(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TAD::float32:
                lightnessArrayHelperY(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TAD::float64:
                lightnessArrayHelperY(lightness, n, static_cast<const double*>(_originalArray.data()), cc, c);
                break;
            }
        }
    }
    return _lightnessArray;
}

float Frame::value(int x, int y, int channelIndex)
{
    float v = std::numeric_limits<float>::quiet_NaN();
    if (x >= 0 && x < width() && y >= 0 && y < height()) {
        if (channelIndex == ColorChannelIndex) {
            v = lightnessArray()[{ size_t(x), size_t(y) }][0];
        } else {
            switch (type()) {
            case TAD::int8:
                v = _originalArray.get<int8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::uint8:
                v = _originalArray.get<uint8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::int16:
                v = _originalArray.get<int16_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::uint16:
                v = _originalArray.get<uint16_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::int32:
                v = _originalArray.get<int32_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::uint32:
                v = _originalArray.get<uint32_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::int64:
                v = _originalArray.get<int64_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::uint64:
                v = _originalArray.get<uint64_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::float32:
                v = _originalArray.get<float>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TAD::float64:
                v = _originalArray.get<double>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            }
        }
    }
    return v;
}

float Frame::minVal(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return _colorMinVal;
    } else {
        if (!std::isfinite(_minVals[channelIndex])) {
            if (type() == TAD::uint8 && colorSpace() != ColorSpaceNone)
                _minVals[channelIndex] = 0.0f;
            _originalArray.componentTagList(channelIndex).value("MINVAL", &(_minVals[channelIndex]));
            if (!std::isfinite(_minVals[channelIndex]))
                _minVals[channelIndex] = statistic(channelIndex).minVal();
        }
        return _minVals[channelIndex];
    }
}

float Frame::maxVal(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return _colorMaxVal;
    } else {
        if (!std::isfinite(_maxVals[channelIndex])) {
            if (type() == TAD::uint8 && colorSpace() != ColorSpaceNone)
                _maxVals[channelIndex] = 255.0f;
            _originalArray.componentTagList(channelIndex).value("MAXVAL", &(_maxVals[channelIndex]));
            if (!std::isfinite(_maxVals[channelIndex]))
                _maxVals[channelIndex] = statistic(channelIndex).maxVal();
        }
        return _maxVals[channelIndex];
    }
}

float Frame::visMinVal(int channelIndex)
{
    return channelIndex == ColorChannelIndex ? _colorVisMinVal : minVal(channelIndex);
}

float Frame::visMaxVal(int channelIndex)
{
    return channelIndex == ColorChannelIndex ? _colorVisMaxVal : maxVal(channelIndex);
}

const Statistic& Frame::statistic(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (_colorStatistic.finiteValues() < 0) {
            _colorStatistic.init(lightnessArray(), 0);
        }
        return _colorStatistic;
    } else {
        if (_statistics[channelIndex].finiteValues() < 0) {
            _statistics[channelIndex].init(_originalArray, channelIndex);
        }
        return _statistics[channelIndex];
    }
}

const Histogram& Frame::histogram(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (_colorHistogram.binCount() == 0) {
            _colorHistogram.init(lightnessArray(), 0, visMinVal(ColorChannelIndex), visMaxVal(ColorChannelIndex));
        }
        return _colorHistogram;
    } else {
        if (_histograms[channelIndex].binCount() == 0) {
            _histograms[channelIndex].init(_originalArray, channelIndex,
                    type() == TAD::uint8 ?   0.0f : minVal(channelIndex),
                    type() == TAD::uint8 ? 255.0f : maxVal(channelIndex));
        }
        return _histograms[channelIndex];
    }
}

void Frame::setChannelIndex(int index)
{
    if (index == ColorChannelIndex)
        assert(colorSpace() != ColorSpaceNone);
    else
        assert(index >= 0 && index < channelCount());
    _channelIndex = index;
}

TAD::ArrayContainer Frame::quadFromLevel0(int qx, int qy)
{
    assert(qx >= 0 && qx < quadTreeLevelWidth(0));
    assert(qy >= 0 && qy < quadTreeLevelHeight(0));

    /* Optimization for the case of only a single quad */
    if (_quadLevel0BorderSize == 0
            && _quadLevel0Description.dimension(0) == _originalArray.dimension(0)
            && _quadLevel0Description.dimension(1) == _originalArray.dimension(1)
            && _quadLevel0Description.componentCount() == _originalArray.componentCount()) {
        return convert(_originalArray, _quadLevel0Description.componentType());
    }

    /* First create quad using original data type */
    TAD::ArrayContainer q(_quadLevel0Description.dimensions(),
            _quadLevel0Description.componentCount(), type());
    const TAD::ArrayContainer& src = _originalArray;
    int srcX = qx * quadWidth() - quadBorderSize(0);
    int srcY = qy * quadHeight() - quadBorderSize(0);

    if (srcX >= 0 && srcY >= 0
            && srcX + int(q.dimension(0)) < width()
            && srcY + int(q.dimension(1)) < height()) {
        // case 1: copy the whole block
        for (size_t y = 0; y < q.dimension(1); y++) {
            std::memcpy(q.get({ 0, y }),
                    src.get({ size_t(srcX), size_t(srcY + y) }),
                    q.dimension(0) * q.elementSize());
        }
    } else {
        // case 2: border coordinates need to be clamped
        int copyableBlockMinX = srcX;
        if (copyableBlockMinX < 0)
            copyableBlockMinX = 0;
        int copyableBlockMaxX = srcX + q.dimension(0) - 1;
        if (copyableBlockMaxX >= width())
            copyableBlockMaxX = width() - 1;
        int copyableBlockMinY = srcY;
        if (copyableBlockMinY < 0)
            copyableBlockMinY = 0;
        int copyableBlockMaxY = srcY + q.dimension(1) - 1;
        if (copyableBlockMaxY >= height())
            copyableBlockMaxY = height() - 1;
        for (int y = 0; y < copyableBlockMinY - srcY; y++) {
            for (int x = 0; x < copyableBlockMinX - srcX; x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(0), size_t(0) }),
                        q.elementSize());
            }
            std::memcpy(q.get({ size_t(copyableBlockMinX - srcX), size_t(y) }),
                    src.get({ size_t(copyableBlockMinX), size_t(0) }),
                    (copyableBlockMaxX - copyableBlockMinX + 1) * q.elementSize());
            for (int x = copyableBlockMaxX - srcX + 1; x < int(q.dimension(0)); x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(width() - 1), size_t(0) }),
                        q.elementSize());
            }
        }
        for (int y = copyableBlockMinY - srcY; y <= copyableBlockMaxY - srcY; y++) {
            for (int x = 0; x < copyableBlockMinX - srcX; x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(0), size_t(srcY + y) }),
                        q.elementSize());
            }
            std::memcpy(q.get({ size_t(copyableBlockMinX - srcX), size_t(y) }),
                    src.get({ size_t(copyableBlockMinX), size_t(srcY + y) }),
                    (copyableBlockMaxX - copyableBlockMinX + 1) * q.elementSize());
            for (int x = copyableBlockMaxX - srcX + 1; x < int(q.dimension(0)); x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(width() - 1), size_t(srcY + y) }),
                        q.elementSize());
            }
        }
        for (int y = copyableBlockMaxY - srcY + 1; y < int(q.dimension(1)); y++) {
            for (int x = 0; x < copyableBlockMinX - srcX; x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(0), size_t(height() - 1) }),
                        q.elementSize());
            }
            std::memcpy(q.get({ size_t(copyableBlockMinX - srcX), size_t(y) }),
                    src.get({ size_t(copyableBlockMinX), size_t(height() - 1) }),
                    (copyableBlockMaxX - copyableBlockMinX + 1) * q.elementSize());
            for (int x = copyableBlockMaxX - srcX + 1; x < int(q.dimension(0)); x++) {
                std::memcpy(q.get({ size_t(x), size_t(y) }),
                        src.get({ size_t(width() - 1), size_t(height() - 1) }),
                        q.elementSize());
            }
        }
    }
#if 0
    for (size_t y = 0; y < q.dimension(1); y++) {
        int sy = srcY + y;
        if (sy < 0)
            sy = 0;
        else if (sy >= height())
            sy = height() - 1;
        for (size_t x = 0; x < q.dimension(0); x++) {
            const void* quadElement = q.get({ x, y });
            int sx = srcX + x;
            if (sx < 0)
                sx = 0;
            else if (sx >= width())
                sx = width() - 1;
            const void* srcElement = src.get({ sx, sy });
            assert(memcmp(quadElement, srcElement, q.elementSize()) == 0);
            if (memcmp(quadElement, srcElement, q.elementSize()) != 0) {
                fprintf(stderr, "FAIL\n");
            }
        }
    }
#endif

    /* Then convert if necessary */
    return convert(q, _quadLevel0Description.componentType());
}

bool Frame::textureChannelIsS(int texChannel)
{
    return (channelCount() <= 4 && type() == TAD::uint8
            && ((colorSpace() == ColorSpaceSGray && colorChannelIndex(0) == texChannel)
                || (colorSpace() == ColorSpaceSRGB &&
                    (colorChannelIndex(0) == texChannel
                     || colorChannelIndex(1) == texChannel
                     || colorChannelIndex(2) == texChannel))));
}

static void uploadArrayToTexture(const TAD::ArrayContainer& array,
        unsigned int texture,
        GLint internalFormat, GLenum format, GLenum type)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    size_t lineSize = array.dimension(0) * array.elementSize();
    if (lineSize % 4 == 0)
        gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else if (lineSize % 2 == 0)
        gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    else
        gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl->glBindTexture(GL_TEXTURE_2D, texture);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
            array.dimension(0), array.dimension(1), 0,
            format, type, array.data());
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ASSERT_GLCHECK();
}

unsigned int Frame::quadTexture(int level, int qx, int qy, int channelIndex,
        unsigned int fbo, unsigned int vao, QOpenGLShaderProgram& quadTreePrg)
{
    if (!_textureHolder.get())
        _textureHolder = std::make_shared<TextureHolder>();

    int texturesPerQuad = (channelCount() <= 4 ? 1 : channelCount());
    if (_textureHolder->size() == 0) {
        int totalQuads = 0;
        for (int l = 0; l < quadTreeLevels(); l++)
            totalQuads += quadTreeLevelWidth(l) * quadTreeLevelHeight(l);
        _textureHolder->create(totalQuads * texturesPerQuad);
    }
    int textureIndex = 0;
    for (int l = 0; l < level; l++)
        textureIndex += quadTreeLevelWidth(l) * quadTreeLevelHeight(l);
    textureIndex += qy * quadTreeLevelWidth(level) + qx;
    textureIndex *= texturesPerQuad;
    if (texturesPerQuad != 1)
        textureIndex += channelIndex;

    if (!_textureHolder->flag(textureIndex)) {
        auto gl = getGlFunctionsFromCurrentContext();
        if (level == 0) {
            if (channelCount() <= 4) {
                // single texture
                uploadArrayToTexture(quadFromLevel0(qx, qy), _textureHolder->texture(textureIndex),
                        _texInternalFormat, _texFormat, _texType);
            } else {
                // one texture per channel
                if (_textureTransferArray.dimensionCount() == 0)
                    _textureTransferArray = TAD::Array<float>({ size_t(quadWidth()), size_t(quadHeight()) }, 1);
                const TAD::Array<float> origQuad = quadFromLevel0(qx, qy);
                for (size_t e = 0; e < _textureTransferArray.elementCount(); e++)
                    _textureTransferArray.set<float>(e, 0, origQuad.get<float>(e, channelIndex));
                uploadArrayToTexture(_textureTransferArray, _textureHolder->texture(textureIndex),
                        _texInternalFormat, _texFormat, _texType);
            }
        } else {
            ASSERT_GLCHECK();
            // get source quad textures
            unsigned int quadTex0 = 0, quadTex1 = 0, quadTex2 = 0, quadTex3 = 0;
            if (2 * qx + 0 < quadTreeLevelWidth(level - 1) && 2 * qy + 0 < quadTreeLevelHeight(level - 1))
                quadTex0 = quadTexture(level - 1, 2 * qx + 0, 2 * qy + 0, channelIndex, fbo, vao, quadTreePrg);
            if (2 * qx + 1 < quadTreeLevelWidth(level - 1) && 2 * qy + 0 < quadTreeLevelHeight(level - 1))
                quadTex1 = quadTexture(level - 1, 2 * qx + 1, 2 * qy + 0, channelIndex, fbo, vao, quadTreePrg);
            if (2 * qx + 0 < quadTreeLevelWidth(level - 1) && 2 * qy + 1 < quadTreeLevelHeight(level - 1))
                quadTex2 = quadTexture(level - 1, 2 * qx + 0, 2 * qy + 1, channelIndex, fbo, vao, quadTreePrg);
            if (2 * qx + 1 < quadTreeLevelWidth(level - 1) && 2 * qy + 1 < quadTreeLevelHeight(level - 1))
                quadTex3 = quadTexture(level - 1, 2 * qx + 1, 2 * qy + 1, channelIndex, fbo, vao, quadTreePrg);
            // backup GL state
            int fboBak, prgBak, vaoBak, vpBak[4];
            gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboBak);
            gl->glGetIntegerv(GL_CURRENT_PROGRAM, &prgBak);
            gl->glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBak);
            gl->glGetIntegerv(GL_VIEWPORT, vpBak);
            // create new tex
            gl->glBindTexture(GL_TEXTURE_2D, _textureHolder->texture(textureIndex));
            gl->glTexImage2D(GL_TEXTURE_2D, 0, _texInternalFormat, quadWidth(), quadHeight(), 0,
                    _texFormat, _texType, nullptr);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // setup fbo
            gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureHolder->texture(textureIndex), 0);
            gl->glViewport(0, 0, quadWidth(), quadHeight());
            // render into new tex
            gl->glUseProgram(quadTreePrg.programId());
            quadTreePrg.setUniformValue("nan", std::numeric_limits<float>::quiet_NaN());
            quadTreePrg.setUniformValue("quadTex0", 0);
            quadTreePrg.setUniformValue("quadTex1", 1);
            quadTreePrg.setUniformValue("quadTex2", 2);
            quadTreePrg.setUniformValue("quadTex3", 3);
            quadTreePrg.setUniformValue("haveQuadTex0", quadTex0 != 0);
            quadTreePrg.setUniformValue("haveQuadTex1", quadTex1 != 0);
            quadTreePrg.setUniformValue("haveQuadTex2", quadTex2 != 0);
            quadTreePrg.setUniformValue("haveQuadTex3", quadTex3 != 0);
            quadTreePrg.setUniformValue("toS0", textureChannelIsS(0));
            quadTreePrg.setUniformValue("toS1", textureChannelIsS(1));
            quadTreePrg.setUniformValue("toS2", textureChannelIsS(2));
            quadTreePrg.setUniformValue("toS3", textureChannelIsS(3));
            float quadWidthWithBorder = quadWidth() + 2 * quadBorderSize(level - 1);
            float quadHeightWithBorder = quadHeight() + 2 * quadBorderSize(level - 1);
            float texCoordFactorX = quadWidth() / quadWidthWithBorder;
            float texCoordFactorY = quadHeight() / quadHeightWithBorder;
            float texCoordOffsetX = quadBorderSize(level) / quadWidthWithBorder;
            float texCoordOffsetY = quadBorderSize(level) / quadHeightWithBorder;
            quadTreePrg.setUniformValue("texCoordFactorX", texCoordFactorX);
            quadTreePrg.setUniformValue("texCoordFactorY", texCoordFactorY);
            quadTreePrg.setUniformValue("texCoordOffsetX", texCoordOffsetX);
            quadTreePrg.setUniformValue("texCoordOffsetY", texCoordOffsetY);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, quadTex0);
            gl->glActiveTexture(GL_TEXTURE1);
            gl->glBindTexture(GL_TEXTURE_2D, quadTex1);
            gl->glActiveTexture(GL_TEXTURE2);
            gl->glBindTexture(GL_TEXTURE_2D, quadTex2);
            gl->glActiveTexture(GL_TEXTURE3);
            gl->glBindTexture(GL_TEXTURE_2D, quadTex3);
            gl->glBindVertexArray(vao);
            gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            // Restore GL state
            gl->glBindFramebuffer(GL_FRAMEBUFFER, fboBak);
            gl->glUseProgram(prgBak);
            gl->glBindVertexArray(vaoBak);
            gl->glViewport(vpBak[0], vpBak[1], vpBak[2], vpBak[3]);
            ASSERT_GLCHECK();
        }
        // generate mipmap only for the highest level
        if (level == quadTreeLevels() - 1) {
            gl->glBindTexture(GL_TEXTURE_2D, _textureHolder->texture(textureIndex));
            if (isOpenGLES()) {
                // mipmap generation does not seem to work reliably!?
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            } else {
                gl->glGenerateMipmap(GL_TEXTURE_2D);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
        }
        ASSERT_GLCHECK();
        _textureHolder->setFlag(textureIndex);
    }
    return _textureHolder->texture(textureIndex);
}

bool Frame::haveLightness() const
{
    return _lightnessArray.elementCount() > 0;
}

bool Frame::haveStatistic(int channelIndex) const
{
    if (channelIndex == ColorChannelIndex) {
        return _colorStatistic.finiteValues() >= 0;
    } else {
        return _statistics[channelIndex].finiteValues() >= 0;
    }
}

bool Frame::haveHistogram(int channelIndex) const
{
    if (channelIndex == ColorChannelIndex) {
        return _colorHistogram.binCount() > 0;
    } else {
        return _histograms[channelIndex].binCount() > 0;
    }
}
