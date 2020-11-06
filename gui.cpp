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

#include "gui.hpp"


Gui::Gui(Set& set) : QMainWindow(),
    _set(set),
    _qv(new QV(_set, this))
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* fileOpenAction = new QAction("&Open...", this);
    fileOpenAction->setShortcut(QKeySequence::Open);
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
    fileMenu->addAction(fileOpenAction);
    QAction* fileReloadAction = new QAction("&Reload", this);
    //fileReloadAction->setShortcut(QKeySequence::Refresh); // F5 conflicts with color map
    connect(fileReloadAction, SIGNAL(triggered()), this, SLOT(fileReload()));
    fileMenu->addAction(fileReloadAction);
    QAction* fileCloseAction = new QAction("&Close", this);
    fileCloseAction->setShortcut(QKeySequence::Close);
    connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(fileClose()));
    fileMenu->addAction(fileCloseAction);
    fileMenu->addSeparator();
    QAction* fileSaveCurrentViewAction = new QAction("Save current view...", this);
    //fileSaveCurrentViewAction->setShortcut(QKeySequence::Save);
    connect(fileSaveCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileSaveCurrentView()));
    fileMenu->addAction(fileSaveCurrentViewAction);
    QAction* fileSaveViewAction = new QAction("&Save 1:1 view...", this);
    fileSaveViewAction->setShortcut(QKeySequence::Save);
    connect(fileSaveViewAction, SIGNAL(triggered()), this, SLOT(fileSaveView()));
    fileMenu->addAction(fileSaveViewAction);
    fileMenu->addSeparator();
    QAction* fileCopyCurrentViewAction = new QAction("Copy current view...", this);
    //fileCopyCurrentViewAction->setShortcut(QKeySequence::Copy);
    connect(fileCopyCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileCopyCurrentView()));
    fileMenu->addAction(fileCopyCurrentViewAction);
    QAction* fileCopyViewAction = new QAction("&Copy 1:1 view...", this);
    fileCopyViewAction->setShortcut(QKeySequence::Copy);
    connect(fileCopyViewAction, SIGNAL(triggered()), this, SLOT(fileCopyView()));
    fileMenu->addAction(fileCopyViewAction);
    fileMenu->addSeparator();
    QAction* fileNextAction = new QAction("Jump to next file", this);
    connect(fileNextAction, SIGNAL(triggered()), this, SLOT(fileNext()));
    fileMenu->addAction(fileNextAction);
    QAction* filePrevAction = new QAction("Jump to previous file", this);
    connect(filePrevAction, SIGNAL(triggered()), this, SLOT(filePrev()));
    fileMenu->addAction(filePrevAction);
    QAction* fileNext10Action = new QAction("Jump 10 files forward", this);
    connect(fileNext10Action, SIGNAL(triggered()), this, SLOT(fileNext10()));
    fileMenu->addAction(fileNext10Action);
    QAction* filePrev10Action = new QAction("Jump 10 files backward", this);
    connect(filePrev10Action, SIGNAL(triggered()), this, SLOT(filePrev10()));
    fileMenu->addAction(filePrev10Action);
    QAction* fileNext100Action = new QAction("Jump 100 files forward", this);
    connect(fileNext100Action, SIGNAL(triggered()), this, SLOT(fileNext100()));
    fileMenu->addAction(fileNext100Action);
    QAction* filePrev100Action = new QAction("Jump 100 files backward", this);
    connect(filePrev100Action, SIGNAL(triggered()), this, SLOT(filePrev100()));
    fileMenu->addAction(filePrev100Action);
    fileMenu->addSeparator();
    QAction *fileQuitAction = new QAction("&Quit", this);
    fileQuitAction->setShortcut(QKeySequence::Quit);
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(fileQuitAction);

    QMenu* frameMenu = menuBar()->addMenu("F&rame");
    QAction* frameNextAction = new QAction("Jump to next frame in this file", this);
    connect(frameNextAction, SIGNAL(triggered()), this, SLOT(frameNext()));
    frameMenu->addAction(frameNextAction);
    QAction* framePrevAction = new QAction("Jump to previous frame in this file", this);
    connect(framePrevAction, SIGNAL(triggered()), this, SLOT(framePrev()));
    frameMenu->addAction(framePrevAction);
    QAction* frameNext10Action = new QAction("Jump 10 frames forward in this file", this);
    connect(frameNext10Action, SIGNAL(triggered()), this, SLOT(frameNext10()));
    frameMenu->addAction(frameNext10Action);
    QAction* framePrev10Action = new QAction("Jump 10 frames backward in this file", this);
    connect(framePrev10Action, SIGNAL(triggered()), this, SLOT(framePrev10()));
    frameMenu->addAction(framePrev10Action);
    QAction* frameNext100Action = new QAction("Jump 100 frames forward in this file", this);
    connect(frameNext100Action, SIGNAL(triggered()), this, SLOT(frameNext100()));
    frameMenu->addAction(frameNext100Action);
    QAction* framePrev100Action = new QAction("Jump 100 frames backward in this file", this);
    connect(framePrev100Action, SIGNAL(triggered()), this, SLOT(framePrev100()));
    frameMenu->addAction(framePrev100Action);

    QMenu* channelMenu = menuBar()->addMenu("&Channel");
    QAction* channelColorAction = new QAction("Show color channels of this frame", this);
    connect(channelColorAction, SIGNAL(triggered()), this, SLOT(channelColor()));
    channelMenu->addAction(channelColorAction);
    QAction* channel0Action = new QAction("Show channel 0 of this frame", this);
    connect(channel0Action, SIGNAL(triggered()), this, SLOT(channel0()));
    channelMenu->addAction(channel0Action);
    QAction* channel1Action = new QAction("Show channel 1 of this frame", this);
    connect(channel1Action, SIGNAL(triggered()), this, SLOT(channel1()));
    channelMenu->addAction(channel1Action);
    QAction* channel2Action = new QAction("Show channel 2 of this frame", this);
    connect(channel2Action, SIGNAL(triggered()), this, SLOT(channel2()));
    channelMenu->addAction(channel2Action);
    QAction* channel3Action = new QAction("Show channel 3 of this frame", this);
    connect(channel3Action, SIGNAL(triggered()), this, SLOT(channel3()));
    channelMenu->addAction(channel3Action);
    QAction* channel4Action = new QAction("Show channel 4 of this frame", this);
    connect(channel4Action, SIGNAL(triggered()), this, SLOT(channel4()));
    channelMenu->addAction(channel4Action);
    QAction* channel5Action = new QAction("Show channel 5 of this frame", this);
    connect(channel5Action, SIGNAL(triggered()), this, SLOT(channel5()));
    channelMenu->addAction(channel5Action);
    QAction* channel6Action = new QAction("Show channel 6 of this frame", this);
    connect(channel6Action, SIGNAL(triggered()), this, SLOT(channel6()));
    channelMenu->addAction(channel6Action);
    QAction* channel7Action = new QAction("Show channel 7 of this frame", this);
    connect(channel7Action, SIGNAL(triggered()), this, SLOT(channel7()));
    channelMenu->addAction(channel7Action);
    QAction* channel8Action = new QAction("Show channel 8 of this frame", this);
    connect(channel8Action, SIGNAL(triggered()), this, SLOT(channel8()));
    channelMenu->addAction(channel8Action);
    QAction* channel9Action = new QAction("Show channel 9 of this frame", this);
    connect(channel9Action, SIGNAL(triggered()), this, SLOT(channel9()));
    channelMenu->addAction(channel9Action);

    QMenu* viewMenu = menuBar()->addMenu("&View");
    QAction* viewToggleFullscreenAction = new QAction("Toggle &Fullscreen", this);
    connect(viewToggleFullscreenAction, SIGNAL(triggered()), this, SLOT(viewToggleFullscreen()));
    viewMenu->addAction(viewToggleFullscreenAction);
    viewMenu->addSeparator();
    QAction* viewZoomInAction = new QAction("Zoom &in", this);
    connect(viewZoomInAction, SIGNAL(triggered()), this, SLOT(viewZoomIn()));
    viewMenu->addAction(viewZoomInAction);
    QAction* viewZoomOutAction = new QAction("Zoom &out", this);
    connect(viewZoomOutAction, SIGNAL(triggered()), this, SLOT(viewZoomOut()));
    viewMenu->addAction(viewZoomOutAction);
    QAction* viewZoomResetAction = new QAction("&Reset zoom", this);
    connect(viewZoomResetAction, SIGNAL(triggered()), this, SLOT(viewZoomReset()));
    viewMenu->addAction(viewZoomResetAction);
    QAction* viewRecenterAction = new QAction("Recenter view", this);
    connect(viewRecenterAction, SIGNAL(triggered()), this, SLOT(viewRecenter()));
    viewMenu->addAction(viewRecenterAction);
    viewMenu->addSeparator();
    _viewToggleLinearInterpolationAction = new QAction("Toggle &linear interpolation for magnified views");
    _viewToggleLinearInterpolationAction->setCheckable(true);
    connect(_viewToggleLinearInterpolationAction, SIGNAL(triggered()), this, SLOT(viewToggleLinearInterpolation()));
    viewMenu->addAction(_viewToggleLinearInterpolationAction);
    _viewToggleGridAction = new QAction("Toggle &grid for magnified views");
    _viewToggleGridAction->setCheckable(true);
    connect(_viewToggleGridAction, SIGNAL(triggered()), this, SLOT(viewToggleGrid()));
    viewMenu->addAction(_viewToggleGridAction);

    QMenu* rangeMenu = menuBar()->addMenu("&Range");
    QAction* rangeDecLoAction = new QAction("Decrease lower bound of visible range", this);
    connect(rangeDecLoAction, SIGNAL(triggered()), this, SLOT(rangeDecLo()));
    rangeMenu->addAction(rangeDecLoAction);
    QAction* rangeIncLoAction = new QAction("Increase lower bound of visible range", this);
    connect(rangeIncLoAction, SIGNAL(triggered()), this, SLOT(rangeIncLo()));
    rangeMenu->addAction(rangeIncLoAction);
    QAction* rangeDecHiAction = new QAction("Decrease upper bound of visible range", this);
    connect(rangeDecHiAction, SIGNAL(triggered()), this, SLOT(rangeDecHi()));
    rangeMenu->addAction(rangeDecHiAction);
    QAction* rangeIncHiAction = new QAction("Increase upper bound of visible range", this);
    connect(rangeIncHiAction, SIGNAL(triggered()), this, SLOT(rangeIncHi()));
    rangeMenu->addAction(rangeIncHiAction);
    QAction* rangeShiftLeftAction = new QAction("Shift visible range to lower values", this);
    connect(rangeShiftLeftAction, SIGNAL(triggered()), this, SLOT(rangeShiftLeft()));
    rangeMenu->addAction(rangeShiftLeftAction);
    QAction* rangeShiftRightAction = new QAction("Shift visible range to higher values", this);
    connect(rangeShiftRightAction, SIGNAL(triggered()), this, SLOT(rangeShiftRight()));
    rangeMenu->addAction(rangeShiftRightAction);
    QAction* rangeResetAction = new QAction("Reset visible range", this);
    connect(rangeResetAction, SIGNAL(triggered()), this, SLOT(rangeReset()));
    rangeMenu->addAction(rangeResetAction);
    rangeMenu->addSeparator();
    _rangeDRRToggleAction = new QAction("Toggle Dynamic Range Reduction (DRR; simple tone mapping)", this);
    _rangeDRRToggleAction->setCheckable(true);
    connect(_rangeDRRToggleAction, SIGNAL(triggered()), this, SLOT(rangeDRRToggle()));
    rangeMenu->addAction(_rangeDRRToggleAction);
    QAction* rangeDRRDecBrightnessAction = new QAction("Decrease DRR brightness", this);
    connect(rangeDRRDecBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRDecBrightness()));
    rangeMenu->addAction(rangeDRRDecBrightnessAction);
    QAction* rangeDRRIncBrightnessAction = new QAction("Increase DRR brightness", this);
    connect(rangeDRRIncBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRIncBrightness()));
    rangeMenu->addAction(rangeDRRIncBrightnessAction);
    QAction* rangeDRRResetBrightnessAction = new QAction("Reset DRR brightness", this);
    connect(rangeDRRResetBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRResetBrightness()));
    rangeMenu->addAction(rangeDRRResetBrightnessAction);

    QMenu* colorMapMenu = menuBar()->addMenu("&Colormap");
    QAction* colorMapDisableAction = new QAction("Disable color map", this);
    connect(colorMapDisableAction, SIGNAL(triggered()), this, SLOT(colorMapDisable()));
    colorMapMenu->addAction(colorMapDisableAction);
    QAction* colorMapCycleSequentialAction = new QAction("Enable next sequential color map", this);
    connect(colorMapCycleSequentialAction, SIGNAL(triggered()), this, SLOT(colorMapCycleSequential()));
    colorMapMenu->addAction(colorMapCycleSequentialAction);
    QAction* colorMapCycleDivergingAction = new QAction("Enable next diverging color map", this);
    connect(colorMapCycleDivergingAction, SIGNAL(triggered()), this, SLOT(colorMapCycleDiverging()));
    colorMapMenu->addAction(colorMapCycleDivergingAction);
    QAction* colorMapQualitativeAction = new QAction("Enable qualitative color map", this);
    connect(colorMapQualitativeAction, SIGNAL(triggered()), this, SLOT(colorMapQualitative()));
    colorMapMenu->addAction(colorMapQualitativeAction);
    QAction* colorMapCustomAction = new QAction("Enable custom color map (import from clipboard in CSV format)", this);
    connect(colorMapCustomAction, SIGNAL(triggered()), this, SLOT(colorMapCustom()));
    colorMapMenu->addAction(colorMapCustomAction);

    connect(_qv, SIGNAL(toggleFullscreen()), this, SLOT(viewToggleFullscreen()));
    connect(_qv, SIGNAL(parametersChanged()), this, SLOT(updateFromParameters()));
    updateFromParameters();
    setCentralWidget(_qv);
    _qv->setFocus();
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
    } else {
        showFullScreen();
        menuBar()->hide();
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

void Gui::updateFromParameters()
{
    Parameters p;
    if (_set.currentParameters()) {
        p = *(_set.currentParameters());
    }
    _viewToggleLinearInterpolationAction->setChecked(p.magInterpolation);
    _viewToggleGridAction->setChecked(p.magGrid);
    _rangeDRRToggleAction->setChecked(p.dynamicRangeReduction);
}
