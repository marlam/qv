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

#ifndef QV_COLOR_HPP
#define QV_COLOR_HPP

#include <cmath>
#include <limits>

typedef enum {
    // these values are shared with shader-view-fragment.glsl
    ColorSpaceNone        = 0,
    ColorSpaceLinearGray  = 1,
    ColorSpaceLinearRGB   = 2,
    ColorSpaceSGray       = 3, // only uint8, 1 channel
    ColorSpaceSRGB        = 4, // only uint8, 3 or 4 channels, alpha must be in channel 3
    ColorSpaceY           = 5,
    ColorSpaceXYZ         = 6
} ColorSpace;

const int ColorChannelIndex = std::numeric_limits<int>::max();

inline float toLinear(float x)
{
    constexpr float c0 = 0.077399380805f; // 1.0f / 12.92f;
    constexpr float c1 = 0.947867298578f; // 1.0f / 1.055f;
    return (x <= 0.04045f ? (x * c0) : std::pow((x + 0.055f) * c1, 2.4f));
}

inline float rgbToY(float r, float g, float b)
{
    return 100.0f * (0.2126f * r + 0.7152f * g + 0.0722f * b);
}

#endif
