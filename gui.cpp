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

#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include "gui.hpp"
#include "version.hpp"


static void myAddAction(QAction* action, QMenu* menu, QV* qv)
{
    menu->addAction(action);
    qv->addAction(action);
}

Gui::Gui(Set& set) : QMainWindow(),
    _set(set),
    _qv(new QV(_set, this))
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* fileOpenAction = new QAction("&Open file(s)...", this);
    fileOpenAction->setShortcuts({ Qt::Key_O, QKeySequence::Open });
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
    myAddAction(fileOpenAction, fileMenu, _qv);
    QAction* fileReloadAction = new QAction("&Reload current file", this);
    if (QKeySequence(QKeySequence::Refresh) != QKeySequence(Qt::Key_F5))
        fileReloadAction->setShortcuts({ Qt::Key_R, QKeySequence::Refresh });
    else
        fileReloadAction->setShortcuts({ Qt::Key_R });
    connect(fileReloadAction, SIGNAL(triggered()), this, SLOT(fileReload()));
    myAddAction(fileReloadAction, fileMenu, _qv);
    QAction* fileCloseAction = new QAction("&Close current file", this);
    fileCloseAction->setShortcuts({ Qt::Key_W, QKeySequence::Close });
    connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(fileClose()));
    myAddAction(fileCloseAction, fileMenu, _qv);
    fileMenu->addSeparator();
    QAction* fileSaveCurrentViewAction = new QAction("Save current view...", this);
    fileSaveCurrentViewAction->setShortcuts({ Qt::Key_F2 });
    connect(fileSaveCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileSaveCurrentView()));
    myAddAction(fileSaveCurrentViewAction, fileMenu, _qv);
    QAction* fileSaveViewAction = new QAction("&Save 1:1 view...", this);
    fileSaveViewAction->setShortcuts({ Qt::Key_F3, QKeySequence::Save });
    connect(fileSaveViewAction, SIGNAL(triggered()), this, SLOT(fileSaveView()));
    myAddAction(fileSaveViewAction, fileMenu, _qv);
    fileMenu->addSeparator();
    QAction* fileCopyCurrentViewAction = new QAction("Copy current view...", this);
    fileCopyCurrentViewAction->setShortcuts({ Qt::Key_F9 });
    connect(fileCopyCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileCopyCurrentView()));
    myAddAction(fileCopyCurrentViewAction, fileMenu, _qv);
    QAction* fileCopyViewAction = new QAction("&Copy 1:1 view...", this);
    fileCopyViewAction->setShortcuts({ Qt::Key_F10, QKeySequence::Copy });
    connect(fileCopyViewAction, SIGNAL(triggered()), this, SLOT(fileCopyView()));
    myAddAction(fileCopyViewAction, fileMenu, _qv);
    fileMenu->addSeparator();
    QAction* fileNextAction = new QAction("Jump to next file", this);
    fileNextAction->setShortcuts({ Qt::Key_Right });
    connect(fileNextAction, SIGNAL(triggered()), this, SLOT(fileNext()));
    myAddAction(fileNextAction, fileMenu, _qv);
    QAction* filePrevAction = new QAction("Jump to previous file", this);
    filePrevAction->setShortcuts({ Qt::Key_Left });
    connect(filePrevAction, SIGNAL(triggered()), this, SLOT(filePrev()));
    myAddAction(filePrevAction, fileMenu, _qv);
    QAction* fileNext10Action = new QAction("Jump 10 files forward", this);
    fileNext10Action->setShortcuts({ Qt::Key_Down });
    connect(fileNext10Action, SIGNAL(triggered()), this, SLOT(fileNext10()));
    myAddAction(fileNext10Action, fileMenu, _qv);
    QAction* filePrev10Action = new QAction("Jump 10 files backward", this);
    filePrev10Action->setShortcuts({ Qt::Key_Up });
    connect(filePrev10Action, SIGNAL(triggered()), this, SLOT(filePrev10()));
    myAddAction(filePrev10Action, fileMenu, _qv);
    QAction* fileNext100Action = new QAction("Jump 100 files forward", this);
    fileNext100Action->setShortcuts({ Qt::Key_PageDown });
    connect(fileNext100Action, SIGNAL(triggered()), this, SLOT(fileNext100()));
    myAddAction(fileNext100Action, fileMenu, _qv);
    QAction* filePrev100Action = new QAction("Jump 100 files backward", this);
    filePrev100Action->setShortcuts({ Qt::Key_PageUp });
    connect(filePrev100Action, SIGNAL(triggered()), this, SLOT(filePrev100()));
    myAddAction(filePrev100Action, fileMenu, _qv);
    fileMenu->addSeparator();
    QAction *fileQuitAction = new QAction("&Quit", this);
    fileQuitAction->setShortcuts({ Qt::Key_Q, QKeySequence::Quit });
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));
    myAddAction(fileQuitAction, fileMenu, _qv);

    QMenu* frameMenu = menuBar()->addMenu("F&rame");
    QAction* frameNextAction = new QAction("Jump to next frame in this file", this);
    frameNextAction->setShortcuts({ Qt::Key_Right + Qt::ShiftModifier });
    connect(frameNextAction, SIGNAL(triggered()), this, SLOT(frameNext()));
    myAddAction(frameNextAction, frameMenu, _qv);
    QAction* framePrevAction = new QAction("Jump to previous frame in this file", this);
    framePrevAction->setShortcuts({ Qt::Key_Left + Qt::ShiftModifier });
    connect(framePrevAction, SIGNAL(triggered()), this, SLOT(framePrev()));
    myAddAction(framePrevAction, frameMenu, _qv);
    QAction* frameNext10Action = new QAction("Jump 10 frames forward in this file", this);
    frameNext10Action->setShortcuts({ Qt::Key_Down + Qt::ShiftModifier });
    connect(frameNext10Action, SIGNAL(triggered()), this, SLOT(frameNext10()));
    myAddAction(frameNext10Action, frameMenu, _qv);
    QAction* framePrev10Action = new QAction("Jump 10 frames backward in this file", this);
    framePrev10Action->setShortcuts({ Qt::Key_Up + Qt::ShiftModifier });
    connect(framePrev10Action, SIGNAL(triggered()), this, SLOT(framePrev10()));
    myAddAction(framePrev10Action, frameMenu, _qv);
    QAction* frameNext100Action = new QAction("Jump 100 frames forward in this file", this);
    frameNext100Action->setShortcuts({ Qt::Key_PageDown + Qt::ShiftModifier });
    connect(frameNext100Action, SIGNAL(triggered()), this, SLOT(frameNext100()));
    myAddAction(frameNext100Action, frameMenu, _qv);
    QAction* framePrev100Action = new QAction("Jump 100 frames backward in this file", this);
    framePrev100Action->setShortcuts({ Qt::Key_PageUp + Qt::ShiftModifier });
    connect(framePrev100Action, SIGNAL(triggered()), this, SLOT(framePrev100()));
    myAddAction(framePrev100Action, frameMenu, _qv);

    QMenu* channelMenu = menuBar()->addMenu("&Channel");
    QAction* channelColorAction = new QAction("Show color channels of this frame", this);
    channelColorAction->setShortcuts({Qt::Key_C });
    connect(channelColorAction, SIGNAL(triggered()), this, SLOT(channelColor()));
    myAddAction(channelColorAction, channelMenu, _qv);
    QAction* channel0Action = new QAction("Show channel 0 of this frame", this);
    channel0Action->setShortcuts({Qt::Key_0 });
    connect(channelColorAction, SIGNAL(triggered()), this, SLOT(channelColor()));
    connect(channel0Action, SIGNAL(triggered()), this, SLOT(channel0()));
    myAddAction(channel0Action, channelMenu, _qv);
    QAction* channel1Action = new QAction("Show channel 1 of this frame", this);
    channel1Action->setShortcuts({Qt::Key_1 });
    connect(channel1Action, SIGNAL(triggered()), this, SLOT(channel1()));
    myAddAction(channel1Action, channelMenu, _qv);
    QAction* channel2Action = new QAction("Show channel 2 of this frame", this);
    channel2Action->setShortcuts({Qt::Key_2 });
    connect(channel2Action, SIGNAL(triggered()), this, SLOT(channel2()));
    myAddAction(channel2Action, channelMenu, _qv);
    QAction* channel3Action = new QAction("Show channel 3 of this frame", this);
    channel3Action->setShortcuts({Qt::Key_3 });
    connect(channel3Action, SIGNAL(triggered()), this, SLOT(channel3()));
    myAddAction(channel3Action, channelMenu, _qv);
    QAction* channel4Action = new QAction("Show channel 4 of this frame", this);
    channel4Action->setShortcuts({Qt::Key_4 });
    connect(channel4Action, SIGNAL(triggered()), this, SLOT(channel4()));
    myAddAction(channel4Action, channelMenu, _qv);
    QAction* channel5Action = new QAction("Show channel 5 of this frame", this);
    channel5Action->setShortcuts({Qt::Key_5 });
    connect(channel5Action, SIGNAL(triggered()), this, SLOT(channel5()));
    myAddAction(channel5Action, channelMenu, _qv);
    QAction* channel6Action = new QAction("Show channel 6 of this frame", this);
    channel6Action->setShortcuts({Qt::Key_6 });
    connect(channel6Action, SIGNAL(triggered()), this, SLOT(channel6()));
    myAddAction(channel6Action, channelMenu, _qv);
    QAction* channel7Action = new QAction("Show channel 7 of this frame", this);
    channel7Action->setShortcuts({Qt::Key_7 });
    connect(channel7Action, SIGNAL(triggered()), this, SLOT(channel7()));
    myAddAction(channel7Action, channelMenu, _qv);
    QAction* channel8Action = new QAction("Show channel 8 of this frame", this);
    channel8Action->setShortcuts({Qt::Key_8 });
    connect(channel8Action, SIGNAL(triggered()), this, SLOT(channel8()));
    myAddAction(channel8Action, channelMenu, _qv);
    QAction* channel9Action = new QAction("Show channel 9 of this frame", this);
    channel9Action->setShortcuts({Qt::Key_9 });
    connect(channel9Action, SIGNAL(triggered()), this, SLOT(channel9()));
    myAddAction(channel9Action, channelMenu, _qv);

    QMenu* viewMenu = menuBar()->addMenu("&View");
    QAction* viewToggleFullscreenAction = new QAction("Toggle &Fullscreen", this);
    if (QKeySequence(QKeySequence::FullScreen) != QKeySequence(Qt::Key_F11))
        viewToggleFullscreenAction->setShortcuts({ Qt::Key_F11, QKeySequence::FullScreen });
    else
        viewToggleFullscreenAction->setShortcuts({ Qt::Key_F11 });
    connect(viewToggleFullscreenAction, SIGNAL(triggered()), this, SLOT(viewToggleFullscreen()));
    myAddAction(viewToggleFullscreenAction, viewMenu, _qv);
    viewMenu->addSeparator();
    QAction* viewZoomInAction = new QAction("Zoom &in", this);
    viewZoomInAction->setShortcuts({ Qt::Key_Plus, QKeySequence::ZoomIn });
    connect(viewZoomInAction, SIGNAL(triggered()), this, SLOT(viewZoomIn()));
    myAddAction(viewZoomInAction, viewMenu, _qv);
    QAction* viewZoomOutAction = new QAction("Zoom &out", this);
    viewZoomOutAction->setShortcuts({ Qt::Key_Minus, QKeySequence::ZoomOut });
    connect(viewZoomOutAction, SIGNAL(triggered()), this, SLOT(viewZoomOut()));
    myAddAction(viewZoomOutAction, viewMenu, _qv);
    QAction* viewZoomResetAction = new QAction("&Reset zoom", this);
    viewZoomResetAction->setShortcuts({ Qt::Key_Equal });
    connect(viewZoomResetAction, SIGNAL(triggered()), this, SLOT(viewZoomReset()));
    myAddAction(viewZoomResetAction, viewMenu, _qv);
    QAction* viewRecenterAction = new QAction("Recenter view", this);
    viewRecenterAction->setShortcuts({ Qt::Key_Space });
    connect(viewRecenterAction, SIGNAL(triggered()), this, SLOT(viewRecenter()));
    myAddAction(viewRecenterAction, viewMenu, _qv);
    viewMenu->addSeparator();
    _viewToggleLinearInterpolationAction = new QAction("Toggle &linear interpolation for magnified views");
    _viewToggleLinearInterpolationAction->setCheckable(true);
    _viewToggleLinearInterpolationAction->setShortcuts({ Qt::Key_L });
    connect(_viewToggleLinearInterpolationAction, SIGNAL(triggered()), this, SLOT(viewToggleLinearInterpolation()));
    myAddAction(_viewToggleLinearInterpolationAction, viewMenu, _qv);
    _viewToggleGridAction = new QAction("Toggle &grid for magnified views");
    _viewToggleGridAction->setCheckable(true);
    _viewToggleGridAction->setShortcuts({ Qt::Key_G });
    connect(_viewToggleGridAction, SIGNAL(triggered()), this, SLOT(viewToggleGrid()));
    myAddAction(_viewToggleGridAction, viewMenu, _qv);

    QMenu* rangeMenu = menuBar()->addMenu("&Range");
    _rangeToggleOverlayAction = new QAction("Toggle histogram and visible range &overlay");
    _rangeToggleOverlayAction->setCheckable(true);
    _rangeToggleOverlayAction->setShortcuts({ Qt::Key_H });
    connect(_rangeToggleOverlayAction, SIGNAL(triggered()), this, SLOT(rangeToggleOverlay()));
    myAddAction(_rangeToggleOverlayAction, rangeMenu, _qv);
    rangeMenu->addSeparator();
    QAction* rangeDecLoAction = new QAction("Decrease lower bound of visible range", this);
    rangeDecLoAction->setShortcuts({ Qt::Key_BraceLeft });
    connect(rangeDecLoAction, SIGNAL(triggered()), this, SLOT(rangeDecLo()));
    myAddAction(rangeDecLoAction, rangeMenu, _qv);
    QAction* rangeIncLoAction = new QAction("Increase lower bound of visible range", this);
    rangeIncLoAction->setShortcuts({ Qt::Key_BraceRight });
    connect(rangeIncLoAction, SIGNAL(triggered()), this, SLOT(rangeIncLo()));
    myAddAction(rangeIncLoAction, rangeMenu, _qv);
    QAction* rangeDecHiAction = new QAction("Decrease upper bound of visible range", this);
    rangeDecHiAction->setShortcuts({ Qt::Key_BracketLeft });
    connect(rangeDecHiAction, SIGNAL(triggered()), this, SLOT(rangeDecHi()));
    myAddAction(rangeDecHiAction, rangeMenu, _qv);
    QAction* rangeIncHiAction = new QAction("Increase upper bound of visible range", this);
    rangeIncHiAction->setShortcuts({ Qt::Key_BracketRight });
    connect(rangeIncHiAction, SIGNAL(triggered()), this, SLOT(rangeIncHi()));
    myAddAction(rangeIncHiAction, rangeMenu, _qv);
    QAction* rangeShiftLeftAction = new QAction("Shift visible range to lower values", this);
    rangeShiftLeftAction->setShortcuts({ Qt::Key_ParenLeft });
    connect(rangeShiftLeftAction, SIGNAL(triggered()), this, SLOT(rangeShiftLeft()));
    myAddAction(rangeShiftLeftAction, rangeMenu, _qv);
    QAction* rangeShiftRightAction = new QAction("Shift visible range to higher values", this);
    rangeShiftRightAction->setShortcuts({ Qt::Key_ParenRight });
    connect(rangeShiftRightAction, SIGNAL(triggered()), this, SLOT(rangeShiftRight()));
    myAddAction(rangeShiftRightAction, rangeMenu, _qv);
    QAction* rangeResetAction = new QAction("Reset visible range", this);
    rangeResetAction->setShortcuts({ Qt::Key_Backslash });
    connect(rangeResetAction, SIGNAL(triggered()), this, SLOT(rangeReset()));
    myAddAction(rangeResetAction, rangeMenu, _qv);
    rangeMenu->addSeparator();
    _rangeDRRToggleAction = new QAction("&Toggle Dynamic Range Reduction (DRR; simple tone mapping)", this);
    _rangeDRRToggleAction->setCheckable(true);
    _rangeDRRToggleAction->setShortcuts({ Qt::Key_D });
    connect(_rangeDRRToggleAction, SIGNAL(triggered()), this, SLOT(rangeDRRToggle()));
    myAddAction(_rangeDRRToggleAction, rangeMenu, _qv);
    QAction* rangeDRRDecBrightnessAction = new QAction("&Decrease DRR brightness", this);
    rangeDRRDecBrightnessAction->setShortcuts({ Qt::Key_Comma });
    connect(rangeDRRDecBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRDecBrightness()));
    myAddAction(rangeDRRDecBrightnessAction, rangeMenu, _qv);
    QAction* rangeDRRIncBrightnessAction = new QAction("&Increase DRR brightness", this);
    rangeDRRIncBrightnessAction->setShortcuts({ Qt::Key_Period });
    connect(rangeDRRIncBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRIncBrightness()));
    myAddAction(rangeDRRIncBrightnessAction, rangeMenu, _qv);
    QAction* rangeDRRResetBrightnessAction = new QAction("&Reset DRR brightness", this);
    rangeDRRResetBrightnessAction->setShortcuts({ Qt::Key_Slash });
    connect(rangeDRRResetBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRResetBrightness()));
    myAddAction(rangeDRRResetBrightnessAction, rangeMenu, _qv);

    QMenu* colorMapMenu = menuBar()->addMenu("&Colormap");
    _colorMapToggleOverlayAction = new QAction("Toggle colormap overlay");
    _colorMapToggleOverlayAction->setCheckable(true);
    _colorMapToggleOverlayAction->setShortcuts({ Qt::Key_M });
    connect(_colorMapToggleOverlayAction, SIGNAL(triggered()), this, SLOT(colorMapToggleOverlay()));
    myAddAction(_colorMapToggleOverlayAction, colorMapMenu, _qv);
    colorMapMenu->addSeparator();
    QAction* colorMapDisableAction = new QAction("Disable color &map", this);
    colorMapDisableAction->setShortcuts({ Qt::Key_F4 });
    connect(colorMapDisableAction, SIGNAL(triggered()), this, SLOT(colorMapDisable()));
    myAddAction(colorMapDisableAction, colorMapMenu, _qv);
    QAction* colorMapCycleSequentialAction = new QAction("Enable next &sequential color map", this);
    colorMapCycleSequentialAction->setShortcuts({ Qt::Key_F5 });
    connect(colorMapCycleSequentialAction, SIGNAL(triggered()), this, SLOT(colorMapCycleSequential()));
    myAddAction(colorMapCycleSequentialAction, colorMapMenu, _qv);
    QAction* colorMapCycleDivergingAction = new QAction("Enable next d&iverging color map", this);
    colorMapCycleDivergingAction->setShortcuts({ Qt::Key_F6 });
    connect(colorMapCycleDivergingAction, SIGNAL(triggered()), this, SLOT(colorMapCycleDiverging()));
    myAddAction(colorMapCycleDivergingAction, colorMapMenu, _qv);
    QAction* colorMapQualitativeAction = new QAction("Enable &qualitative color map", this);
    colorMapQualitativeAction->setShortcuts({ Qt::Key_F7 });
    connect(colorMapQualitativeAction, SIGNAL(triggered()), this, SLOT(colorMapQualitative()));
    myAddAction(colorMapQualitativeAction, colorMapMenu, _qv);
    QAction* colorMapCustomAction = new QAction("Enable &custom color map (import from clipboard in image or CSV format)", this);
    colorMapCustomAction->setShortcuts({ Qt::Key_F8 });
    connect(colorMapCustomAction, SIGNAL(triggered()), this, SLOT(colorMapCustom()));
    myAddAction(colorMapCustomAction, colorMapMenu, _qv);

    QMenu* analysisMenu = menuBar()->addMenu("&Analysis");
    _analysisToggleApplyCurrentParametersToAllFilesAction = new QAction("Toggle &application of current parameters to all files", this);
    _analysisToggleApplyCurrentParametersToAllFilesAction->setCheckable(true);
    _analysisToggleApplyCurrentParametersToAllFilesAction->setShortcuts({ Qt::Key_A });
    connect(_analysisToggleApplyCurrentParametersToAllFilesAction, SIGNAL(triggered()), this, SLOT(analysisToggleApplyCurrentParametersToAllFiles()));
    myAddAction(_analysisToggleApplyCurrentParametersToAllFilesAction, analysisMenu, _qv);
    analysisMenu->addSeparator();
    _analysisToggleInfoAction = new QAction("Toggle &info overlay", this);
    _analysisToggleInfoAction->setShortcuts({ Qt::Key_I });
    _analysisToggleInfoAction->setCheckable(true);
    connect(_analysisToggleInfoAction, SIGNAL(triggered()), this, SLOT(analysisToggleInfo()));
    myAddAction(_analysisToggleInfoAction, analysisMenu, _qv);
    _analysisToggleStatisticsAction = new QAction("Toggle &statistics overlay", this);
    _analysisToggleStatisticsAction->setShortcuts({ Qt::Key_S });
    _analysisToggleStatisticsAction->setCheckable(true);
    connect(_analysisToggleStatisticsAction, SIGNAL(triggered()), this, SLOT(analysisToggleStatistics()));
    myAddAction(_analysisToggleStatisticsAction, analysisMenu, _qv);
    _analysisToggleValueAction = new QAction("Toggle &value inspection overlay", this);
    _analysisToggleValueAction->setShortcuts({ Qt::Key_V });
    _analysisToggleValueAction->setCheckable(true);
    connect(_analysisToggleValueAction, SIGNAL(triggered()), this, SLOT(analysisToggleValue()));
    myAddAction(_analysisToggleValueAction, analysisMenu, _qv);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    _helpToggleOverlayAction = new QAction("Toggle &help overlay", this);
    _helpToggleOverlayAction->setCheckable(true);
    if (QKeySequence(QKeySequence::HelpContents) != QKeySequence(Qt::Key_F1))
        _helpToggleOverlayAction->setShortcuts({ Qt::Key_F1, QKeySequence::HelpContents });
    else
        _helpToggleOverlayAction->setShortcuts({ Qt::Key_F1 });
    connect(_helpToggleOverlayAction, SIGNAL(triggered()), this, SLOT(helpToggleOverlay()));
    myAddAction(_helpToggleOverlayAction, helpMenu, _qv);
    helpMenu->addSeparator();
    QAction* helpAboutAction = new QAction("&About");
    connect(helpAboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
    myAddAction(helpAboutAction, helpMenu, _qv);

    connect(_qv, SIGNAL(toggleFullscreen()), this, SLOT(viewToggleFullscreen()));
    connect(_qv, SIGNAL(parametersChanged()), this, SLOT(updateFromParameters()));
    updateFromParameters();
    setCentralWidget(_qv);
}

