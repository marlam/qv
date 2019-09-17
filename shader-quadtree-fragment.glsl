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

uniform sampler2D quadTex0;
uniform sampler2D quadTex1;
uniform sampler2D quadTex2;
uniform sampler2D quadTex3;
uniform bool haveQuadTex0;
uniform bool haveQuadTex1;
uniform bool haveQuadTex2;
uniform bool haveQuadTex3;
uniform bool toS0;
uniform bool toS1;
uniform bool toS2;
uniform bool toS3;
uniform float nan;

smooth in vec2 vtexcoord;

layout(location = 0) out vec4 fcolor;

float linear_to_s(float x)
{
    return (x <= 0.0031308 ? (x * 12.92) : (1.055 * pow(x, 1.0 / 2.4) - 0.055));
}

void main(void)
{
    int quadX = (vtexcoord.x < 0.5 ? 0 : 1);
    int quadY = (vtexcoord.y < 0.5 ? 0 : 1);
    vec2 tc = 2.0 * vtexcoord - vec2(quadX, quadY);
    vec4 v = vec4(nan);
    if (quadX == 0 && quadY == 0 && haveQuadTex0)
        v = texture(quadTex0, tc);
    else if (quadX == 1 && quadY == 0 && haveQuadTex1)
        v = texture(quadTex1, tc);
    else if (quadX == 0 && quadY == 1 && haveQuadTex2)
        v = texture(quadTex2, tc);
    else if (haveQuadTex3)
        v = texture(quadTex3, tc);
    if (toS0)
        v[0] = linear_to_s(v[0]);
    if (toS1)
        v[1] = linear_to_s(v[1]);
    if (toS2)
        v[2] = linear_to_s(v[2]);
    if (toS3)
        v[3] = linear_to_s(v[3]);
    fcolor = v;
}
