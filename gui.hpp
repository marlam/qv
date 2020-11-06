/*
 * Copyright (C) 2020 Computer Graphics Group, University of Siegen
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

#ifndef GUI_HPP
#define GUI_HPP

#include <QMainWindow>

#include "set.hpp"
#include "qv.hpp"

class QAction;

class Gui : public QMainWindow
{
Q_OBJECT

private:
    Set& _set;
    QV* _qv;
    QAction* _viewToggleLinearInterpolationAction;
    QAction* _viewToggleGridAction;

private slots:
    void fileOpen();
    void fileClose();
    void fileReload();
    void fileSaveCurrentView();
    void fileSaveView();
    void fileCopyCurrentView();
    void fileCopyView();
    void fileNext();
    void filePrev();
    void fileNext10();
    void filePrev10();
    void fileNext100();
    void filePrev100();
    void frameNext();
    void framePrev();
    void frameNext10();
    void framePrev10();
    void frameNext100();
    void framePrev100();
    void channelColor();
    void channel0();
    void channel1();
    void channel2();
    void channel3();
    void channel4();
    void channel5();
    void channel6();
    void channel7();
    void channel8();
    void channel9();
    void viewToggleFullscreen();
    void viewZoomIn();
    void viewZoomOut();
    void viewZoomReset();
    void viewRecenter();
    void viewToggleLinearInterpolation();
    void viewToggleGrid();

    void updateFromParameters();

public:
    Gui(Set& set);
};

#endif
