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
    assert(_channelIndex == -1);

    // Convert to float, extract channel arrays
    _type = a.componentType();
    TAD::Array<float> array = a.convert(TAD::float32);
    _arrays.resize(array.componentCount());
    _linearArrays.resize(array.componentCount());
    _minVals.resize(array.componentCount());
    _maxVals.resize(array.componentCount());
    _statistics.resize(array.componentCount());
    _histograms.resize(array.componentCount());
    for (size_t i = 0; i < _arrays.size(); i++) {
        _arrays[i] = TAD::Array<float>({ array.dimension(0), array.dimension(1) }, 1);
        for (size_t e = 0; e < array.elementCount(); e++)
            std::memcpy(_arrays[i].get(e), array.get<float>(e) + i, sizeof(float));
        _minVals[i] = std::numeric_limits<float>::quiet_NaN();
        _maxVals[i] = std::numeric_limits<float>::quiet_NaN();
        if (_type == TAD::uint8) {
            _minVals[i] = 0.0f;
            _maxVals[i] = 255.0f;
        }
        a.componentTagList(i).value("MINVAL", &(_minVals[i]));
        a.componentTagList(i).value("MAXVAL", &(_maxVals[i]));
        if (!std::isfinite(_minVals[i]) || !std::isfinite(_maxVals[i])) {
            _minVals[i] = statistic(i).minVal();
            _maxVals[i] = statistic(i).maxVal();
        }
    }
    // Determine color space, if any
    _colorSpace = ColorSpaceNone;
    _colorChannels[0] = -1;
    _colorChannels[1] = -1;
    _colorChannels[2] = -1;
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "GRAY");
        if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceLinearGray;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "RED");
        _colorChannels[1] = componentIndex(array, "GREEN");
        _colorChannels[2] = componentIndex(array, "BLUE");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0)
            _colorSpace = ColorSpaceLinearRGB;
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "SRGB/LUM");
        if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceSLum;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "SRGB/R");
        _colorChannels[1] = componentIndex(array, "SRGB/G");
        _colorChannels[2] = componentIndex(array, "SRGB/B");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0)
            _colorSpace = ColorSpaceSRGB;
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "SRGB/RED");
        _colorChannels[1] = componentIndex(array, "SRGB/GREEN");
        _colorChannels[2] = componentIndex(array, "SRGB/BLUE");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0)
            _colorSpace = ColorSpaceSRGB;
    }
    if (_colorSpace == ColorSpaceNone) {
        _colorChannels[0] = componentIndex(array, "XYZ/X");
        _colorChannels[1] = componentIndex(array, "XYZ/Y");
        _colorChannels[2] = componentIndex(array, "XYZ/Z");
        if (_colorChannels[0] >= 0 && _colorChannels[1] >= 0 && _colorChannels[2] >= 0) {
            _colorSpace = ColorSpaceXYZ;
        } else if (_colorChannels[0] >= 0) {
            _colorSpace = ColorSpaceY;
            _colorChannels[1] = _colorChannels[0];
            _colorChannels[2] = _colorChannels[0];
        }
    }
    if (_colorSpace != ColorSpaceNone) {
        _alphaChannel = componentIndex(array, "ALPHA");
    }
    _linearColorSpace = _colorSpace;
    if (_colorSpace == ColorSpaceLinearGray || _colorSpace == ColorSpaceY) {
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
        _linearColorSpace = ColorSpaceLinearGray;
    } else if (_colorSpace == ColorSpaceSRGB) {
        _colorMinVal = 0.0f;
        _colorMaxVal = 255.0f;
        _colorVisMinVal = 0.0f;
        _colorVisMaxVal = 100.0f;
        _linearColorSpace = ColorSpaceLinearRGB;
    }
    // Set initial channel
    _channelIndex = (_colorSpace != ColorSpaceNone ? ColorChannelIndex : 0);
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

const TAD::Array<float>& Frame::linearArray(int channel)
{
    if (_linearArrays[channel].elementCount() == 0) {
        if ((colorSpace() == ColorSpaceSLum && channel == colorChannelIndex())
                || (colorSpace() == ColorSpaceSRGB
                    && (channel == colorChannelIndex(0) || channel == colorChannelIndex(1) || channel == colorChannelIndex(2)))) {
            _linearArrays[channel] = TAD::Array<float>({ size_t(width()), size_t(height()) }, 1);
            for (size_t e = 0; e < _linearArrays[channel].elementCount(); e++) {
                float sv = _arrays[channel].get<float>(e, 0) / 255.0f;
                float v = snormToLinear(sv);
                _linearArrays[channel].set<float>(e, 0, v * 255.0f);
            }
        } else {
            _linearArrays[channel] = _arrays[channel];
        }
    }
    return _linearArrays[channel];
}

