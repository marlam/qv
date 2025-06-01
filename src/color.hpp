/*
 * Copyright (C) 2019, 2020, 2021
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * Copyright (C) 2023, 2024, 2025
 * Martin Lambers <marlam@marlam.de>
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
    ColorSpaceSGray       = 3,
    ColorSpaceSRGB        = 4,
    ColorSpaceY           = 5,
    ColorSpaceXYZ         = 6
} ColorSpace;

const int ColorChannelIndex = std::numeric_limits<int>::max();

inline float toLinear(float x)
{
    constexpr float c0 = 0.077399380805f; // 1.0f / 12.92f;
    constexpr float c1 = 0.947867298578f; // 1.0f / 1.055f;
    return (x <= 0.04045f ? (x * c0) : std::powf((x + 0.055f) * c1, 2.4f));
}

inline float toS(float x)
{
    constexpr float c0 = 0.416666666667f;
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * std::powf(x, c0) - 0.055f));
}

inline float rgbToY(float r, float g, float b)
{
    return 100.0f * (0.2126f * r + 0.7152f * g + 0.0722f * b);
}

inline float YToL(float Y)
{
    constexpr float one_over_d65_y = 0.01f; // 1 / D65.Y
    constexpr float c0 = 0.00885645167904f; // 6.0f * 6.0f * 6.0f / (29.0f * 29.0f * 29.0f);
    constexpr float c1 = 903.296296296f; // 29.0f * 29.0f * 29.0f / (3.0f * 3.0f * 3.0f);

    float ratio = one_over_d65_y * Y;
    float L = (ratio <= c0 ? c1 * ratio : 116.0f * std::cbrt(ratio) - 16.0f);
    return L;
}

inline float rgbToL(float r, float g, float b)
{
    return YToL(rgbToY(r, g, b));
}

#endif