void Gui::fileOpen()
{
    _qv->openFile();
}

void Gui::fileClose()
{
    _qv->closeFile();
}

void Gui::fileReload()
{
    _qv->reloadFile();
}

void Gui::fileSaveCurrentView()
{
    _qv->saveView(false);
}

void Gui::fileSaveView()
{
    _qv->saveView(true);
}

void Gui::fileCopyCurrentView()
{
    _qv->copyView(false);
}

void Gui::fileCopyView()
{
    _qv->copyView(true);
}

void Gui::fileNext()
{
    _qv->adjustFileIndex(+1);
}

void Gui::filePrev()
{
    _qv->adjustFileIndex(-1);
}

void Gui::fileNext10()
{
    _qv->adjustFileIndex(+10);
}

void Gui::filePrev10()
{
    _qv->adjustFileIndex(-10);
}

void Gui::fileNext100()
{
    _qv->adjustFileIndex(+100);
}

void Gui::filePrev100()
{
    _qv->adjustFileIndex(-100);
}

void Gui::frameNext()
{
    _qv->adjustFrameIndex(+1);
}

void Gui::framePrev()
{
    _qv->adjustFrameIndex(-1);
}

void Gui::frameNext10()
{
    _qv->adjustFrameIndex(+10);
}

void Gui::framePrev10()
{
    _qv->adjustFrameIndex(-10);
}