const TAD::Array<float>& Frame::lumArray()
{
    if (_lumArray.elementCount() == 0) {
        if (colorSpace() == ColorSpaceLinearGray || colorSpace() == ColorSpaceY) {
            _lumArray = _arrays[colorChannelIndex(0)];
        } else if (colorSpace() == ColorSpaceXYZ) {
            _lumArray = _arrays[colorChannelIndex(1)];
        } else if (colorSpace() == ColorSpaceLinearRGB) {
            _lumArray = TAD::Array<float>({ size_t(width()), size_t(height()) }, 1);
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                _lumArray.set<float>(e, 0, rgbToY(
                            _arrays[colorChannelIndex(0)].get<float>(e, 0),
                            _arrays[colorChannelIndex(1)].get<float>(e, 0),
                            _arrays[colorChannelIndex(2)].get<float>(e, 0)));
            }
        } else if (colorSpace() == ColorSpaceSLum) {
            _lumArray = TAD::Array<float>({ size_t(width()), size_t(height()) }, 1);
            const TAD::Array<float>& linArray = linearArray(colorChannelIndex(0));
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                _lumArray.set<float>(e, 0, linArray.get<float>(e, 0) / 255.0f * 100.0f);
            }
        } else if (colorSpace() == ColorSpaceSRGB) {
            _lumArray = TAD::Array<float>({ size_t(width()), size_t(height()) }, 1);
            const TAD::Array<float>& linArray0 = linearArray(colorChannelIndex(0));
            const TAD::Array<float>& linArray1 = linearArray(colorChannelIndex(1));
            const TAD::Array<float>& linArray2 = linearArray(colorChannelIndex(2));
            for (size_t e = 0; e < _lumArray.elementCount(); e++) {
                _lumArray.set<float>(e, 0, rgbToY(
                            linArray0.get<float>(e, 0) / 255.0f,
                            linArray1.get<float>(e, 0) / 255.0f,
                            linArray2.get<float>(e, 0) / 255.0f));
            }
        }
    }
    return _lumArray;
}

const TAD::Array<float>& Frame::data(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return lumArray();
    } else {
        return _arrays[channelIndex];
    }
}

float Frame::minVal(int channelIndex) const
{
    return channelIndex == ColorChannelIndex ? _colorMinVal : _minVals[channelIndex];
}

float Frame::maxVal(int channelIndex) const
{
    return channelIndex == ColorChannelIndex ? _colorMaxVal : _maxVals[channelIndex];
}

float Frame::visMinVal(int channelIndex) const
{
    return channelIndex == ColorChannelIndex ? _colorVisMinVal : minVal(channelIndex);
}

float Frame::visMaxVal(int channelIndex) const
{
    return channelIndex == ColorChannelIndex ? _colorVisMaxVal : maxVal(channelIndex);
}

const Statistic& Frame::statistic(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (_colorStatistic.finiteValues() < 0)
            _colorStatistic.init(lumArray());
        return _colorStatistic;
    } else {
        if (_statistics[channelIndex].finiteValues() < 0)
            _statistics[channelIndex].init(_arrays[channelIndex]);
        return _statistics[channelIndex];
    }
}

const Histogram& Frame::histogram(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        if (_colorHistogram.binCount() == 0) {
            _colorHistogram.init(lumArray(), _type, _colorVisMinVal, _colorVisMaxVal);
        }
        return _colorHistogram;
    } else {
        if (_histograms[channelIndex].binCount() == 0) {
            _histograms[channelIndex].init(_arrays[channelIndex], _type,
                    minVal(channelIndex), maxVal(channelIndex));
        }
        return _histograms[channelIndex];
    }
}

unsigned int Frame::texture(int channelIndex)
{
    auto gl = getGlFunctionsFromCurrentContext();
    if (_textures.size() != size_t(channelCount()))
        _textures.resize(channelCount(), 0);
    if (_textures[channelIndex] == 0) {
        gl->glGenTextures(1, &(_textures[channelIndex]));
        gl->glBindTexture(GL_TEXTURE_2D, _textures[channelIndex]);
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glGetGlobalPBO());
        gl->glBufferData(GL_PIXEL_UNPACK_BUFFER, linearArray(channelIndex).dataSize(), nullptr, GL_STREAM_DRAW);
        void* ptr = gl->glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, linearArray(channelIndex).dataSize(),
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        std::memcpy(ptr, linearArray(channelIndex).data(), linearArray(channelIndex).dataSize());
        gl->glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width(), height(), 0, GL_RED, GL_FLOAT, nullptr);
        gl->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        gl->glGenerateMipmap(GL_TEXTURE_2D);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    return _textures[channelIndex];
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
