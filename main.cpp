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

// for isatty():
#if __has_include(<unistd.h>)
# include <unistd.h>
#elif __has_include(<io.h>)
# include <io.h>
#endif

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QOpenGLContext>

#include "version.hpp"
#include "set.hpp"
#include "gl.hpp"
#include "gui.hpp"


int main(int argc, char* argv[])
{
    // Initialize Qt
    QApplication app(argc, argv);
    QApplication::setApplicationName("qv");
    QApplication::setApplicationVersion(QV_VERSION);
    QCommandLineParser parser;
    parser.setApplicationDescription("A quick viewer for 2D data -- see https://marlam.de/qv");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[directory|file...]", "Data to display.");
    parser.addOptions({
            { { "i", "input" }, "Set tag for import (can be given more than once).", "KEY=VALUE" },
    });
    parser.process(app);
    QStringList posArgs = parser.positionalArguments();

    // Evaluate the -i|--input option
    TAD::TagList importerHints;
    QStringList tags = parser.values("input");
    for (int i = 0; i < tags.size(); i++) {
        std::string tag = qPrintable(tags[i]);
        size_t j = tag.find('=', 1);
        if (j == std::string::npos) {
            importerHints.set(tag, "");
        } else {
            importerHints.set(tag.substr(0, j), tag.substr(j + 1));
        }
    }

    // Build the set of files to view
    Set set;
    set.setImporterHints(importerHints);
    bool err = false;
    std::string errMsg;
    for (int i = 0; i < posArgs.size(); i++) {
        std::string name = qPrintable(posArgs[i]);
        if (std::filesystem::exists(name)) {
            if (std::filesystem::is_directory(name)) {
                std::vector<std::string> paths;
                for (auto& p: std::filesystem::directory_iterator(name))
                    paths.push_back(p.path().string());
                std::sort(paths.begin(), paths.end());
                for (size_t i = 0; i < paths.size(); i++) {
                    if (!set.addFile(paths[i], errMsg)) {
                        fprintf(stderr, "ignoring %s\n", errMsg.c_str());
                    }
                }
            } else {
                if (!set.addFile(name, errMsg)) {
                    err = true;
                    break;
                }
            }
        } else {
            errMsg = name + ": " + std::strerror(ENOENT);
            err = true;
        }
    }
    if (!err && set.fileCount() > 0 && !set.setFileIndex(0, errMsg)) {
        err = true;
    }
    if (err) {
        // if we started from a terminal, print error to stderr, otherwise to GUI
        if (isatty(fileno(stderr))) {
            fprintf(stderr, "%s\n", errMsg.c_str());
        } else {
            QMessageBox::critical(nullptr, "Error", errMsg.c_str());
        }
        return 1;
    }

    // Set the OpenGL context parameters
    QSurfaceFormat format;
    format.setRedBufferSize(10);
    format.setGreenBufferSize(10);
    format.setBlueBufferSize(10);
    format.setAlphaBufferSize(0);
    format.setStencilBufferSize(0);
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        format.setVersion(3, 0);
    } else {
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(3, 3);
    }
    QSurfaceFormat::setDefaultFormat(format);

    // Create and show GUI
    Gui gui(set);
    gui.show();

    return app.exec();
}
