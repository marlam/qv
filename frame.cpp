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

#include <limits>
#include <cmath>

#include "frame.hpp"
#include "gl.hpp"


Frame::Frame() :
    _colorSpace(ColorSpaceNone), _colorChannels { -1, -1, -1 }, _alphaChannel(-1),
    _channelIndex(-1)
{
}

Frame::~Frame()
{
    clearTextures();
}

Frame::Frame(const Frame& f)
{
    *this = f;
}

Frame& Frame::operator=(const Frame& f)
{
    _originalArray = f._originalArray;
    _floatArray = f._floatArray;
    _lumArray = f._lumArray;
    _lumArrayChannel = f._lumArrayChannel;
    _minVals = f._minVals;
    _maxVals = f._maxVals;
    _statistics = f._statistics;
    _histograms = f._histograms;
    _colorSpace = f._colorSpace;
    _colorChannels[0] = f._colorChannels[0];
    _colorChannels[1] = f._colorChannels[1];
    _colorChannels[2] = f._colorChannels[2];
    _alphaChannel = f._alphaChannel;
    _colorMinVal = f._colorMinVal;
    _colorMaxVal = f._colorMaxVal;
    _colorVisMinVal = f._colorVisMinVal;
    _colorVisMaxVal = f._colorVisMaxVal;
    _colorStatistic = f._colorStatistic;
    _colorHistogram = f._colorHistogram;
    _channelIndex = f._channelIndex;
    _textures.clear(); // We need to regenerate them; the destructor of the old frame will delete them
    return *this;
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
        _colorChannels[0] = componentIndex(_originalArray, "RED");
        _colorChannels[1] = componentIndex(_originalArray, "GREEN");
        _colorChannels[2] = componentIndex(_originalArray, "BLUE");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0) {
            _colorSpace = ColorSpaceLinearRGB;
        }
    }
    if (_colorSpace == ColorSpaceNone
            && type() == TAD::uint8 && (channelCount() == 1 || channelCount() == 2)) {
        _colorChannels[0] = componentIndex(_originalArray, "SRGB/LUM");
        if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceSLum;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace == ColorSpaceNone
            && type() == TAD::uint8 && (channelCount() == 3 || channelCount() == 4)) {
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
        } else {
            _colorChannels[0] = componentIndex(_originalArray, "GRAY");
            if (_colorChannels[0] >= 0) {
                _colorSpace = ColorSpaceY;
                _colorChannels[1] = _colorChannels[0];
                _colorChannels[2] = _colorChannels[0];
            }
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _alphaChannel = -1;
    }
    if (_colorSpace == ColorSpaceY) {
        _colorMinVal = minVal(colorChannelIndex(0));
        _colorMaxVal = maxVal(colorChannelIndex(0));
        _colorVisMinVal = _colorMinVal;
        _colorVisMaxVal = _colorMaxVal;
    } else if (_colorSpace == ColorSpaceXYZ) {
        _colorMinVal = minVal(colorChannelIndex(1));
        _colorMaxVal = maxVal(colorChannelIndex(1));
        _colorVisMinVal = _colorMinVal;
        _colorVisMaxVal = _colorMaxVal;
    } else if (_colorSpace == ColorSpaceLinearRGB) {
        _colorMinVal = std::min(std::min(minVal(colorChannelIndex(0)), minVal(colorChannelIndex(1))), minVal(colorChannelIndex(2)));
        _colorMaxVal = std::max(std::max(maxVal(colorChannelIndex(0)), maxVal(colorChannelIndex(1))), maxVal(colorChannelIndex(2)));
        _colorVisMinVal = statistic(ColorChannelIndex).minVal();
        _colorVisMaxVal = statistic(ColorChannelIndex).maxVal();
    } else if (_colorSpace == ColorSpaceSLum) {
        _colorMinVal = 0.0f;
        _colorMaxVal = 255.0f;
        _colorVisMinVal = 0.0f;
        _colorVisMaxVal = 100.0f;
    } else if (_colorSpace == ColorSpaceSRGB) {
        _colorMinVal = 0.0f;
        _colorMaxVal = 255.0f;
        _colorVisMinVal = 0.0f;
        _colorVisMaxVal = 100.0f;
    }
    // Set initial channel
    _channelIndex = (_colorSpace != ColorSpaceNone ? ColorChannelIndex : 0);
}