void Gui::frameNext100()
{
    _qv->adjustFrameIndex(+100);
}

void Gui::framePrev100()
{
    _qv->adjustFrameIndex(-100);
}

void Gui::channelColor()
{
    _qv->setChannelIndex(ColorChannelIndex);
}

void Gui::channel0()
{
    _qv->setChannelIndex(0);
}

void Gui::channel1()
{
    _qv->setChannelIndex(1);
}

void Gui::channel2()
{
    _qv->setChannelIndex(2);
}

void Gui::channel3()
{
    _qv->setChannelIndex(3);
}

void Gui::channel4()
{
    _qv->setChannelIndex(4);
}

void Gui::channel5()
{
    _qv->setChannelIndex(5);
}

void Gui::channel6()
{
    _qv->setChannelIndex(6);
}

void Gui::channel7()
{
    _qv->setChannelIndex(7);
}

void Gui::channel8()
{
    _qv->setChannelIndex(8);
}

void Gui::channel9()
{
    _qv->setChannelIndex(9);
}

void Gui::viewToggleFullscreen()
{
    if (windowState() & Qt::WindowFullScreen) {
        showNormal();
        menuBar()->show();
        activateWindow();
        setFocus();
    } else {
        menuBar()->hide();
        showFullScreen();
        activateWindow();
        setFocus();
    }
}

