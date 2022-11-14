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

#ifndef QV_PARAMETERS_HPP
#define QV_PARAMETERS_HPP

#include <vector>

#include "colormap.hpp"


class Parameters {
private:
    std::vector<float> _visMinVals;
    std::vector<float> _visMaxVals;
    float _colorVisMinVal;
    float _colorVisMaxVal;
    ColorMap _colorMap;

public:
    /* Constructor */
    Parameters();

    /* Trivial parameters */
    bool magInterpolation;
    bool magGrid;
    float zoom;
    float xOffset, yOffset;
    bool dynamicRangeReduction;
    float drrBrightness;

    /* Managed parameters */
    float visMinVal(int channelIndex);
    void setVisMinVal(int channelIndex, float v);
    float visMaxVal(int channelIndex);
    void setVisMaxVal(int channelIndex, float v);
    ColorMap& colorMap() { return _colorMap; }
};

#endif
