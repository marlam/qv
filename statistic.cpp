/*
 * Copyright (C) 2019, 2020, 2021, 2022
 * Computer Graphics Group, University of Siegen
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

#include <omp.h>

#include "statistic.hpp"


Statistic::Statistic() :
    _initialized(false),
    _finiteValues(0),
    _minVal(std::numeric_limits<float>::quiet_NaN()),
    _maxVal(std::numeric_limits<float>::quiet_NaN()),
    _sampleMean(std::numeric_limits<float>::quiet_NaN()),
    _sampleVariance(std::numeric_limits<float>::quiet_NaN()),
    _sampleDeviation(std::numeric_limits<float>::quiet_NaN())
{
}

template<typename T>
static void initHelper(const TAD::Array<T> array, size_t componentIndex,
        unsigned long long& _finiteValues, float& _minVal, float& _maxVal,
        float& _sampleMean, float& _sampleVariance, float& _sampleDeviation)
{
    size_t n = array.elementCount();
    size_t cc = array.componentCount();
    const T* data = array[0];

    int maxParts = omp_get_max_threads();
    std::vector<unsigned long long> partFiniteValues(maxParts, 0);
    std::vector<float> partMinVals(maxParts, std::numeric_limits<float>::quiet_NaN());
    std::vector<float> partMaxVals(maxParts, std::numeric_limits<float>::quiet_NaN());
    std::vector<double> partSums(maxParts, 0.0);
    std::vector<double> partSumsOfSquares(maxParts, 0.0);
    int parts;
    #pragma omp parallel
    {
        parts = omp_get_num_threads();
        size_t partSize = n / parts + (n % parts == 0 ? 0 : 1);
        int p = omp_get_thread_num();

        for (size_t pe = 0; pe < partSize; pe++) {
            size_t e = p * partSize + pe;
            if (e >= n)
                break;
            T val = data[e * cc + componentIndex];
            if (std::isfinite(val)) {
                partFiniteValues[p]++;
                if (partFiniteValues[p] == 1) {
                    partMinVals[p] = val;
                    partMaxVals[p] = val;
                } else if (val < partMinVals[p]) {
                    partMinVals[p] = val;
                } else if (val > partMaxVals[p]) {
                    partMaxVals[p] = val;
                }
                partSums[p] += val;
                partSumsOfSquares[p] += val * val;
            }
        }
    }

    double sum = 0.0;
    double sumOfSquares = 0.0;
    for (int p = 0; p < parts; p++) {
        _finiteValues += partFiniteValues[p];
        sum += partSums[p];
        sumOfSquares += partSumsOfSquares[p];
        if (std::isfinite(partMinVals[p]) && (!std::isfinite(_minVal) || _minVal > partMinVals[p]))
            _minVal = partMinVals[p];
        if (std::isfinite(partMaxVals[p]) && (!std::isfinite(_maxVal) || _maxVal < partMaxVals[p]))
            _maxVal = partMaxVals[p];
    }
    if (_finiteValues > 0) {
        _sampleMean = sum / _finiteValues;
        if (_finiteValues > 1) {
            _sampleVariance = (sumOfSquares - sum / _finiteValues * sum) / (_finiteValues - 1);
            if (_sampleVariance < 0.0f)
                _sampleVariance = 0.0f;
            _sampleDeviation = std::sqrt(_sampleVariance);
        } else if (_finiteValues == 1) {
            _sampleVariance = 0.0f;
            _sampleDeviation = 0.0f;
        }
    }
}

void Statistic::init(const TAD::ArrayContainer& array, size_t componentIndex)
{
    assert(!_initialized);
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
    _initialized = true;
}
