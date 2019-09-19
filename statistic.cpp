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
#include <type_traits>
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

template<typename T>
static void initHelper(const TAD::Array<T> array, size_t componentIndex,
        long long& _finiteValues, float& _minVal, float& _maxVal,
        float& _sampleMean, float& _sampleVariance, float& _sampleDeviation)
{
    float minVal = +std::numeric_limits<float>::max();
    float maxVal = -std::numeric_limits<float>::max();
    double sum = 0.0;
    double sumOfSquares = 0.0;
    _finiteValues = 0;
    for (size_t e = 0; e < array.elementCount(); e++) {
        T val = array[e][componentIndex];
        if (std::is_floating_point<T>::value && !std::isfinite(val))
            continue;
        _finiteValues++;
        if (val < minVal)
            minVal = val;
        else if (val > maxVal)
            maxVal = val;
        sum += val;
        sumOfSquares += val * val;
    }
    if (_finiteValues > 0) {
        _minVal = minVal;
        _maxVal = maxVal;
        _sampleMean = sum / _finiteValues;
        if (_finiteValues > 1) {
            _sampleVariance = (sumOfSquares - sum / array.elementCount() * sum) / (_finiteValues - 1);
            if (_sampleVariance < 0.0f)
                _sampleVariance = 0.0f;
            _sampleDeviation = std::sqrt(_sampleVariance);
        }
    }
}

void Statistic::init(const TAD::ArrayContainer& array, size_t componentIndex)
{
    assert(_finiteValues == -1);
    switch (array.componentType()) {
    case TAD::int8:
        initHelper(TAD::Array<int8_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::uint8:
        initHelper(TAD::Array<uint8_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::int16:
        initHelper(TAD::Array<int16_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::uint16:
        initHelper(TAD::Array<uint16_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::int32:
        initHelper(TAD::Array<int32_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::uint32:
        initHelper(TAD::Array<uint32_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::int64:
        initHelper(TAD::Array<int64_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::uint64:
        initHelper(TAD::Array<uint64_t>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::float32:
        initHelper(TAD::Array<float>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    case TAD::float64:
        initHelper(TAD::Array<double>(array), componentIndex, _finiteValues, _minVal, _maxVal, _sampleMean, _sampleVariance, _sampleDeviation);
        break;
    }
}
