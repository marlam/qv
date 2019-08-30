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

#ifndef QV_FRAME_HPP
#define QV_FRAME_HPP

#include <vector>

#include <tad/array.hpp>

#include "color.hpp"
#include "statistic.hpp"
#include "histogram.hpp"


class Frame {
private:
    /* data: */
    TAD::ArrayContainer _originalArray;
    TAD::Array<float> _floatArray;
    TAD::Array<float> _lumArray;
    int _lumArrayChannel;
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
    /* textures: */
    std::vector<unsigned int> _textures; // one per channel
    /* current channel: */
    int _channelIndex;

    const TAD::Array<float>& floatArray();
    const TAD::Array<float>& lumArray(int& lumArrayChannel);

    void clearTextures();

public:
    Frame();
    ~Frame();

    void init(const TAD::ArrayContainer& a);
    void reset();

    TAD::Type type() const { return _originalArray.componentType(); }
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

    unsigned int texture(int channelIndex);

    void setChannelIndex(int index);
    int channelIndex() const { return _channelIndex; }
};

#endif