void Frame::reset()
{
    clearTextures();
    *this = Frame();
}

const TAD::Array<float>& Frame::floatArray()
{
    if (_floatArray.elementCount() == 0)
        _floatArray = _originalArray.convert(TAD::float32);
    return _floatArray;
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
        case ColorSpaceLinearRGB:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(R)";
            else if (channelIndex == colorChannelIndex(1))
                channelName += "(G)";
            else if (channelIndex == colorChannelIndex(2))
                channelName += "(B)";
            break;
        case ColorSpaceSLum:
            if (channelIndex == colorChannelIndex(0))
                channelName += "(sLum)";
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

const TAD::Array<float>& Frame::lumArray(int& lumArrayChannel)
{
    if (_lumArray.elementCount() == 0) {
        if (colorSpace() == ColorSpaceY) {
            _lumArray = floatArray();
            _lumArrayChannel = colorChannelIndex(0);
        } else if (colorSpace() == ColorSpaceXYZ) {
            _lumArray = floatArray();
            _lumArrayChannel = colorChannelIndex(1);
        } else if (colorSpace() == ColorSpaceLinearRGB) {
            _lumArray = TAD::Array<float>(_originalArray.dimensions(), 1);
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                const float* elem = floatArray().get<float>(e);
                float r = elem[colorChannelIndex(0)];
                float g = elem[colorChannelIndex(1)];
                float b = elem[colorChannelIndex(1)];
                float y = rgbToY(r, g, b);
                _lumArray.set<float>(e, 0, y);
            }
            _lumArrayChannel = 0;
        } else if (colorSpace() == ColorSpaceSLum) {
            _lumArray = TAD::Array<float>(_originalArray.dimensions(), 1);
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                uint8_t lum = _originalArray.get<uint8_t>(e, colorChannelIndex(0));
                float y = toLinear(lum / 255.0f) * 100.0f;
                _lumArray.set<float>(e, 0, y);
            }
            _lumArrayChannel = 0;
        } else if (colorSpace() == ColorSpaceSRGB) {
            _lumArray = TAD::Array<float>(_originalArray.dimensions(), 1);
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                const uint8_t* elem = _originalArray.get<uint8_t>(e);
                float r = toLinear(elem[colorChannelIndex(0)] / 255.0f);
                float g = toLinear(elem[colorChannelIndex(1)] / 255.0f);
                float b = toLinear(elem[colorChannelIndex(1)] / 255.0f);
                float y = rgbToY(r, g, b);
                _lumArray.set<float>(e, 0, y);
            }
            _lumArrayChannel = 0;
        }
    }
    lumArrayChannel = _lumArrayChannel;
    return _lumArray;
}

float Frame::value(int x, int y, int channelIndex)
{
    float v;
    if (channelIndex == ColorChannelIndex) {
        int lc;
        const TAD::Array<float>& l = lumArray(lc);
        v = l.get<float>({ size_t(x), size_t(y) }, size_t(lc));
    } else if (type() == TAD::uint8) {
        v = _originalArray.get<uint8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
    } else {
        v = floatArray().get<float>({ size_t(x), size_t(y) }, size_t(channelIndex));
    }
    return v;
}