void Gui::viewZoomIn()
{
    _qv->adjustZoom(+1);
}

void Gui::viewZoomOut()
{
    _qv->adjustZoom(-1);
}

void Gui::viewZoomReset()
{
    _qv->resetZoom();
}

void Gui::viewRecenter()
{
    _qv->recenter();
}

void Gui::viewToggleLinearInterpolation()
{
    _qv->toggleLinearInterpolation();
}

void Gui::viewToggleGrid()
{
    _qv->toggleGrid();
}

void Gui::rangeToggleOverlay()
{
    _qv->toggleOverlayHistogram();
}

void Gui::rangeDecLo()
{
    _qv->adjustVisInterval(-1, 0);
}

void Gui::rangeIncLo()
{
    _qv->adjustVisInterval(+1, 0);
}

void Gui::rangeDecHi()
{
    _qv->adjustVisInterval(0, -1);
}

void Gui::rangeIncHi()
{
    _qv->adjustVisInterval(0, +1);
}

void Gui::rangeShiftLeft()
{
    _qv->adjustVisInterval(-1, -1);
}

void Gui::rangeShiftRight()
{
    _qv->adjustVisInterval(+1, +1);
}

void Gui::rangeReset()
{
    _qv->resetVisInterval();
}

void Gui::rangeDRRToggle()
{
    _qv->toggleDRR();
}

