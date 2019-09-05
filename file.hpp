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

#ifndef QV_FILE_HPP
#define QV_FILE_HPP

#include <tad/io.hpp>

#include "frame.hpp"

// All frames in a file are uniform: 2d, same width/height, same component types, same component number.

class File {
private:
    std::string _fileName;
    TAD::Importer _importer;
    TAD::ArrayDescription _description;
    Frame _frame;
    int _frameIndex;

    TAD::Importer importer();

public:
    File();

    bool init(const std::string& fileName, std::string& errorMessage);

    const std::string& fileName() const { return _fileName; }
    int frameCount(std::string& errorMessage); // returns < 1 on error

    bool setFrameIndex(int index, std::string& errorMessage); // index=-1 is allowed and frees resources; this cannot fail
    int frameIndex() const { return _frameIndex; }
    Frame* currentFrame() { return frameIndex() >= 0 ? &_frame : nullptr; }

    bool reload(std::string& errorMessage);
};

#endif
