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
    // Initialize quadtree representation. Quads in level 0 are never explicitly
    // stored in order to not duplicate the original data in memory
    TAD::Type quadType = TAD::float32;
    if (channelCount() <= 4) {
        // single texture
        GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
        _texFormat = formats[channelCount() - 1];
        if (colorSpace() == ColorSpaceSLum || colorSpace() == ColorSpaceSRGB) {
            _texInternalFormat = (colorSpace() == ColorSpaceSLum ? GL_SRGB8
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
    std::vector<size_t> quadDims(2, 1024 + 2 * quadBorderSize(0));
    if (width() <= 4096 && height() <= 4096) {
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
    if (x >= 0 && x < width() && y >= 0 && y < height()) {
        if (channelIndex == ColorChannelIndex) {
            int lc;
            const TAD::Array<float>& l = lumArray(lc);
            v = l.get<float>({ size_t(x), size_t(y) }, size_t(lc));
        } else if (type() == TAD::uint8) {
            v = _originalArray.get<uint8_t>({ size_t(x), size_t(y) }, size_t(channelIndex));
        } else {
            v = floatArray().get<float>({ size_t(x), size_t(y) }, size_t(channelIndex));
        }
    } else {
        v = std::numeric_limits<float>::quiet_NaN();
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

    TAD::ArrayContainer q(_quadLevel0Description);
    TAD::ArrayContainer srcArray;
    if (q.componentType() == type())
        srcArray = _originalArray;
    else
        srcArray = floatArray();
    int srcX = qx * quadWidth() - quadBorderSize(0);
    int srcY = qy * quadHeight() - quadBorderSize(0);
    if (srcX >= 0 && srcY >= 0
            && srcX + int(q.dimension(0)) < width()
            && srcY + int(q.dimension(1)) < height()) {
        for (size_t y = 0; y < q.dimension(1); y++) {
            std::memcpy(q.get({ 0, y }),
                    srcArray.get({ size_t(srcX), size_t(srcY + y) }),
                    q.dimension(0) * q.elementSize());
        }
    } else {
        for (size_t y = 0; y < q.dimension(1); y++) {
            int clampedSrcY = srcY + y;
            if (clampedSrcY < 0)
                clampedSrcY = 0;
            else if (clampedSrcY >= height())
                clampedSrcY = height() - 1;
            for (size_t x = 0; x < q.dimension(0); x++) {
                int clampedSrcX = srcX + x;
                if (clampedSrcX < 0)
                    clampedSrcX = 0;
                else if (clampedSrcX >= width())
                    clampedSrcX = width() - 1;
                std::memcpy(q.get({ x, y }),
                        srcArray.get({ size_t(clampedSrcX), size_t(clampedSrcY) }),
                        q.elementSize());
            }
        }
    }
    return q;
}

bool Frame::textureChannelIsS(int texChannel)
{
    return (channelCount() <= 4
            && ((colorSpace() == ColorSpaceSLum && colorChannelIndex(0) == texChannel)
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
