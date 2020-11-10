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


class Gui : public QMainWindow
{
Q_OBJECT

private:
    Set& _set;
    QV* _qv;
    QMenu* _contextMenu;

    QAction* _fileOpenAction;
    QAction* _fileCloseAction;
    QAction* _fileReloadAction;
    QAction* _fileSaveCurrentViewAction;
    QAction* _fileSaveViewAction;
    QAction* _fileCopyCurrentViewAction;
    QAction* _fileCopyViewAction;
    QAction* _fileNextAction;
    QAction* _filePrevAction;
    QAction* _fileNext10Action;
    QAction* _filePrev10Action;
    QAction* _fileNext100Action;
    QAction* _filePrev100Action;
    QAction* _fileQuitAction;
    QAction* _frameNextAction;
    QAction* _framePrevAction;
    QAction* _frameNext10Action;
    QAction* _framePrev10Action;
    QAction* _frameNext100Action;
    QAction* _framePrev100Action;
    QAction* _channelColorAction;
    QAction* _channel0Action;
    QAction* _channel1Action;
    QAction* _channel2Action;
    QAction* _channel3Action;
    QAction* _channel4Action;
    QAction* _channel5Action;
    QAction* _channel6Action;
    QAction* _channel7Action;
    QAction* _channel8Action;
    QAction* _channel9Action;
    QAction* _viewToggleFullscreenAction;
    QAction* _viewZoomInAction;
    QAction* _viewZoomOutAction;
    QAction* _viewZoomResetAction;
    QAction* _viewRecenterAction;
    QAction* _viewToggleLinearInterpolationAction;
    QAction* _viewToggleGridAction;
    QAction* _rangeToggleOverlayAction;
    QAction* _rangeDecLoAction;
    QAction* _rangeIncLoAction;
    QAction* _rangeDecHiAction;
    QAction* _rangeIncHiAction;
    QAction* _rangeShiftLeftAction;
    QAction* _rangeShiftRightAction;
    QAction* _rangeResetAction;
    QAction* _rangeDRRToggleAction;
    QAction* _rangeDRRDecBrightnessAction;
    QAction* _rangeDRRIncBrightnessAction;
    QAction* _rangeDRRResetBrightnessAction;
    QAction* _colorMapToggleOverlayAction;
    QAction* _colorMapDisableAction;
    QAction* _colorMapCycleSequentialAction;
    QAction* _colorMapCycleDivergingAction;
    QAction* _colorMapQualitativeAction;
    QAction* _colorMapCustomAction;
    QAction* _analysisToggleApplyCurrentParametersToAllFilesAction;
    QAction* _analysisToggleInfoAction;
    QAction* _analysisToggleStatisticsAction;
    QAction* _analysisToggleValueAction;
    QAction* _helpAboutAction;

    QMenu* addQVMenu(const QString& title);
    void addQVAction(QAction* action, QMenu* menu);

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
    void fileQuit();
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
    void rangeToggleOverlay();
    void rangeDecLo();
    void rangeIncLo();
    void rangeDecHi();
    void rangeIncHi();
    void rangeShiftLeft();
    void rangeShiftRight();
    void rangeReset();
    void rangeDRRToggle();
    void rangeDRRDecBrightness();
    void rangeDRRIncBrightness();
    void rangeDRRResetBrightness();
    void colorMapToggleOverlay();
    void colorMapDisable();
    void colorMapCycleSequential();
    void colorMapCycleDiverging();
    void colorMapQualitative();
    void colorMapCustom();
    void analysisToggleApplyCurrentParametersToAllFiles();
    void analysisToggleInfo();
    void analysisToggleStatistics();
    void analysisToggleValue();
    void helpAbout();

    void updateFromParameters();

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent* event) override;
#endif

public:
    Gui(Set& set);
};

#endif
