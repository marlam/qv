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

#ifndef QV_SET_HPP
#define QV_SET_HPP

#include "file.hpp"
#include "parameters.hpp"

class Set {
private:
    TAD::TagList _importerHints;
    std::vector<File> _files;
    std::vector<Parameters> _parameters;
    int _fileIndex;
    bool _keepParameterIndex;
    int _parameterIndex;

public:
    Set();

    void setImporterHints(const TAD::TagList& importerHints) { _importerHints = importerHints; }

    bool addFile(const std::string& fileName, std::string& errorMessage);
    void removeFile(int fileIndex);
    int fileCount() const { return _files.size(); }

    bool setFileIndex(int index, std::string& errorMessage);
    int fileIndex() const { return _fileIndex; }

    File* file(int index) { return &(_files[index]); }
    File* currentFile() { return fileIndex() >= 0 ? file(fileIndex()) : nullptr; }

    bool applyCurrentParametersToAllFiles() const { return _keepParameterIndex; }
    void setApplyCurrentParametersToAllFiles(bool flag);
    void toggleApplyCurrentParametersToAllFiles() { setApplyCurrentParametersToAllFiles(!applyCurrentParametersToAllFiles()); }
    int parameterIndex() const { return _parameterIndex; }
    Parameters* parameters(int index) { return &(_parameters[index]); }
    Parameters* currentParameters() { return parameterIndex() >= 0 ? parameters(parameterIndex()) : nullptr; }

    std::string currentDescription();
};

#endif
