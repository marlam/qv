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

#ifndef QV_FRAME_HPP
#define QV_FRAME_HPP

#include <vector>
#include <memory>

#include <QOpenGLShaderProgram>

#include <tgd/array.hpp>

#include "color.hpp"
#include "statistic.hpp"
#include "histogram.hpp"


class Frame {
private:
    /* data: */
    TGD::ArrayContainer _originalArray;
    TGD::Array<float> _lightnessArray; // perceptually linear lightness from CIELUV
    /* per channel: */
    std::vector<float> _minVals, _maxVals;
    std::vector<Statistic> _statistics;
    std::vector<Histogram> _histograms;
    /* color: */
    ColorSpace _colorSpace;
    int _colorChannels[3], _alphaChannel;
    float _colorMinVal, _colorMaxVal;
    float _colorVisMinVal, _colorVisMaxVal;
    Statistic _colorStatistic;
    Histogram _colorHistogram;
    /* current channel: */
    int _channelIndex;
    /* quadtree: */
    int _quadLevel0BorderSize;
    TGD::ArrayDescription _quadLevel0Description;
    std::vector<int> _quadTreeWidths;
    std::vector<int> _quadTreeHeights;
    std::vector<TGD::ArrayContainer> _quads;
    /* textures: */
    unsigned int _texInternalFormat;
    unsigned int _texFormat;
    unsigned int _texType;
    TGD::Array<float> _textureTransferArray;

    const TGD::Array<float>& lightnessArray();
    int quadIndex(int level, int qx, int qy) const; // returns -1 if nonexistent
    TGD::ArrayContainer quadFromLevel0(int qx, int qy) const;
    TGD::ArrayContainer quadFromLevel(int l, int qx, int qy) const;
    bool textureChannelIsS(int index) const;

public:
    // OpenGL is required to support at least the following as GL_MAX_TEXTURE_SIZE:
    static constexpr int requiredMaxTextureSize = 8192;

    Frame();

    void init(const TGD::ArrayContainer& a);
    void reset();

    const TGD::ArrayContainer& array() const { return _originalArray; }
    TGD::Type type() const { return _originalArray.componentType(); }
    int channelCount() const { return _originalArray.componentCount(); }
    int width() const { return _originalArray.elementCount() > 0 ? _originalArray.dimension(0) : 0; }
    int height() const { return _originalArray.elementCount() > 0 ? _originalArray.dimension(1) : 0; }
    float value(int x, int y, int channelIndex);

    ColorSpace colorSpace() const { return _colorSpace; }
    bool hasAlpha() const { return _alphaChannel >= 0; }
    int colorChannelIndex(int colorSpaceIndex = 0) const { return _colorChannels[colorSpaceIndex]; }
    int alphaChannelIndex() const { return _alphaChannel; }

    std::string channelName(int channelIndex) const;
    std::string currentChannelName() const { return channelName(channelIndex()); }

    float minVal(int channelIndex);
    float currentMinVal() { return minVal(channelIndex()); }
    float maxVal(int channelIndex);
    float currentMaxVal() { return maxVal(channelIndex()); }

    float visMinVal(int channelIndex);
    float currentVisMinVal() { return visMinVal(channelIndex()); }
    float visMaxVal(int channelIndex);
    float currentVisMaxVal() { return visMaxVal(channelIndex()); }

    const Statistic& statistic(int channelIndex);
    const Statistic& currentStatistic() { return statistic(channelIndex()); }
    const Histogram& histogram(int channelIndex);
    const Histogram& currentHistogram() { return histogram(channelIndex()); }

    void setChannelIndex(int index);
    int channelIndex() const { return _channelIndex; }

    // Quadtree representation for rendering
    int quadBorderSize(int level) const { return (level == 0 ? _quadLevel0BorderSize : 0); }
    int quadWidth() const { return _quadLevel0Description.dimension(0) - 2 * quadBorderSize(0); }
    int quadHeight() const { return _quadLevel0Description.dimension(1) - 2 * quadBorderSize(0); }
    int quadTreeLevels() const { return _quadTreeWidths.size(); }
    int quadTreeLevelWidth(int level) const { return _quadTreeWidths[level]; }
    int quadTreeLevelHeight(int level) const { return _quadTreeHeights[level]; }
    void uploadQuadToTexture(unsigned int tex, int level, int qx, int qy, int channelIndex);

    // Query whether some information is already computed or not yet
    bool haveLightness() const;
    bool haveStatistic(int channelIndex) const;
    bool haveHistogram(int channelIndex) const;
};

#endif
