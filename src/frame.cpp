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

#include <limits>
#include <type_traits>
#include <cmath>

#include "frame.hpp"
#include "alloc.hpp"
#include "gl.hpp"


Frame::Frame() :
    _colorSpace(ColorSpaceNone), _colorChannels { -1, -1, -1 }, _alphaChannel(-1),
    _channelIndex(-1)
{
}

static int componentIndex(const TGD::ArrayContainer& a, const std::string& interpretationValue)
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

void Frame::init(const TGD::ArrayContainer& a)
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
        if (type() == TGD::uint8) {
            _colorMinVal = 0.0f;
            _colorMaxVal = 255.0f;
            _colorVisMinVal = 0.0f;
            _colorVisMaxVal = 100.0f;
        } else if (type() == TGD::uint16) {
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
    TGD::Type quadType = TGD::float32;
    if (channelCount() <= 4) {
        // single texture
        GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
        _texFormat = formats[channelCount() - 1];
        if (type() == TGD::uint8 && (colorSpace() == ColorSpaceSGray || colorSpace() == ColorSpaceSRGB)) {
            _texInternalFormat = (colorSpace() == ColorSpaceSGray ? GL_SRGB8
                    : colorSpace() == ColorSpaceSRGB && !hasAlpha() ? GL_SRGB8
                    : GL_SRGB8_ALPHA8);
            _texType = GL_UNSIGNED_BYTE;
            quadType = TGD::uint8;
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
    _quadLevel0Description = TGD::ArrayDescription(quadDims, channelCount(), quadType);
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

const TGD::Array<float>& Frame::lightnessArray()
{
    if (_lightnessArray.elementCount() == 0) {
        _lightnessArray = TGD::Array<float>(_originalArray.dimensions(), 1, defaultAllocator());
        float* lightness = static_cast<float*>(_lightnessArray.data());
        size_t n = _lightnessArray.elementCount();
        int cc = _originalArray.componentCount();
        if (colorSpace() == ColorSpaceLinearGray) {
            int c = colorChannelIndex(0);
            switch (type()) {
            case TGD::int8:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint8:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int16:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint16:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::float32:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TGD::float64:
                lightnessArrayHelperLinearGray(lightness, n, static_cast<const double*>(_originalArray.data()), cc, c);
                break;
            }
        } else if (colorSpace() == ColorSpaceLinearRGB) {
            int cr = colorChannelIndex(0);
            int cg = colorChannelIndex(1);
            int cb = colorChannelIndex(2);
            switch (type()) {
            case TGD::int8:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint8:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int16:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint16:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::float32:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const float*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::float64:
                lightnessArrayHelperLinearRGB(lightness, n, static_cast<const double*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            }
        } else if (colorSpace() == ColorSpaceSGray) {
            int c = colorChannelIndex(0);
            switch (type()) {
            case TGD::int8:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint8:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int16:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint16:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::float32:
                lightnessArrayHelperSGray(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TGD::float64:
                lightnessArrayHelperSGray(lightness, n, static_cast<const double*>(_originalArray.data()), cc, c);
                break;
            }
        } else if (colorSpace() == ColorSpaceSRGB) {
            int cr = colorChannelIndex(0);
            int cg = colorChannelIndex(1);
            int cb = colorChannelIndex(2);
            switch (type()) {
            case TGD::int8:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint8:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int16:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint16:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::int64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::uint64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::float32:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const float*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            case TGD::float64:
                lightnessArrayHelperSRGB(lightness, n, static_cast<const double*>(_originalArray.data()), cc, cr, cg, cb);
                break;
            }
        } else if (colorSpace() == ColorSpaceY || colorSpace() == ColorSpaceXYZ) {
            int c = colorChannelIndex(colorSpace() == ColorSpaceY ? 0 : 1);
            switch (type()) {
            case TGD::int8:
                lightnessArrayHelperY(lightness, n, static_cast<const int8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint8:
                lightnessArrayHelperY(lightness, n, static_cast<const uint8_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int16:
                lightnessArrayHelperY(lightness, n, static_cast<const int16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint16:
                lightnessArrayHelperY(lightness, n, static_cast<const uint16_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int32:
                lightnessArrayHelperY(lightness, n, static_cast<const int32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint32:
                lightnessArrayHelperY(lightness, n, static_cast<const uint32_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::int64:
                lightnessArrayHelperY(lightness, n, static_cast<const int64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::uint64:
                lightnessArrayHelperY(lightness, n, static_cast<const uint64_t*>(_originalArray.data()), cc, c);
                break;
            case TGD::float32:
                lightnessArrayHelperY(lightness, n, static_cast<const float*>(_originalArray.data()), cc, c);
                break;
            case TGD::float64:
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
            case TGD::int8:
                v = _originalArray.get<int8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::uint8:
                v = _originalArray.get<uint8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::int16:
                v = _originalArray.get<int16_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::uint16:
                v = _originalArray.get<uint16_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::int32:
                v = _originalArray.get<int32_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::uint32:
                v = _originalArray.get<uint32_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::int64:
                v = _originalArray.get<int64_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::uint64:
                v = _originalArray.get<uint64_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::float32:
                v = _originalArray.get<float>({ size_t(x), size_t(y) }, size_t(channelIndex));
                break;
            case TGD::float64:
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
            if (type() == TGD::uint8 && colorSpace() != ColorSpaceNone)
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
            if (type() == TGD::uint8 && colorSpace() != ColorSpaceNone)
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
        if (!_colorStatistic.initialized()) {
            _colorStatistic.init(lightnessArray(), 0);
        }
        return _colorStatistic;
    } else {
        if (!_statistics[channelIndex].initialized()) {
            _statistics[channelIndex].init(_originalArray, channelIndex);
        }
        return _statistics[channelIndex];
    }
}

const Histogram& Frame::histogram(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (!_colorHistogram.initialized()) {
            _colorHistogram.init(lightnessArray(), 0, visMinVal(ColorChannelIndex), visMaxVal(ColorChannelIndex));
        }
        return _colorHistogram;
    } else {
        if (!_histograms[channelIndex].initialized()) {
            _histograms[channelIndex].init(_originalArray, channelIndex,
                    type() == TGD::uint8 ?   0.0f : minVal(channelIndex),
                    type() == TGD::uint8 ? 255.0f : maxVal(channelIndex));
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

int Frame::quadIndex(int level, int qx, int qy) const // returns -1 if nonexistent
{
    int i = -1;
    if (qx >= 0 && qx < quadTreeLevelWidth(level)
            && qy >= 0 && qy < quadTreeLevelHeight(level)) {
        i = 0;
        for (int l = 0; l < level; l++)
            i += quadTreeLevelWidth(l) * quadTreeLevelHeight(l);
        i += qy * quadTreeLevelWidth(level) + qx;
    }
    return i;
}

void Frame::computeQuadOnLevel0Worker(TGD::ArrayContainer& q, int qx, int qy) const
{
    const TGD::ArrayContainer& src = _originalArray;
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
}

void Frame::computeQuadOnLevel0(TGD::ArrayContainer& quad, int qx, int qy)
{
    assert(qx >= 0 && qx < quadTreeLevelWidth(0));
    assert(qy >= 0 && qy < quadTreeLevelHeight(0));

    if (_quadLevel0Description.componentType() == type()) {
        // write results directly into quad
        computeQuadOnLevel0Worker(quad, qx, qy);
    } else {
        // compute in original data type first
        if (_quadLevel0Tmp.dimensionCount() == 0) {
            _quadLevel0Tmp = TGD::ArrayContainer(
                    _quadLevel0Description.dimensions(),
                    _quadLevel0Description.componentCount(),
                    type(), defaultAllocator());
        }
        computeQuadOnLevel0Worker(_quadLevel0Tmp, qx, qy);
        // convert
        convert(quad, _quadLevel0Tmp);
    }
}

bool Frame::textureChannelIsS(int texChannel) const
{
    return (channelCount() <= 4 && type() == TGD::uint8
            && ((colorSpace() == ColorSpaceSGray && colorChannelIndex(0) == texChannel)
                || (colorSpace() == ColorSpaceSRGB &&
                    (colorChannelIndex(0) == texChannel
                     || colorChannelIndex(1) == texChannel
                     || colorChannelIndex(2) == texChannel))));
}

static float sToLinear(float x)
{
    const float c0 = 0.077399380805f; // 1.0 / 12.92
    const float c1 = 0.947867298578f; // 1.0 / 1.055;
    return (x <= 0.04045f ? (x * c0) : std::powf((x + 0.055f) * c1, 2.4f));
}

static float linearToS(float x)
{
    const float c0 = 0.416666666667f;
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * std::powf(x, c0) - 0.055f));
}

static void interpolate(TGD::ArrayContainer& dst,
        size_t dstXOffset, size_t dstYOffset, size_t w, size_t h,
        const TGD::ArrayContainer& src, size_t srcXOffset, size_t srcYOffset,
        const bool isS[4])
{
    if (dst.componentType() == TGD::uint8) {
        TGD::Array<uint8_t> d = dst;
        TGD::Array<uint8_t> s = src;
        for (size_t y = 0; y < h; y++) {
            for (size_t x = 0; x < w; x++) {
                size_t dste = (dstYOffset + y) * dst.dimension(0) + (dstXOffset + x);
                size_t srce0 = (srcYOffset + 2 * y + 0) * src.dimension(0) + (srcXOffset + 2 * x + 0);
                size_t srce1 = (srcYOffset + 2 * y + 0) * src.dimension(0) + (srcXOffset + 2 * x + 1);
                size_t srce2 = (srcYOffset + 2 * y + 1) * src.dimension(0) + (srcXOffset + 2 * x + 0);
                size_t srce3 = (srcYOffset + 2 * y + 1) * src.dimension(0) + (srcXOffset + 2 * x + 1);
                for (size_t c = 0; c < dst.componentCount(); c++) {
                    float v;
                    if (isS[c]) {
                        v = linearToS((sToLinear(s[srce0][c]) + sToLinear(s[srce1][c]) + sToLinear(s[srce2][c]) + sToLinear(s[srce3][c])) * 0.25f);
                    } else {
                        v = (s[srce0][c] + s[srce1][c] + s[srce2][c] + s[srce3][c]) * 0.25f;
                    }
                    d[dste][c] = std::round(v);
                }
            }
        }
    } else {
        TGD::Array<float> d = dst;
        TGD::Array<float> s = src;
        for (size_t y = 0; y < h; y++) {
            for (size_t x = 0; x < w; x++) {
                size_t dste = (dstYOffset + y) * dst.dimension(0) + (dstXOffset + x);
                size_t srce0 = (srcYOffset + 2 * y + 0) * src.dimension(0) + (srcXOffset + 2 * x + 0);
                size_t srce1 = (srcYOffset + 2 * y + 0) * src.dimension(0) + (srcXOffset + 2 * x + 1);
                size_t srce2 = (srcYOffset + 2 * y + 1) * src.dimension(0) + (srcXOffset + 2 * x + 0);
                size_t srce3 = (srcYOffset + 2 * y + 1) * src.dimension(0) + (srcXOffset + 2 * x + 1);
                for (size_t c = 0; c < dst.componentCount(); c++) {
                    d[dste][c] = (s[srce0][c] + s[srce1][c] + s[srce2][c] + s[srce3][c]) * 0.25f;
                }
            }
        }
    }
}

static void setInvalid(TGD::ArrayContainer& dst,
        size_t dstXOffset, size_t dstYOffset, size_t w, size_t h)
{
    if (!defaultAllocator().clearsMemory()) {
        for (size_t y = 0; y < h; y++) {
            size_t dste = (y + dstYOffset) * dst.dimension(0) + dstXOffset;
            std::memset(dst.get(dste), 0, w * dst.elementSize());
        }
    }
}

void Frame::computeQuadOnLevel(TGD::ArrayContainer& q, int level, int qx, int qy) const
{
    assert(q.componentType() == TGD::uint8 || q.componentType() == TGD::float32);
    assert(level >= 1);

    int q0Index = quadIndex(level - 1, 2 * qx + 0, 2 * qy + 0);
    int q1Index = quadIndex(level - 1, 2 * qx + 1, 2 * qy + 0);
    int q2Index = quadIndex(level - 1, 2 * qx + 0, 2 * qy + 1);
    int q3Index = quadIndex(level - 1, 2 * qx + 1, 2 * qy + 1);
    size_t srcXOffset = (level == 1 ? _quadLevel0BorderSize : 0);
    size_t srcYOffset = (level == 1 ? _quadLevel0BorderSize : 0);
    size_t w = quadWidth() / 2;
    size_t h = quadHeight() / 2;
    bool isS[4] = { textureChannelIsS(0), textureChannelIsS(1), textureChannelIsS(2), textureChannelIsS(3) };

    size_t dstXOffset = 0;
    size_t dstYOffset = 0;
    if (q0Index >= 0) {
        interpolate(q, dstXOffset, dstYOffset, w, h, _quads[q0Index], srcXOffset, srcYOffset, isS);
    } else {
        setInvalid(q, dstXOffset, dstYOffset, w, h);
    }

    dstXOffset = w;
    dstYOffset = 0;
    if (q1Index >= 0) {
        interpolate(q, dstXOffset, dstYOffset, w, h, _quads[q1Index], srcXOffset, srcYOffset, isS);
    } else {
        setInvalid(q, dstXOffset, dstYOffset, w, h);
    }

    dstXOffset = 0;
    dstYOffset = h;
    if (q2Index >= 0) {
        interpolate(q, dstXOffset, dstYOffset, w, h, _quads[q2Index], srcXOffset, srcYOffset, isS);
    } else {
        setInvalid(q, dstXOffset, dstYOffset, w, h);
    }

    dstXOffset = w;
    dstYOffset = h;
    if (q3Index >= 0) {
        interpolate(q, dstXOffset, dstYOffset, w, h, _quads[q3Index], srcXOffset, srcYOffset, isS);
    } else {
        setInvalid(q, dstXOffset, dstYOffset, w, h);
    }
}

static void uploadArrayToTexture(const TGD::ArrayContainer& array,
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

void Frame::quadSubtreeNeedsRecomputing(int level, int qx, int qy)
{
    int qi = quadIndex(level, qx, qy);
    if (qi < 0 || size_t(qi) >= _quadNeedsRecomputing.size())
        return;
    _quadNeedsRecomputing[qi] = true;
    if (level > 0) {
        quadSubtreeNeedsRecomputing(level - 1, 2 * qx + 0, 2 * qy + 0);
        quadSubtreeNeedsRecomputing(level - 1, 2 * qx + 1, 2 * qy + 0);
        quadSubtreeNeedsRecomputing(level - 1, 2 * qx + 0, 2 * qy + 1);
        quadSubtreeNeedsRecomputing(level - 1, 2 * qx + 1, 2 * qy + 1);
    }
}

void Frame::prepareQuadsForRendering(const std::vector<std::tuple<int, int, int>>& relevantQuads, bool refreshQuads)
{
    if (refreshQuads) {
        for (size_t i = 0; i < relevantQuads.size(); i++) {
            quadSubtreeNeedsRecomputing(
                    std::get<0>(relevantQuads[i]),
                    std::get<1>(relevantQuads[i]),
                    std::get<2>(relevantQuads[i]));
        }
    }
}

void Frame::uploadQuadToTexture(unsigned int tex, int level, int qx, int qy, int channelIndex)
{
    /* Optimization for the case of only a single quad */
    if (_quadLevel0BorderSize == 0
            && _quadLevel0Description.dimension(0) == _originalArray.dimension(0)
            && _quadLevel0Description.dimension(1) == _originalArray.dimension(1)
            && _quadLevel0Description.componentCount() == _originalArray.componentCount()
            && _quadLevel0Description.componentType() == _originalArray.componentType()) {
        if (_quads.size() == 0) {
            _quads.resize(1);
            _quads[0] = _originalArray;
            _quadNeedsRecomputing.resize(1);
        }
        _quadNeedsRecomputing[0] = false;
    }

    /* Create quads if not done yet */
    if (_quads.size() == 0) {
        // Reserve space for all quads
        size_t level0Quads = quadTreeLevelWidth(0) * quadTreeLevelHeight(0);
        size_t totalQuads = level0Quads;
        for (int l = 1; l < quadTreeLevels(); l++) {
            totalQuads += quadTreeLevelWidth(l) * quadTreeLevelHeight(l);
        }
        _quads.resize(totalQuads);
        _quadNeedsRecomputing.resize(totalQuads);
        // Allocate all quads
        for (size_t i = 0; i < totalQuads; i++) {
            if (i < level0Quads) {
                _quads[i] = TGD::ArrayContainer(_quadLevel0Description, defaultAllocator());
            } else {
                _quads[i] = TGD::ArrayContainer(
                        { size_t(quadWidth()), size_t(quadHeight()) },
                        _quadLevel0Description.componentCount(),
                        _quadLevel0Description.componentType(),
                        defaultAllocator());
            }
            _quadNeedsRecomputing[i] = true;
        }
    }

    /* Get index of the requested quad */
    int qi = quadIndex(level, qx, qy);

    /* Recompute quads as necessary */
    if (_quadNeedsRecomputing[qi]) {
        // Compute level 0 quads in parallel
        #pragma omp parallel for schedule(dynamic)
        for (int q = 0; q < quadTreeLevelHeight(0) * quadTreeLevelWidth(0); q++) {
            if (_quadNeedsRecomputing[q]) {
                int qy = q / quadTreeLevelWidth(0);
                int qx = q % quadTreeLevelWidth(0);
                computeQuadOnLevel0(_quads[q], qx, qy);
                _quadNeedsRecomputing[q] = false;
            }
        }
        int quadLevelBaseIndex = quadTreeLevelHeight(0) * quadTreeLevelWidth(0);
        // Compute higher level quads, in parallel per level
        for (int l = 1; l < quadTreeLevels(); l++) {
            #pragma omp parallel for schedule(dynamic)
            for (int q = 0; q < quadTreeLevelHeight(l) * quadTreeLevelWidth(l); q++) {
                if (_quadNeedsRecomputing[quadLevelBaseIndex + q]) {
                    int qy = q / quadTreeLevelWidth(l);
                    int qx = q % quadTreeLevelWidth(l);
                    computeQuadOnLevel(_quads[quadLevelBaseIndex + q], l, qx, qy);
                    _quadNeedsRecomputing[quadLevelBaseIndex + q] = false;
                }
            }
            quadLevelBaseIndex += quadTreeLevelHeight(l) * quadTreeLevelWidth(l);
        }
    }

    /* Upload requested quad data to texture */
    auto gl = getGlFunctionsFromCurrentContext();
    ASSERT_GLCHECK();
    if (channelCount() <= 4) {
        // single texture
        uploadArrayToTexture(_quads[qi], tex,
                _texInternalFormat, _texFormat, _texType);
    } else {
        // one texture per channel
        if (_textureTransferArray.dimensionCount() == 0)
            _textureTransferArray = TGD::Array<float>({ size_t(quadWidth()), size_t(quadHeight()) }, 1,
                    TGD::Allocator() /* we want in-memory storage here */);
        const TGD::Array<float> origQuad = _quads[qi];
        for (size_t e = 0; e < _textureTransferArray.elementCount(); e++)
            _textureTransferArray.set<float>(e, 0, origQuad.get<float>(e, channelIndex));
        uploadArrayToTexture(_textureTransferArray, tex,
                _texInternalFormat, _texFormat, _texType);
    }
    // generate mipmap only for the highest level
    if (level == quadTreeLevels() - 1) {
        gl->glBindTexture(GL_TEXTURE_2D, tex);
        if (isOpenGLES()) {
            // mipmap generation does not seem to work reliably!?
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            gl->glGenerateMipmap(GL_TEXTURE_2D);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
    }
    ASSERT_GLCHECK();
}

bool Frame::haveLightness() const
{
    return _lightnessArray.elementCount() > 0;
}

bool Frame::haveStatistic(int channelIndex) const
{
    if (channelIndex == ColorChannelIndex) {
        return _colorStatistic.initialized();
    } else {
        return _statistics[channelIndex].initialized();
    }
}

bool Frame::haveHistogram(int channelIndex) const
{
    if (channelIndex == ColorChannelIndex) {
        return _colorHistogram.initialized();
    } else {
        return _histograms[channelIndex].initialized();
    }
}
