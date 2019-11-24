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

#include "parameters.hpp"
#include "color.hpp"


Parameters::Parameters() :
    _colorVisMinVal(std::numeric_limits<float>::quiet_NaN()),
    _colorVisMaxVal(std::numeric_limits<float>::quiet_NaN()),
    magInterpolation(true), magGrid(false),
    zoom(1.0f), xOffset(0), yOffset(0),
    dynamicRangeReduction(false), drrBrightness(64.0f)
{
}

float Parameters::visMinVal(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return _colorVisMinVal;
    } else {
        if (_visMinVals.size() <= size_t(channelIndex))
            _visMinVals.resize(channelIndex + 1, std::numeric_limits<float>::quiet_NaN());
        return _visMinVals[channelIndex];
    }
}

void Parameters::setVisMinVal(int channelIndex, float v)
{
    if (channelIndex == ColorChannelIndex) {
        _colorVisMinVal = v;
    } else {
        if (_visMinVals.size() <= size_t(channelIndex))
            _visMinVals.resize(channelIndex + 1, std::numeric_limits<float>::quiet_NaN());
        _visMinVals[channelIndex] = v;
    }
}

float Parameters::visMaxVal(int channelIndex)
{
    if (channelIndex == ColorChannelIndex) {
        return _colorVisMaxVal;
    } else {
        if (_visMaxVals.size() <= size_t(channelIndex))
            _visMaxVals.resize(channelIndex + 1, std::numeric_limits<float>::quiet_NaN());
        return _visMaxVals[channelIndex];
    }
}

void Parameters::setVisMaxVal(int channelIndex, float v)
{
    if (channelIndex == ColorChannelIndex) {
        _colorVisMaxVal = v;
    } else {
        if (_visMaxVals.size() <= size_t(channelIndex))
            _visMaxVals.resize(channelIndex + 1, std::numeric_limits<float>::quiet_NaN());
        _visMaxVals[channelIndex] = v;
    }
}
