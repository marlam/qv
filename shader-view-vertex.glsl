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

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texcoord;

uniform float quadFactorX, quadFactorY;
uniform float quadOffsetX, quadOffsetY;
uniform float xFactor, yFactor;
uniform float xOffset, yOffset;

uniform float texCoordFactorX, texCoordFactorY;
uniform float texCoordOffsetX, texCoordOffsetY;

smooth out vec2 vtexcoord;

void main(void)
{
    vtexcoord = vec2(texCoordFactorX, texCoordFactorY) * texcoord + vec2(texCoordOffsetX, texCoordOffsetY);
    vec2 qpos = ((0.5 * pos.xy + 0.5) + vec2(quadOffsetX, quadOffsetY)) * vec2(quadFactorX, quadFactorY);
    qpos = 2.0 * qpos - 1.0;
    gl_Position = vec4(qpos * vec2(xFactor, yFactor) + vec2(xOffset, yOffset), 0.0, 1.0);
}