float Frame::minVal(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return _colorMinVal;
    } else {
        if (!std::isfinite(_minVals[channelIndex])) {
            if (type() == TAD::uint8)
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
            if (type() == TAD::uint8)
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
            int lc;
            const TAD::Array<float>& l = lumArray(lc);
            _colorStatistic.init(l, lc);
        }
        return _colorStatistic;
    } else {
        if (_statistics[channelIndex].finiteValues() < 0) {
            if (type() == TAD::uint8) {
                _statistics[channelIndex].init(TAD::Array<uint8_t>(_originalArray), channelIndex);
            } else {
                _statistics[channelIndex].init(floatArray(), channelIndex);
            }
        }
        return _statistics[channelIndex];
    }
}

const Histogram& Frame::histogram(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (_colorHistogram.binCount() == 0) {
            int lc;
            const TAD::Array<float>& l = lumArray(lc);
            _colorHistogram.init(l, lc, visMinVal(ColorChannelIndex), visMaxVal(ColorChannelIndex));
        }
        return _colorHistogram;
    } else {
        if (_histograms[channelIndex].binCount() == 0) {
            if (type() == TAD::uint8) {
                _histograms[channelIndex].init(TAD::Array<uint8_t>(_originalArray), channelIndex);
            } else {
                _histograms[channelIndex].init(floatArray(), channelIndex,
                        minVal(channelIndex), maxVal(channelIndex));
            }
        }
        return _histograms[channelIndex];
    }
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
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glGetGlobalPBO());
    gl->glBufferData(GL_PIXEL_UNPACK_BUFFER, array.dataSize(), nullptr, GL_STREAM_DRAW);
    void* ptr = gl->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, array.dataSize(),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    std::memcpy(ptr, array.data(), array.dataSize());
    gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
            array.dimension(0), array.dimension(1), 0,
            format, type, nullptr);
    gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    if (isOpenGLES()) {
        // mipmap generation does not seem to work reliably!?
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        gl->glGenerateMipmap(GL_TEXTURE_2D);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    ASSERT_GLCHECK();
}

unsigned int Frame::texture(int channelIndex)
{
    auto gl = getGlFunctionsFromCurrentContext();
    unsigned int tex = 0;
    if (channelCount() <= 4) {
        // single texture
        if (_textures.size() != 1) {
            _textures.resize(1);
            gl->glGenTextures(1, _textures.data());
            GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
            GLenum format = formats[channelCount() - 1];
            if (colorSpace() == ColorSpaceSLum || colorSpace() == ColorSpaceSRGB) {
                GLint internalFormat = (colorSpace() == ColorSpaceSLum ? GL_SRGB8
                        : colorSpace() == ColorSpaceSRGB && !hasAlpha() ? GL_SRGB8
                        : GL_SRGB8_ALPHA8);
                GLenum type = GL_UNSIGNED_BYTE;
                uploadArrayToTexture(_originalArray, _textures[0], internalFormat, format, type);
            } else {
                GLint internalFormats[4] = { GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
                GLint internalFormat = internalFormats[channelCount() - 1];
                GLenum type = GL_FLOAT;
                uploadArrayToTexture(floatArray(), _textures[0], internalFormat, format, type);
            }
        }
        tex = _textures[0];
    } else {
        // one texture per channel
        if (_textures.size() != size_t(channelCount()))
            _textures.resize(channelCount(), 0);
        if (_textures[channelIndex] == 0) {
            gl->glGenTextures(1, &(_textures[channelIndex]));
            TAD::Array<float> dataArray(_originalArray.dimensions(), 1);
            for (size_t e = 0; e < dataArray.elementCount(); e++)
                dataArray.set<float>(e, 0, floatArray().get<float>(e, channelIndex));
            uploadArrayToTexture(dataArray, _textures[channelIndex], GL_R32F, GL_RED, GL_FLOAT);
        }
        tex = _textures[channelIndex];
    }
    return tex;
}

void Frame::clearTextures()
{
    if (_textures.size() > 0) {
        auto gl = getGlFunctionsFromCurrentContext();
        if (gl) // at program termination, the GL context might already be gone
            gl->glDeleteTextures(_textures.size(), _textures.data());
        _textures.clear();
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
