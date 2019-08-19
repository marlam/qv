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

#include <cerrno>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <QGuiApplication>
#include <QSurfaceFormat>

#include "set.hpp"
#include "qv.hpp"


int main(int argc, char* argv[])
{
    std::string errMsg;

    // Initialize Qt
    QGuiApplication app(argc, argv);

    // Build the set of files to view
    Set set;
    bool err = false;
    for (int i = 1; i < argc; i++) {
        std::string name = argv[i];
        if (std::filesystem::exists(name)) {
            if (std::filesystem::is_directory(name)) {
                std::vector<std::string> paths;
                for (auto& p: std::filesystem::directory_iterator(name))
                    paths.push_back(p.path());
                std::sort(paths.begin(), paths.end());
                for (size_t i = 0; i < paths.size(); i++) {
                    if (!set.add(paths[i], errMsg)) {
                        fprintf(stderr, "Ignoring %s\n", errMsg.c_str());
                    }
                }
            } else {
                if (!set.add(name, errMsg)) {
                    fprintf(stderr, "%s\n", errMsg.c_str());
                    err = true;
                }
            }
        } else {
            fprintf(stderr, "%s: %s\n", name.c_str(), std::strerror(ENOENT));
            err = true;
        }
    }
    if (err) {
        return 1;
    }
    if (set.fileCount() == 0) {
        fprintf(stderr, "Usage: %s <directory|file...>\n", argv[0]);
        return 1;
    }

    // Initialize set
    if (!set.setFileIndex(0, errMsg)) {
        fprintf(stderr, "%s\n", errMsg.c_str());
        return 1;
    }

    // Initialize parameters
    Parameters parameters;
    parameters.magInterpolation = (set.currentFile()->currentFrame()->channelIndex() == ColorChannelIndex);

    // Set the OpenGL context parameters
    QSurfaceFormat format;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        format.setVersion(3, 0);
    } else {
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(3, 3);
    }
    QSurfaceFormat::setDefaultFormat(format);

    // Present window
    QV qv(set, parameters);
    qv.show();

    return app.exec();
}
