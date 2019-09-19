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
#include <cmath>

#include "histogram.hpp"


Histogram::Histogram()
{
}

static int binIndexHelper(float v, float minVal, float maxVal,
        const std::vector<unsigned long long>& bins)
{
    int b = (v - minVal) / (maxVal - minVal) * bins.size();
    if (b < 0)
        b = 0;
    else if (b >= int(bins.size()))
        b = bins.size() - 1;
    return b;
}

int Histogram::binIndex(float v) const
{
    return binIndexHelper(v, _minVal, _maxVal, _bins);
}

template<typename T>
static void initHelper(const TAD::Array<T> array, size_t componentIndex, size_t bins,
        float _minVal, float _maxVal,
        std::vector<unsigned long long>& _bins, unsigned long long& _maxBinVal)
{
    _bins.resize(bins);
    size_t n = array.elementCount();
    size_t cc = array.componentCount();
    const T* data = array[0];
    for (size_t e = 0; e < n; e++) {
        T val = data[e * cc + componentIndex];
        if (!std::isfinite(val))
            continue;
        _bins[binIndexHelper(val, _minVal, _maxVal, _bins)]++;
    }
    _maxBinVal = 0;
    for (size_t i = 0; i < _bins.size(); i++)
        if (_bins[i] > _maxBinVal)
            _maxBinVal = _bins[i];
}

void Histogram::init(const TAD::ArrayContainer& array, size_t componentIndex, float minVal, float maxVal)
{
    assert(_bins.size() == 0);
    _minVal = minVal;
    _maxVal = maxVal;
    switch (array.componentType()) {
    case TAD::int8:
        initHelper(TAD::Array<int8_t>(array), componentIndex, 256, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::uint8:
        initHelper(TAD::Array<uint8_t>(array), componentIndex, 256, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::int16:
        initHelper(TAD::Array<int16_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::uint16:
        initHelper(TAD::Array<uint16_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::int32:
        initHelper(TAD::Array<int32_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::uint32:
        initHelper(TAD::Array<uint32_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::int64:
        initHelper(TAD::Array<int64_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::uint64:
        initHelper(TAD::Array<uint64_t>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::float32:
        initHelper(TAD::Array<float>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    case TAD::float64:
        initHelper(TAD::Array<double>(array), componentIndex, 1024, _minVal, _maxVal, _bins, _maxBinVal);
        break;
    }
}
