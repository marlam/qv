/*
 * Copyright (C) 2019, 2020, 2021
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

uniform sampler2D tex0, tex1, tex2, alphaTex;

uniform float dataWidth, dataHeight;

// these values are shared with frame.hpp!
const int ColorSpaceNone        = 0;
const int ColorSpaceLinearGray  = 1;
const int ColorSpaceLinearRGB   = 2;
const int ColorSpaceSGray       = 3;
const int ColorSpaceSRGB        = 4;
const int ColorSpaceY           = 5;
const int ColorSpaceXYZ         = 6;
uniform bool showColor;
uniform int colorSpace;
uniform int channelCount;
uniform int dataChannelIndex;
uniform int colorChannel0Index;
uniform int colorChannel1Index;
uniform int colorChannel2Index;
uniform int alphaChannelIndex;

uniform bool srgbWas8Bit;
uniform bool srgbWas16Bit;

uniform float visMinVal;
uniform float visMaxVal;

uniform bool dynamicRangeReduction;
uniform float drrBrightness;

uniform bool colorMap;
uniform sampler2D colorMapTex;

uniform bool magGrid;

smooth in vec2 vTexCoord;
smooth in vec2 vDataCoord;

layout(location = 0) out vec4 fcolor;

const vec3 d65_xyz = vec3(95.047, 100.000, 108.883);
const float d65_u_prime = 0.197839824821;
const float d65_v_prime = 0.468336302932;

vec3 adjust_y(vec3 xyz, float new_y)
{
    float sum = xyz.x + xyz.y + xyz.z;
    // keep old chromaticity in terms of x, y
    float x = xyz.x / sum;
    float y = xyz.y / sum;
    // apply new luminance
    float r = new_y / y;
    return vec3(r * x, new_y, r * (1.0 - x - y));
}

vec3 l_to_xyz(float l) // l from Luv color space (perceptually linear)
{
    vec3 xyz;
    if (l <= 8.0) {
        const float c0 = 0.00110705645988; // 3.0f * 3.0f * 3.0f / (29.0f * 29.0f * 29.0f);
        xyz.y = d65_xyz.y * l * c0;
    } else {
        float tmp = (l + 16.0f) / 116.0f;
        xyz.y = d65_xyz.y * tmp * tmp * tmp;
    }
    xyz.x = xyz.y * (9.0f * d65_u_prime) / (4.0f * d65_v_prime);
    xyz.z = xyz.y * (12.0f - 3.0f * d65_u_prime - 20.0f * d65_v_prime) / (4.0f * d65_v_prime);
    return xyz;
}

float xyz_to_l(vec3 xyz) // l from Luv color space (perceptually linear)
{
    const float c0 = 0.00885645167904; // 6.0f * 6.0f * 6.0f / (29.0f * 29.0f * 29.0f);
    float ratio = xyz.y / d65_xyz.y;
    float l;
    if (ratio <= c0) {
        const float c1 = 903.296296296; // 29.0f * 29.0f * 29.0f / (3.0f * 3.0f * 3.0f);
        l = c1 * ratio;
    } else {
        l = 116.0 * pow(ratio, (1.0 / 3.0)) - 16.0;
    }
    return l;
}

vec3 rgb_to_xyz(vec3 rgb)
{
    // values from http://terathon.com/blog/rgb-xyz-conversion-matrix-accuracy/
    return 100.0 * vec3(
            (0.412391 * rgb.r + 0.357584 * rgb.g + 0.180481 * rgb.b),
            (0.212639 * rgb.r + 0.715169 * rgb.g + 0.072192 * rgb.b),
            (0.019331 * rgb.r + 0.119195 * rgb.g + 0.950532 * rgb.b));
}

vec3 xyz_to_rgb(vec3 xyz)
{
    // values from http://terathon.com/blog/rgb-xyz-conversion-matrix-accuracy/
    return 0.01 * vec3(
            (+3.240970 * xyz.x - 1.537383 * xyz.y - 0.498611 * xyz.z),
            (-0.969244 * xyz.x + 1.875968 * xyz.y + 0.041555 * xyz.z),
            (+0.055630 * xyz.x - 0.203977 * xyz.y + 1.056972 * xyz.z));
}

float s_to_linear(float x)
{
    const float c0 = 0.077399380805; // 1.0 / 12.92
    const float c1 = 0.947867298578; // 1.0 / 1.055;
    return (x <= 0.04045 ? (x * c0) : pow((x + 0.055) * c1, 2.4));
}

float linear_to_s(float x)
{
    const float c0 = 0.416666666667; // 1.0 / 2.4
    return (x <= 0.0031308 ? (x * 12.92) : (1.055 * pow(x, c0) - 0.055));
}

vec3 rgb_to_srgb(vec3 rgb)
{
    return vec3(linear_to_s(rgb.r), linear_to_s(rgb.g), linear_to_s(rgb.b));
}

// Dynamic Range Reduction proposed by C. Schlick in the chapter
// "Quantization Techniques for the Visualization of High Dynamic Range Pictures"
// in Photorealistic Rendering Techniques, Springer, 1994.
float uniformRationalQuantization(float v /* in [0,1] */)
{
    // drrBrightness in [1,infty)
    return drrBrightness * v / ((drrBrightness - 1.0) * v + 1.0);
}

