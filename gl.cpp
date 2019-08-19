/*
 * Copyright (C) 2017, 2018, 2019
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

#include <cstdlib>
#include <cstring>

#include <QOpenGLContext>

#include "gl.hpp"

QOpenGLExtraFunctions* getGlFunctionsFromCurrentContext()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        return nullptr;
    }
    return ctx->extraFunctions();
}

void glCheck(const char* callingFunction, const char* file, int line)
{
    auto gl = getGlFunctionsFromCurrentContext();
    GLenum err = gl->glGetError();
    if (err != GL_NO_ERROR) {
        qCritical("%s:%d: OpenGL error 0x%04X in the following function:", file, line, err);
        qCritical("%s", callingFunction);
        std::exit(1);
    }
}

static unsigned int pbo = 0;

unsigned int glGetGlobalPBO()
{
    if (pbo == 0) {
        auto gl = getGlFunctionsFromCurrentContext();
        gl->glGenBuffers(1, &pbo);
    }
    return pbo;
}
