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
    TAD::Type _type;
    /* per channel: */
    std::vector<TAD::Array<float>> _arrays;
    std::vector<TAD::Array<float>> _linearArrays;
    std::vector<float> _minVals, _maxVals;
    std::vector<Statistic> _statistics;
    std::vector<Histogram> _histograms;
    /* color: */
    ColorSpace _colorSpace, _linearColorSpace;
    int _colorChannels[3];
    float _colorMinVal, _colorMaxVal;
    float _colorVisMinVal, _colorVisMaxVal;
    TAD::Array<float> _lumArray;
    Statistic _colorStatistic;
    Histogram _colorHistogram;
    /* textures: */
    std::vector<unsigned int> _textures; // one per channel
    /* current channel: */
    int _channelIndex;

    const TAD::Array<float>& linearArray(int channel);
    const TAD::Array<float>& lumArray();

public:
    Frame();
    ~Frame();

    void init(const TAD::ArrayContainer& a);

    TAD::Type type() const { return _type; }
    int channelCount() const { return _arrays.size(); }
    int width() const { return channelCount() > 0 ? _arrays[0].dimension(0) : 0; } 
    int height() const { return channelCount() > 0 ? _arrays[0].dimension(1) : 0; }
    const TAD::Array<float>& data(int channelIndex);

    ColorSpace colorSpace() const { return _colorSpace; }
    int colorChannelIndex(int colorSpaceIndex = 0) { return _colorChannels[colorSpaceIndex]; }

    float minVal(int channelIndex) const;
    float currentMinVal() const { return minVal(channelIndex()); }
    float maxVal(int channelIndex) const;
    float currentMaxVal() const { return maxVal(channelIndex()); }

    float visMinVal(int channelIndex) const;
    float currentVisMinVal() const { return visMinVal(channelIndex()); }
    float visMaxVal(int channelIndex) const;
    float currentVisMaxVal() const { return visMaxVal(channelIndex()); }

    const Statistic& statistic(int channelIndex);
    const Statistic& currentStatistic() { return statistic(channelIndex()); }
    const Histogram& histogram(int channelIndex);
    const Histogram& currentHistogram() { return histogram(channelIndex()); }

    unsigned int texture(int channelIndex);
    ColorSpace textureColorSpace() const { return _linearColorSpace; }
    void clearTextures();

    void setChannelIndex(int index);
    int channelIndex() const { return _channelIndex; }
};

#endif
