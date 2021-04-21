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

#include <limits>

#include "file.hpp"


File::File() : _frameIndex(-1)
{
}

TAD::Importer& File::importer()
{
    if (_importer.fileName().size() == 0) {
        _importer.initialize(fileName(), _importerHints);
    }
    return _importer;
}

bool File::init(const std::string& fileName, const TAD::TagList& importerHints, std::string& errorMessage)
{
    _fileName = fileName;
    _importerHints = importerHints;
    TAD::Error tadError = importer().checkAccess();
    if (tadError != TAD::ErrorNone) {
        errorMessage = fileName + ": " + TAD::strerror(tadError);
        return false;
    }
    _description = TAD::ArrayDescription();
    _frame.reset();
    _frameIndex = -1;
    return true;
}

int File::frameCount(std::string& errorMessage)
{
    int arrayCount = importer().arrayCount();
    if (arrayCount < 0) {
        errorMessage = fileName() + ": unknown number of frames";
        return -1;
    } else if (arrayCount == 0) {
        errorMessage = fileName() + ": no frames";
        return -1;
    } else {
        return arrayCount;
    }
}

static bool isCompatible(const TAD::ArrayDescription& desc0, const TAD::ArrayDescription& desc1)
{
    if (desc1.componentType() != desc0.componentType())
        return false;
    if (desc1.componentCount() != desc0.componentCount())
        return false;
    if (desc1.dimensionCount() != desc0.dimensionCount())
        return false;
    for (size_t i = 1; i < desc1.dimensionCount(); i++)
        if (desc1.dimension(i) != desc0.dimension(i))
            return false;
    return true;
}

bool File::setFrameIndex(int index, std::string& errorMessage)
{
    if (index == _frameIndex) {
        return true;
    }
    if (index < 0) {
        _importer = TAD::Importer();
        _frame.reset();
        _frameIndex = -1;
        return true;
    }
    int frCnt = frameCount(errorMessage);
    if (frCnt < 1)
        return false;
    if (index >= frCnt) {
        errorMessage = fileName() + ": " + "array " + std::to_string(index) + " does not exist";
        return false;
    }
    TAD::Error tadError;
    TAD::ArrayContainer a;
    a = importer().readArray(&tadError, index);
    if (tadError != TAD::ErrorNone) {
        errorMessage = fileName() + ": " + TAD::strerror(tadError);
        return false;
    }
    if (_description.dimensionCount() == 0) {
        // first frame to read: initialize description
        if (a.dimensionCount() != 2) {
            errorMessage = fileName() + ": " + "array does not have two dimensions";
            return false;
        }
        for (size_t i = 0; i < a.dimensionCount(); i++) {
            if (a.dimension(i) < 1) {
                errorMessage = fileName() + ": " + "array has invalid dimensions";
                return false;
            } else if (a.dimension(i) >= size_t(std::numeric_limits<int>::max())) {
                errorMessage = fileName() + ": " + "array is too big";
                return false;
            }
        }
        _description = a;
    } else if (!isCompatible(_description, a)) {
        errorMessage = fileName() + ": " + "incompatible arrays";
        return false;
    }
    int channelIndex = (currentFrame() ? currentFrame()->channelIndex() : -1);
    _frame.init(a);
    _frameIndex = index;
    if ((channelIndex == ColorChannelIndex && _frame.colorSpace() == ColorSpaceNone)
            || (channelIndex != ColorChannelIndex && channelIndex >= _frame.channelCount()))
        channelIndex = -1;
    if (channelIndex >= 0)
        _frame.setChannelIndex(channelIndex);
    return true;
}

bool File::reload(std::string& errorMessage)
{
    if (_description.dimensionCount() == 0) {
        // we did not load anything yet
        return setFrameIndex(0, errorMessage);
    }

    TAD::ArrayDescription origDescription = _description;
    TAD::Importer newImporter(fileName(), _importerHints);
    TAD::Error tadError = newImporter.checkAccess();
    if (tadError != TAD::ErrorNone) {
        errorMessage = fileName() + ": " + TAD::strerror(tadError);
        return false;
    }
    TAD::ArrayContainer a;
    a = newImporter.readArray(&tadError);
    if (tadError != TAD::ErrorNone) {
        errorMessage = fileName() + ": " + TAD::strerror(tadError);
        return false;
    }
    if (!isCompatible(origDescription, a)) {
        errorMessage = fileName() + ": " + "incompatible after reload";
        return false;
    }

    int index = frameIndex();
    int channelIndex = (currentFrame() ? currentFrame()->channelIndex() : -1);
    if (index == 0) {
        _importer = newImporter;
        _description = a;
        _frame.init(a);
        _frameIndex = 0;
        if ((channelIndex == ColorChannelIndex && _frame.colorSpace() == ColorSpaceNone)
                || (channelIndex != ColorChannelIndex && channelIndex >= _frame.channelCount()))
            channelIndex = -1;
        if (channelIndex >= 0)
            _frame.setChannelIndex(channelIndex);
        return true;
    } else {
        int frCnt = frameCount(errorMessage);
        if (frCnt < 1)
            return false;
        if (index >= frCnt)
            index = frCnt - 1;
        _importer = newImporter;
        _description = a;
        _frameIndex = -1;
        if (setFrameIndex(index, errorMessage)) {
            if ((channelIndex == ColorChannelIndex && _frame.colorSpace() == ColorSpaceNone)
                    || (channelIndex != ColorChannelIndex && channelIndex >= _frame.channelCount()))
                channelIndex = -1;
            if (channelIndex >= 0)
                _frame.setChannelIndex(channelIndex);
            return true;
        } else {
            return false;
        }
    }
}