void Gui::rangeDRRDecBrightness()
{
    _qv->adjustDRRBrightness(-1);
}

void Gui::rangeDRRIncBrightness()
{
    _qv->adjustDRRBrightness(+1);
}

void Gui::rangeDRRResetBrightness()
{
    _qv->adjustDRRBrightness(0);
}

void Gui::colorMapToggleOverlay()
{
    _qv->toggleOverlayColormap();
}

void Gui::colorMapDisable()
{
    _qv->changeColorMap(ColorMapNone);
}

void Gui::colorMapCycleSequential()
{
    _qv->changeColorMap(ColorMapSequential);
}

void Gui::colorMapCycleDiverging()
{
    _qv->changeColorMap(ColorMapDiverging);
}

void Gui::colorMapQualitative()
{
    _qv->changeColorMap(ColorMapQualitative);
}

void Gui::colorMapCustom()
{
    _qv->changeColorMap(ColorMapCustom);
}

void Gui::analysisToggleApplyCurrentParametersToAllFiles()
{
    _qv->toggleApplyCurrentParametersToAllFiles();
}

void Gui::analysisToggleInfo()
{
    _qv->toggleOverlayInfo();
}

void Gui::analysisToggleStatistics()
{
    _qv->toggleOverlayStatistics();
}

void Gui::analysisToggleValue()
{
    _qv->toggleOverlayValue();
}

