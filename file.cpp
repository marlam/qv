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

#include "file.hpp"

File::File() : _frameIndex(-1)
{
}

TAD::Importer File::importer()
{
    if (_importer.fileName().size() == 0) {
        _importer.initialize(fileName());
    }
    return _importer;
}

bool File::init(const std::string& fileName, std::string& errorMessage)
{
    _fileName = fileName;
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

int File::frameCount()
{
    int arrayCount = importer().arrayCount();
    if (arrayCount <= 0)
        arrayCount = 1;
    return arrayCount;
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
    if (index >= frameCount()) {
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
        _description = a;
    } else if (!isCompatible(_description, a)) {
        errorMessage = fileName() + ": " + "incompatible arrays";
        return false;
    }
    _frame.init(a);
    _frameIndex = index;
    return true;
}

bool File::reload(std::string& errorMessage)
{
    if (_description.dimensionCount() == 0) {
        // we did not load anything yet
        return setFrameIndex(0, errorMessage);
    }

    TAD::ArrayDescription origDescription = _description;
    TAD::Importer newImporter(fileName());
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

    _importer = newImporter;
    _description = a;
    int index = frameIndex();
    _frame.init(a);
    _frameIndex = 0;
    if (index == 0) {
        return true;
    } else {
        if (index >= frameCount())
            index = frameCount() - 1;
        return setFrameIndex(index, errorMessage);
    }
}
