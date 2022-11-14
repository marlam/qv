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
#include <cmath>

#include <omp.h>

#include "histogram.hpp"


Histogram::Histogram() : _initialized(false)
{
}

static int binIndexHelper(float v, float minVal, float maxVal, int bins)
{
    int b = (v - minVal) / (maxVal - minVal) * bins;
    if (b < 0)
        b = 0;
    else if (b >= bins)
        b = bins - 1;
    return b;
}

int Histogram::binIndex(float v) const
{
    return binIndexHelper(v, _minVal, _maxVal, _bins.size());
}

template<typename T>
static void initHelper(const TGD::Array<T> array, size_t componentIndex,
        float _minVal, float _maxVal, size_t binCount,
        std::vector<unsigned long long>& _bins, unsigned long long& _maxBinVal)
{
    size_t n = array.elementCount();
    size_t cc = array.componentCount();
    const T* data = array[0];

    int maxParts = omp_get_max_threads();
    std::vector<unsigned long long> partBins(maxParts * binCount, 0);
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
                partBins[p * binCount + binIndexHelper(val, _minVal, _maxVal, binCount)]++;
            }
        }
    }

    _bins.resize(binCount, 0);
    for (int p = 0; p < parts; p++) {
        for (size_t b = 0; b < binCount; b++)
            _bins[b] += partBins[p * binCount + b];
    }
    _maxBinVal = _bins[0];
    for (size_t b = 1; b < binCount; b++) {
        if (_bins[b] > _maxBinVal)
            _maxBinVal = _bins[b];
    }
}

void Histogram::init(const TGD::ArrayContainer& array, size_t componentIndex, float minVal, float maxVal)
{
    assert(_bins.size() == 0);
    _minVal = minVal;
    _maxVal = maxVal;
    switch (array.componentType()) {
    case TGD::int8:
        initHelper<int8_t>(TGD::Array<int8_t>(array), componentIndex, _minVal, _maxVal, 256, _bins, _maxBinVal);
        break;
    case TGD::uint8:
        initHelper<uint8_t>(TGD::Array<uint8_t>(array), componentIndex, _minVal, _maxVal, 256, _bins, _maxBinVal);
        break;
    case TGD::int16:
        initHelper<int16_t>(TGD::Array<int16_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::uint16:
        initHelper<uint16_t>(TGD::Array<uint16_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::int32:
        initHelper<int32_t>(TGD::Array<int32_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::uint32:
        initHelper<uint32_t>(TGD::Array<uint32_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::int64:
        initHelper<int64_t>(TGD::Array<int64_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::uint64:
        initHelper<uint64_t>(TGD::Array<uint64_t>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::float32:
        initHelper<float>(TGD::Array<float>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    case TGD::float64:
        initHelper<double>(TGD::Array<double>(array), componentIndex, _minVal, _maxVal, 1024, _bins, _maxBinVal);
        break;
    }
    _initialized = true;
}