void Gui::helpToggleOverlay()
{
    _qv->toggleOverlayHelp();
}

void Gui::helpAbout()
{
    QMessageBox::about(this, "About qv",
                QString("<p>qv version %1<br>"
                    "   <a href=\"https://marlam.de/qv\">https://marlam.de/qv</a></p>"
                    "<p>Copyright (C) 2020<br>"
                    "   <a href=\"https://www.cg.informatik.uni-siegen.de/\">"
                    "   Computer Graphics Group, University of Siegen</a>.<br>"
                    "   Written by <a href=\"https://marlam.de/\">Martin Lambers</a>.<br>"
                    "   This is free software under the terms of the "
                    "<a href=\"https://www.debian.org/legal/licenses/mit\">MIT/Expat License</a>. "
                    "   There is NO WARRANTY, to the extent permitted by law."
                    "</p>").arg(QV_VERSION));
}

void Gui::updateFromParameters()
{
    Parameters p;
    if (_set.currentParameters()) {
        p = *(_set.currentParameters());
    }
    _viewToggleLinearInterpolationAction->setChecked(p.magInterpolation);
    _viewToggleGridAction->setChecked(p.magGrid);
    _rangeToggleOverlayAction->setChecked(_qv->overlayHistogramActive);
    _rangeDRRToggleAction->setChecked(p.dynamicRangeReduction);
    _colorMapToggleOverlayAction->setChecked(_qv->overlayColorMapActive);
    _analysisToggleApplyCurrentParametersToAllFilesAction->setChecked(_set.applyCurrentParametersToAllFiles());
    _analysisToggleInfoAction->setChecked(_qv->overlayInfoActive);
    _analysisToggleStatisticsAction->setChecked(_qv->overlayStatisticActive);
    _analysisToggleValueAction->setChecked(_qv->overlayValueActive);
    _helpToggleOverlayAction->setChecked(_qv->overlayHelpActive);
}
