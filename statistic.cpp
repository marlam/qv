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

#include <cassert>
#include <limits>
#include <cmath>

#include "statistic.hpp"


Statistic::Statistic() :
    _finiteValues(-1),
    _minVal(std::numeric_limits<float>::quiet_NaN()),
    _maxVal(std::numeric_limits<float>::quiet_NaN()),
    _sampleMean(std::numeric_limits<float>::quiet_NaN()),
    _sampleVariance(std::numeric_limits<float>::quiet_NaN()),
    _sampleDeviation(std::numeric_limits<float>::quiet_NaN())
{
}

void Statistic::init(const TAD::Array<float>& array)
{
    assert(_finiteValues == -1);

    float minVal = +std::numeric_limits<float>::max();
    float maxVal = -std::numeric_limits<float>::max();
    double sum = 0.0;
    double sumOfSquares = 0.0;
    _finiteValues = 0;

    for (size_t e = 0; e < array.elementCount(); e++) {
        float val = array.get<float>(e, 0);
        if (std::isfinite(val)) {
            _finiteValues++;
            if (val < minVal)
                minVal = val;
            else if (val > maxVal)
                maxVal = val;
            sum += val;
            sumOfSquares += val * val;
        }
    }
    if (_finiteValues > 0) {
        _minVal = minVal;
        _maxVal = maxVal;
        _sampleMean = sum / _finiteValues;
        if (_finiteValues > 1) {
            _sampleVariance = (sumOfSquares - sum / array.elementCount() * sum) / (_finiteValues - 1);
            _sampleDeviation = std::sqrt(_sampleVariance);
        }
    }
}
