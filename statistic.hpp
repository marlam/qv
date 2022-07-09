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

#ifndef QV_STATISTIC_HPP
#define QV_STATISTIC_HPP

#include <tad/array.hpp>

class Statistic {
private:
    bool _initialized;
    long long _finiteValues;
    float _minVal;
    float _maxVal;
    float _sampleMean;
    float _sampleVariance;
    float _sampleDeviation;

public:
    Statistic();

    void init(const TAD::ArrayContainer& array, size_t componentIndex);

    bool initialized() const { return _initialized; }
    long long finiteValues() const { return _finiteValues; }
    float minVal() const { return _minVal; }
    float maxVal() const { return _maxVal; }
    float sampleMean() const { return _sampleMean; }
    float sampleVariance() const { return _sampleVariance; }
    float sampleDeviation() const { return _sampleDeviation; }
};

#endif
