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

#include <filesystem>

#include "set.hpp"


Set::Set() : _fileIndex(-1)
{
}

bool Set::add(const std::string& fileName, std::string& errorMessage)
{
    File file;
    if (file.init(fileName, errorMessage)) {
        _files.push_back(file);
        return true;
    } else {
        return false;
    }
}

bool Set::setFileIndex(int index, std::string& errorMessage)
{
    if (_fileIndex == index) {
        return true;
    }

    if (index < 0 || index >= fileCount()) {
        errorMessage = "file " + std::to_string(index) + " does not exist";
        return false;
    }

    int frameIndex = 0;
    if (_fileIndex >= 0) {
        frameIndex = _files[_fileIndex].frameIndex();
    }

    if (frameIndex < 0)
        frameIndex = 0;
    else if (frameIndex >= _files[index].frameCount())
        frameIndex = _files[index].frameCount() - 1;

    if (!_files[index].setFrameIndex(frameIndex, errorMessage))
        return false;

    _fileIndex = index;
    return true;
}

std::string Set::currentDescription()
{
    std::string desc;
    if (currentFile()) {
        if (fileCount() > 1) {
            desc = std::to_string(fileIndex())
                + '/' + std::to_string(fileCount()) + ' ';
        }
        std::string fileName = std::filesystem::path(currentFile()->fileName()).filename();
        desc += fileName + ' ';
        if (currentFile()->frameCount() > 1) {
            desc += std::to_string(currentFile()->frameIndex())
                + '/' + std::to_string(currentFile()->frameCount()) + ' ';
        }
        if (currentFile()->currentFrame()->channelCount() > 1) {
            if (currentFile()->currentFrame()->channelIndex() == ColorChannelIndex)
                desc += "color";
            else
                desc += "ch. " + std::to_string(currentFile()->currentFrame()->channelIndex())
                    + '/' + std::to_string(currentFile()->currentFrame()->channelCount());
        }
    }
    return desc;
}