void main(void)
{
    vec3 srgb;

    if (vDataCoord.x >= dataWidth || vDataCoord.y >= dataHeight)
        discard;

    if (!showColor) {
        // Get value
        float v = texture(tex0, vTexCoord)[dataChannelIndex];
        if ((colorSpace == ColorSpaceSGray || colorSpace == ColorSpaceSRGB) && srgbWas8Bit)
            v = linear_to_s(v) * 255.0f;
        // Apply range selection
        v = (v - visMinVal) / (visMaxVal - visMinVal);
        v = clamp(v, 0.0, 1.0);
        // Apply dynamic range reduction
        if (dynamicRangeReduction) {
            v = uniformRationalQuantization(v);
        }
        // Apply color map
        if (colorMap) {
            srgb = rgb_to_srgb(texture(colorMapTex, vec2(v, 0.5)).rgb);
        } else {
            vec3 xyz = l_to_xyz(100.0 * v);
            srgb = rgb_to_srgb(xyz_to_rgb(xyz));
        }
    } else {
        // Read data into canonical form
        vec4 data = vec4(0.0, 0.0, 0.0, 1.0);
        if (channelCount <= 4) {
            vec4 tmpData = texture(tex0, vTexCoord);
            data[0] = tmpData[colorChannel0Index];
            data[1] = tmpData[colorChannel1Index];
            data[2] = tmpData[colorChannel2Index];
            if (alphaChannelIndex >= 0) {
                data[3] = tmpData[alphaChannelIndex];
            }
        } else {
            data[0] = texture(tex0, vTexCoord).r;
            data[1] = texture(tex1, vTexCoord).r;
            data[2] = texture(tex2, vTexCoord).r;
            if (alphaChannelIndex >= 0) {
                data[3] = texture(alphaTex, vTexCoord).r;
            }
        }
        if (colorSpace == ColorSpaceSGray || colorSpace == ColorSpaceSRGB) {
            if (srgbWas16Bit)
                data /= 65535.0f;
            if (!srgbWas8Bit)
                for (int i = 0; i < 3; i++)
                    data[i] = s_to_linear(data[i]);
        }
        // Get color
        vec3 xyz;
        if (colorSpace == ColorSpaceLinearGray || colorSpace == ColorSpaceSGray) {
            xyz = l_to_xyz(100.0 * data[0]);
        } else if (colorSpace == ColorSpaceLinearRGB || colorSpace == ColorSpaceSRGB) {
            xyz = rgb_to_xyz(vec3(data[0], data[1], data[2]));
        } else if (colorSpace == ColorSpaceY) {
            xyz = adjust_y(d65_xyz, data[0]);
        } else if (colorSpace == ColorSpaceXYZ) {
            xyz = vec3(data[0], data[1], data[2]);
        }
        // Apply range selection
        float y = xyz.y;
        y = (y - visMinVal) / (visMaxVal - visMinVal);
        y = clamp(y, 0.0, 1.0);
        // Apply dynamic range reduction
        if (dynamicRangeReduction) {
            y = uniformRationalQuantization(y);
        }
        xyz = adjust_y(xyz, 100.0 * y);
        // Apply color map
        if (colorMap) {
            srgb = rgb_to_srgb(texture(colorMapTex, vec2(0.01 * xyz_to_l(xyz), 0.5)).rgb);
        } else {
            vec3 rgb = xyz_to_rgb(xyz);
            if (alphaChannelIndex >= 0) {
                float alpha = clamp(data[3], 0.0, 1.0);
                rgb = clamp(rgb, vec3(0.0), vec3(1.0));
                rgb = rgb * alpha + vec3(1.0 - alpha);
            }
            srgb = rgb_to_srgb(rgb);
        }
    }

    // Apply grid
    if (magGrid) {
        vec2 texelCoord = vTexCoord * vec2(textureSize(tex0, 0));
        vec2 fragmentSizeInTexels = vec2(dFdx(texelCoord.x), dFdy(texelCoord.y));
        if (all(lessThan(fragmentSizeInTexels, vec2(0.5)))
                && any(lessThanEqual(fract(texelCoord), fragmentSizeInTexels))) {
            float l = dot(srgb, srgb);
            srgb = vec3(1.0 - l);
        }
    }
    fcolor = vec4(srgb, 1.0);
}
