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

#include <filesystem>

#include "set.hpp"


Set::Set() : _fileIndex(-1), _keepParameterIndex(false), _parameterIndex(-1)
{
}

bool Set::addFile(const std::string& fileName, std::string& errorMessage)
{
    File file;
    if (file.init(fileName, _importerHints, errorMessage)) {
        _files.push_back(file);
        _parameters.push_back(Parameters());
        return true;
    } else {
        return false;
    }
}

void Set::removeFile(int removeIndex)
{
    if (removeIndex < 0 || removeIndex >= fileCount())
        return;

    if (removeIndex == fileIndex()) {
        std::string tmpErrMsg;
        bool ret;
        if (removeIndex == fileCount() - 1) {
            ret = setFileIndex(fileCount() - 2, tmpErrMsg);
            _files.erase(_files.begin() + removeIndex);
            _parameters.erase(_parameters.begin() + removeIndex);
            if (!ret)
                setFileIndex(-1, tmpErrMsg);
        } else {
            ret = setFileIndex(fileIndex() + 1, tmpErrMsg);
            _files.erase(_files.begin() + removeIndex);
            _parameters.erase(_parameters.begin() + removeIndex);
            if (!ret)
                setFileIndex(-1, tmpErrMsg);
            else
                _fileIndex--;
        }
    } else if (removeIndex < fileIndex()) {
        _files.erase(_files.begin() + removeIndex);
        _parameters.erase(_parameters.begin() + removeIndex);
        _fileIndex--;
    } else { // fileIndex() < removeIndex
        _files.erase(_files.begin() + removeIndex);
        _parameters.erase(_parameters.begin() + removeIndex);
    }
    if (removeIndex == parameterIndex()) {
        _parameterIndex = _fileIndex;
    } else if (removeIndex < parameterIndex()) {
        _parameterIndex--;
    } else { // parameterIndex() < removeIndex
        // nothing to be done
    }
}

bool Set::setFileIndex(int index, std::string& errorMessage)
{
    if (_fileIndex == index) {
        if (!_keepParameterIndex)
            _parameterIndex = _fileIndex;
        return true;
    }

    if (index < 0) {
        if (currentFile())
            currentFile()->setFrameIndex(-1, errorMessage); // cannot fail
        _fileIndex = -1;
        _parameterIndex = -1;
        return true;
    }

    if (index >= fileCount()) {
        errorMessage = "file " + std::to_string(index) + " does not exist";
        return false;
    }

    int frameIndex = 0;
    int channelIndex = -1;
    if (_fileIndex >= 0) {
        frameIndex = currentFile()->frameIndex();
        if (frameIndex >= 0)
            channelIndex = currentFile()->currentFrame()->channelIndex();
    }

    if (frameIndex < 0) {
        frameIndex = 0;
    } else {
        int frameCount = _files[index].frameCount(errorMessage);
        if (frameCount == 0)
            return false;
        if (frameCount < 0)
            frameIndex = 0;
        else if (frameIndex >= frameCount)
            frameIndex = frameCount - 1;
    }

    if (!_files[index].setFrameIndex(frameIndex, errorMessage))
        return false;

    if ((channelIndex == ColorChannelIndex && _files[index].currentFrame()->colorSpace() == ColorSpaceNone)
            || (channelIndex != ColorChannelIndex && channelIndex >= _files[index].currentFrame()->channelCount()))
        channelIndex = -1;
    if (channelIndex >= 0)
        _files[index].currentFrame()->setChannelIndex(channelIndex);

    if (_fileIndex >= 0 && _fileIndex < fileCount())
        _files[_fileIndex].setFrameIndex(-1, errorMessage);

    _fileIndex = index;
    if (!_keepParameterIndex || _parameterIndex < 0)
        _parameterIndex = index;
    return true;
}

void Set::setApplyCurrentParametersToAllFiles(bool flag)
{
    _keepParameterIndex = flag;
    if (!_keepParameterIndex)
        _parameterIndex = fileIndex();
}

std::string Set::currentDescription()
{
    std::string desc;
    std::string dummyErrorMsg;
    if (currentFile()) {
        if (fileCount() > 1) {
            desc = std::to_string(fileIndex() + 1)
                + '/' + std::to_string(fileCount()) + ' ';
        }
        std::string fileName = std::filesystem::path(currentFile()->fileName()).filename().string();
        desc += fileName + ' ';
        if (currentFile()->frameCount(dummyErrorMsg) != 1) {
            desc += std::to_string(currentFile()->frameIndex() + 1) + '/';
            if (currentFile()->frameCount(dummyErrorMsg) > 1) {
                desc += std::to_string(currentFile()->frameCount(dummyErrorMsg));
            } else {
                if (!currentFile()->haveSeenLastFrame())
                    desc += ">=";
                desc += std::to_string(currentFile()->maxFrameIndexSoFar() + 1);
            }
            desc += ' ';
        }
        if (currentFile()->currentFrame()->channelCount() > 1) {
            desc += currentFile()->currentFrame()->currentChannelName();
        }
    }
    return desc;
}
