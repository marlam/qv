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
#include <QContextMenuEvent>
#include <QActionGroup>

#include "gui.hpp"
#include "version.hpp"


QMenu* Gui::addQVMenu(const QString& title)
{
    QMenu* menu = menuBar()->addMenu(title);
    _contextMenu->addMenu(menu);
    return menu;
}

void Gui::addQVAction(QAction* action, QMenu* menu)
{
    menu->addAction(action);
    _qv->addAction(action);
}

Gui::Gui(Set& set) : QMainWindow(),
    _set(set),
    _qv(new QV(_set, this)),
    _contextMenu(new QMenu(this))
{
    QMenu* fileMenu = addQVMenu("&File");
    _fileOpenAction = new QAction("&Open file(s)...", this);
    _fileOpenAction->setShortcuts({ Qt::Key_O, QKeySequence::Open });
    connect(_fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
    addQVAction(_fileOpenAction, fileMenu);
    _fileReloadAction = new QAction("&Reload current file", this);
    if (QKeySequence(QKeySequence::Refresh) != QKeySequence(Qt::Key_F5))
        _fileReloadAction->setShortcuts({ Qt::Key_R, QKeySequence::Refresh });
    else
        _fileReloadAction->setShortcuts({ Qt::Key_R });
    connect(_fileReloadAction, SIGNAL(triggered()), this, SLOT(fileReload()));
    addQVAction(_fileReloadAction, fileMenu);
    _fileCloseAction = new QAction("&Close current file", this);
    _fileCloseAction->setShortcuts({ Qt::Key_W, QKeySequence::Close });
    connect(_fileCloseAction, SIGNAL(triggered()), this, SLOT(fileClose()));
    addQVAction(_fileCloseAction, fileMenu);
    fileMenu->addSeparator();
    _fileSaveCurrentViewAction = new QAction("Save current view...", this);
    _fileSaveCurrentViewAction->setShortcuts({ Qt::Key_F2 });
    connect(_fileSaveCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileSaveCurrentView()));
    addQVAction(_fileSaveCurrentViewAction, fileMenu);
    _fileSaveViewAction = new QAction("&Save 1:1 view...", this);
    _fileSaveViewAction->setShortcuts({ Qt::Key_F3, QKeySequence::Save });
    connect(_fileSaveViewAction, SIGNAL(triggered()), this, SLOT(fileSaveView()));
    addQVAction(_fileSaveViewAction, fileMenu);
    fileMenu->addSeparator();
    _fileCopyCurrentViewAction = new QAction("Copy current view...", this);
    _fileCopyCurrentViewAction->setShortcuts({ Qt::Key_F9 });
    connect(_fileCopyCurrentViewAction, SIGNAL(triggered()), this, SLOT(fileCopyCurrentView()));
    addQVAction(_fileCopyCurrentViewAction, fileMenu);
    _fileCopyViewAction = new QAction("&Copy 1:1 view...", this);
    _fileCopyViewAction->setShortcuts({ Qt::Key_F10, QKeySequence::Copy });
    connect(_fileCopyViewAction, SIGNAL(triggered()), this, SLOT(fileCopyView()));
    addQVAction(_fileCopyViewAction, fileMenu);
    fileMenu->addSeparator();
    _fileNextAction = new QAction("Jump to next file", this);
    _fileNextAction->setShortcuts({ Qt::Key_Right });
    connect(_fileNextAction, SIGNAL(triggered()), this, SLOT(fileNext()));
    addQVAction(_fileNextAction, fileMenu);
    _filePrevAction = new QAction("Jump to previous file", this);
    _filePrevAction->setShortcuts({ Qt::Key_Left });
    connect(_filePrevAction, SIGNAL(triggered()), this, SLOT(filePrev()));
    addQVAction(_filePrevAction, fileMenu);
    _fileNext10Action = new QAction("Jump 10 files forward", this);
    _fileNext10Action->setShortcuts({ Qt::Key_Down });
    connect(_fileNext10Action, SIGNAL(triggered()), this, SLOT(fileNext10()));
    addQVAction(_fileNext10Action, fileMenu);
    _filePrev10Action = new QAction("Jump 10 files backward", this);
    _filePrev10Action->setShortcuts({ Qt::Key_Up });
    connect(_filePrev10Action, SIGNAL(triggered()), this, SLOT(filePrev10()));
    addQVAction(_filePrev10Action, fileMenu);
    _fileNext100Action = new QAction("Jump 100 files forward", this);
    _fileNext100Action->setShortcuts({ Qt::Key_PageDown });
    connect(_fileNext100Action, SIGNAL(triggered()), this, SLOT(fileNext100()));
    addQVAction(_fileNext100Action, fileMenu);
    _filePrev100Action = new QAction("Jump 100 files backward", this);
    _filePrev100Action->setShortcuts({ Qt::Key_PageUp });
    connect(_filePrev100Action, SIGNAL(triggered()), this, SLOT(filePrev100()));
    addQVAction(_filePrev100Action, fileMenu);
    fileMenu->addSeparator();
    _fileQuitAction = new QAction("&Quit", this);
    _fileQuitAction->setShortcuts({ Qt::Key_Q, QKeySequence::Quit });
    connect(_fileQuitAction, SIGNAL(triggered()), this, SLOT(fileQuit()));
    addQVAction(_fileQuitAction, fileMenu);

    QMenu* frameMenu = addQVMenu("F&rame");
    _frameToggleInfoAction = new QAction("Toggle &info overlay", this);
    _frameToggleInfoAction->setShortcuts({ Qt::Key_I });
    _frameToggleInfoAction->setCheckable(true);
    connect(_frameToggleInfoAction, SIGNAL(triggered()), this, SLOT(frameToggleInfo()));
    addQVAction(_frameToggleInfoAction, frameMenu);
    _frameToggleValueAction = new QAction("Toggle &value inspection overlay", this);
    _frameToggleValueAction->setShortcuts({ Qt::Key_V });
    _frameToggleValueAction->setCheckable(true);
    connect(_frameToggleValueAction, SIGNAL(triggered()), this, SLOT(frameToggleValue()));
    addQVAction(_frameToggleValueAction, frameMenu);
    frameMenu->addSeparator();
    _frameNextAction = new QAction("Jump to next frame in this file", this);
    _frameNextAction->setShortcuts({ Qt::Key_Right + Qt::ShiftModifier });
    connect(_frameNextAction, SIGNAL(triggered()), this, SLOT(frameNext()));
    addQVAction(_frameNextAction, frameMenu);
    _framePrevAction = new QAction("Jump to previous frame in this file", this);
    _framePrevAction->setShortcuts({ Qt::Key_Left + Qt::ShiftModifier });
    connect(_framePrevAction, SIGNAL(triggered()), this, SLOT(framePrev()));
    addQVAction(_framePrevAction, frameMenu);
    _frameNext10Action = new QAction("Jump 10 frames forward in this file", this);
    _frameNext10Action->setShortcuts({ Qt::Key_Down + Qt::ShiftModifier });
    connect(_frameNext10Action, SIGNAL(triggered()), this, SLOT(frameNext10()));
    addQVAction(_frameNext10Action, frameMenu);
    _framePrev10Action = new QAction("Jump 10 frames backward in this file", this);
    _framePrev10Action->setShortcuts({ Qt::Key_Up + Qt::ShiftModifier });
    connect(_framePrev10Action, SIGNAL(triggered()), this, SLOT(framePrev10()));
    addQVAction(_framePrev10Action, frameMenu);
    _frameNext100Action = new QAction("Jump 100 frames forward in this file", this);
    _frameNext100Action->setShortcuts({ Qt::Key_PageDown + Qt::ShiftModifier });
    connect(_frameNext100Action, SIGNAL(triggered()), this, SLOT(frameNext100()));
    addQVAction(_frameNext100Action, frameMenu);
    _framePrev100Action = new QAction("Jump 100 frames backward in this file", this);
    _framePrev100Action->setShortcuts({ Qt::Key_PageUp + Qt::ShiftModifier });
    connect(_framePrev100Action, SIGNAL(triggered()), this, SLOT(framePrev100()));
    addQVAction(_framePrev100Action, frameMenu);

    QMenu* channelMenu = addQVMenu("&Channel");
    _channelToggleStatisticsAction = new QAction("Toggle &statistics overlay", this);
    _channelToggleStatisticsAction->setShortcuts({ Qt::Key_S });
    _channelToggleStatisticsAction->setCheckable(true);
    connect(_channelToggleStatisticsAction, SIGNAL(triggered()), this, SLOT(channelToggleStatistics()));
    addQVAction(_channelToggleStatisticsAction, channelMenu);
    channelMenu->addSeparator();
    _channelColorAction = new QAction("Show color channels of this frame", this);
    _channelColorAction->setShortcuts({Qt::Key_C });
    _channelColorAction->setCheckable(true);
    connect(_channelColorAction, SIGNAL(triggered()), this, SLOT(channelColor()));
    addQVAction(_channelColorAction, channelMenu);
    _channel0Action = new QAction("Show channel 0 of this frame", this);
    _channel0Action->setShortcuts({Qt::Key_0 });
    _channel0Action->setCheckable(true);
    connect(_channelColorAction, SIGNAL(triggered()), this, SLOT(channelColor()));
    connect(_channel0Action, SIGNAL(triggered()), this, SLOT(channel0()));
    addQVAction(_channel0Action, channelMenu);
    _channel1Action = new QAction("Show channel 1 of this frame", this);
    _channel1Action->setShortcuts({Qt::Key_1 });
    _channel1Action->setCheckable(true);
    connect(_channel1Action, SIGNAL(triggered()), this, SLOT(channel1()));
    addQVAction(_channel1Action, channelMenu);
    _channel2Action = new QAction("Show channel 2 of this frame", this);
    _channel2Action->setShortcuts({Qt::Key_2 });
    _channel2Action->setCheckable(true);
    connect(_channel2Action, SIGNAL(triggered()), this, SLOT(channel2()));
    addQVAction(_channel2Action, channelMenu);
    _channel3Action = new QAction("Show channel 3 of this frame", this);
    _channel3Action->setShortcuts({Qt::Key_3 });
    _channel3Action->setCheckable(true);
    connect(_channel3Action, SIGNAL(triggered()), this, SLOT(channel3()));
    addQVAction(_channel3Action, channelMenu);
    _channel4Action = new QAction("Show channel 4 of this frame", this);
    _channel4Action->setShortcuts({Qt::Key_4 });
    _channel4Action->setCheckable(true);
    connect(_channel4Action, SIGNAL(triggered()), this, SLOT(channel4()));
    addQVAction(_channel4Action, channelMenu);
    _channel5Action = new QAction("Show channel 5 of this frame", this);
    _channel5Action->setShortcuts({Qt::Key_5 });
    _channel5Action->setCheckable(true);
    connect(_channel5Action, SIGNAL(triggered()), this, SLOT(channel5()));
    addQVAction(_channel5Action, channelMenu);
    _channel6Action = new QAction("Show channel 6 of this frame", this);
    _channel6Action->setShortcuts({Qt::Key_6 });
    _channel6Action->setCheckable(true);
    connect(_channel6Action, SIGNAL(triggered()), this, SLOT(channel6()));
    addQVAction(_channel6Action, channelMenu);
    _channel7Action = new QAction("Show channel 7 of this frame", this);
    _channel7Action->setShortcuts({Qt::Key_7 });
    _channel7Action->setCheckable(true);
    connect(_channel7Action, SIGNAL(triggered()), this, SLOT(channel7()));
    addQVAction(_channel7Action, channelMenu);
    _channel8Action = new QAction("Show channel 8 of this frame", this);
    _channel8Action->setShortcuts({Qt::Key_8 });
    _channel8Action->setCheckable(true);
    connect(_channel8Action, SIGNAL(triggered()), this, SLOT(channel8()));
    addQVAction(_channel8Action, channelMenu);
    _channel9Action = new QAction("Show channel 9 of this frame", this);
    _channel9Action->setShortcuts({Qt::Key_9 });
    _channel9Action->setCheckable(true);
    connect(_channel9Action, SIGNAL(triggered()), this, SLOT(channel9()));
    addQVAction(_channel9Action, channelMenu);
    QActionGroup* channelSelectionGroup = new QActionGroup(this);
    channelSelectionGroup->addAction(_channelColorAction);
    channelSelectionGroup->addAction(_channel0Action);
    channelSelectionGroup->addAction(_channel1Action);
    channelSelectionGroup->addAction(_channel2Action);
    channelSelectionGroup->addAction(_channel3Action);
    channelSelectionGroup->addAction(_channel4Action);
    channelSelectionGroup->addAction(_channel5Action);
    channelSelectionGroup->addAction(_channel6Action);
    channelSelectionGroup->addAction(_channel7Action);
    channelSelectionGroup->addAction(_channel8Action);
    channelSelectionGroup->addAction(_channel9Action);

    QMenu* rangeMenu = addQVMenu("&Range");
    _rangeToggleOverlayAction = new QAction("Toggle histogram and visible range &overlay");
    _rangeToggleOverlayAction->setCheckable(true);
    _rangeToggleOverlayAction->setShortcuts({ Qt::Key_H });
    connect(_rangeToggleOverlayAction, SIGNAL(triggered()), this, SLOT(rangeToggleOverlay()));
    addQVAction(_rangeToggleOverlayAction, rangeMenu);
    _rangeDecLoAction = new QAction("Decrease lower bound of visible range", this);
    _rangeDecLoAction->setShortcuts({ Qt::Key_BraceLeft });
    connect(_rangeDecLoAction, SIGNAL(triggered()), this, SLOT(rangeDecLo()));
    addQVAction(_rangeDecLoAction, rangeMenu);
    _rangeIncLoAction = new QAction("Increase lower bound of visible range", this);
    _rangeIncLoAction->setShortcuts({ Qt::Key_BraceRight });
    connect(_rangeIncLoAction, SIGNAL(triggered()), this, SLOT(rangeIncLo()));
    addQVAction(_rangeIncLoAction, rangeMenu);
    _rangeDecHiAction = new QAction("Decrease upper bound of visible range", this);
    _rangeDecHiAction->setShortcuts({ Qt::Key_BracketLeft });
    connect(_rangeDecHiAction, SIGNAL(triggered()), this, SLOT(rangeDecHi()));
    addQVAction(_rangeDecHiAction, rangeMenu);
    _rangeIncHiAction = new QAction("Increase upper bound of visible range", this);
    _rangeIncHiAction->setShortcuts({ Qt::Key_BracketRight });
    connect(_rangeIncHiAction, SIGNAL(triggered()), this, SLOT(rangeIncHi()));
    addQVAction(_rangeIncHiAction, rangeMenu);
    _rangeShiftLeftAction = new QAction("Shift visible range to lower values", this);
    _rangeShiftLeftAction->setShortcuts({ Qt::Key_ParenLeft });
    connect(_rangeShiftLeftAction, SIGNAL(triggered()), this, SLOT(rangeShiftLeft()));
    addQVAction(_rangeShiftLeftAction, rangeMenu);
    _rangeShiftRightAction = new QAction("Shift visible range to higher values", this);
    _rangeShiftRightAction->setShortcuts({ Qt::Key_ParenRight });
    connect(_rangeShiftRightAction, SIGNAL(triggered()), this, SLOT(rangeShiftRight()));
    addQVAction(_rangeShiftRightAction, rangeMenu);
    _rangeResetAction = new QAction("Reset visible range", this);
    _rangeResetAction->setShortcuts({ Qt::Key_Backslash });
    connect(_rangeResetAction, SIGNAL(triggered()), this, SLOT(rangeReset()));
    addQVAction(_rangeResetAction, rangeMenu);
    rangeMenu->addSeparator();
    _rangeDRRToggleAction = new QAction("&Toggle Dynamic Range Reduction (DRR; simple tone mapping)", this);
    _rangeDRRToggleAction->setCheckable(true);
    _rangeDRRToggleAction->setShortcuts({ Qt::Key_D });
    connect(_rangeDRRToggleAction, SIGNAL(triggered()), this, SLOT(rangeDRRToggle()));
    addQVAction(_rangeDRRToggleAction, rangeMenu);
    _rangeDRRDecBrightnessAction = new QAction("&Decrease DRR brightness", this);
    _rangeDRRDecBrightnessAction->setShortcuts({ Qt::Key_Comma });
    connect(_rangeDRRDecBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRDecBrightness()));
    addQVAction(_rangeDRRDecBrightnessAction, rangeMenu);
    _rangeDRRIncBrightnessAction = new QAction("&Increase DRR brightness", this);
    _rangeDRRIncBrightnessAction->setShortcuts({ Qt::Key_Period });
    connect(_rangeDRRIncBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRIncBrightness()));
    addQVAction(_rangeDRRIncBrightnessAction, rangeMenu);
    _rangeDRRResetBrightnessAction = new QAction("&Reset DRR brightness", this);
    _rangeDRRResetBrightnessAction->setShortcuts({ Qt::Key_Slash });
    connect(_rangeDRRResetBrightnessAction, SIGNAL(triggered()), this, SLOT(rangeDRRResetBrightness()));
    addQVAction(_rangeDRRResetBrightnessAction, rangeMenu);

    QMenu* colorMapMenu = addQVMenu("&Colormap");
    _colorMapToggleOverlayAction = new QAction("Toggle colormap overlay");
    _colorMapToggleOverlayAction->setCheckable(true);
    _colorMapToggleOverlayAction->setShortcuts({ Qt::Key_M });
    connect(_colorMapToggleOverlayAction, SIGNAL(triggered()), this, SLOT(colorMapToggleOverlay()));
    addQVAction(_colorMapToggleOverlayAction, colorMapMenu);
    _colorMapDisableAction = new QAction("Disable color &map", this);
    _colorMapDisableAction->setShortcuts({ Qt::Key_F4 });
    connect(_colorMapDisableAction, SIGNAL(triggered()), this, SLOT(colorMapDisable()));
    addQVAction(_colorMapDisableAction, colorMapMenu);
    _colorMapCycleSequentialAction = new QAction("Enable next &sequential color map", this);
    _colorMapCycleSequentialAction->setShortcuts({ Qt::Key_F5 });
    connect(_colorMapCycleSequentialAction, SIGNAL(triggered()), this, SLOT(colorMapCycleSequential()));
    addQVAction(_colorMapCycleSequentialAction, colorMapMenu);
    _colorMapCycleDivergingAction = new QAction("Enable next d&iverging color map", this);
    _colorMapCycleDivergingAction->setShortcuts({ Qt::Key_F6 });
    connect(_colorMapCycleDivergingAction, SIGNAL(triggered()), this, SLOT(colorMapCycleDiverging()));
    addQVAction(_colorMapCycleDivergingAction, colorMapMenu);
    _colorMapQualitativeAction = new QAction("Enable next &qualitative color map", this);
    _colorMapQualitativeAction->setShortcuts({ Qt::Key_F7 });
    connect(_colorMapQualitativeAction, SIGNAL(triggered()), this, SLOT(colorMapQualitative()));
    addQVAction(_colorMapQualitativeAction, colorMapMenu);
    _colorMapCustomAction = new QAction("Enable &custom color map (import from clipboard in image or CSV format)", this);
    _colorMapCustomAction->setShortcuts({ Qt::Key_F8 });
    connect(_colorMapCustomAction, SIGNAL(triggered()), this, SLOT(colorMapCustom()));
    addQVAction(_colorMapCustomAction, colorMapMenu);

    QMenu* viewMenu = addQVMenu("&View");
    _viewToggleFullscreenAction = new QAction("Toggle &Fullscreen", this);
    if (QKeySequence(QKeySequence::FullScreen) != QKeySequence(Qt::Key_F11))
        _viewToggleFullscreenAction->setShortcuts({ Qt::Key_F11, QKeySequence::FullScreen });
    else
        _viewToggleFullscreenAction->setShortcuts({ Qt::Key_F11 });
    connect(_viewToggleFullscreenAction, SIGNAL(triggered()), this, SLOT(viewToggleFullscreen()));
    addQVAction(_viewToggleFullscreenAction, viewMenu);
    viewMenu->addSeparator();
    _viewZoomInAction = new QAction("Zoom &in", this);
    _viewZoomInAction->setShortcuts({ Qt::Key_Plus, QKeySequence::ZoomIn });
    connect(_viewZoomInAction, SIGNAL(triggered()), this, SLOT(viewZoomIn()));
    addQVAction(_viewZoomInAction, viewMenu);
    _viewZoomOutAction = new QAction("Zoom &out", this);
    _viewZoomOutAction->setShortcuts({ Qt::Key_Minus, QKeySequence::ZoomOut });
    connect(_viewZoomOutAction, SIGNAL(triggered()), this, SLOT(viewZoomOut()));
    addQVAction(_viewZoomOutAction, viewMenu);
    _viewZoomResetAction = new QAction("&Reset zoom", this);
    _viewZoomResetAction->setShortcuts({ Qt::Key_Equal });
    connect(_viewZoomResetAction, SIGNAL(triggered()), this, SLOT(viewZoomReset()));
    addQVAction(_viewZoomResetAction, viewMenu);
    _viewRecenterAction = new QAction("Recenter view", this);
    _viewRecenterAction->setShortcuts({ Qt::Key_Space });
    connect(_viewRecenterAction, SIGNAL(triggered()), this, SLOT(viewRecenter()));
    addQVAction(_viewRecenterAction, viewMenu);
    viewMenu->addSeparator();
    _viewToggleLinearInterpolationAction = new QAction("Toggle &linear interpolation for magnified views");
    _viewToggleLinearInterpolationAction->setCheckable(true);
    _viewToggleLinearInterpolationAction->setShortcuts({ Qt::Key_L });
    connect(_viewToggleLinearInterpolationAction, SIGNAL(triggered()), this, SLOT(viewToggleLinearInterpolation()));
    addQVAction(_viewToggleLinearInterpolationAction, viewMenu);
    _viewToggleGridAction = new QAction("Toggle &grid for magnified views");
    _viewToggleGridAction->setCheckable(true);
    _viewToggleGridAction->setShortcuts({ Qt::Key_G });
    connect(_viewToggleGridAction, SIGNAL(triggered()), this, SLOT(viewToggleGrid()));
    addQVAction(_viewToggleGridAction, viewMenu);
    _viewToggleApplyCurrentParametersToAllFilesAction = new QAction("Toggle &application of current parameters to all files", this);
    _viewToggleApplyCurrentParametersToAllFilesAction->setCheckable(true);
    _viewToggleApplyCurrentParametersToAllFilesAction->setShortcuts({ Qt::Key_A });
    connect(_viewToggleApplyCurrentParametersToAllFilesAction, SIGNAL(triggered()), this, SLOT(viewToggleApplyCurrentParametersToAllFiles()));
    addQVAction(_viewToggleApplyCurrentParametersToAllFilesAction, viewMenu);

    QMenu* helpMenu = addQVMenu("&Help");
    _helpAboutAction = new QAction("&About");
    connect(_helpAboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
    addQVAction(_helpAboutAction, helpMenu);

    connect(_qv, SIGNAL(toggleFullscreen()), this, SLOT(viewToggleFullscreen()));
    connect(_qv, SIGNAL(parametersChanged()), this, SLOT(updateFromParameters()));
    updateFromParameters();
    setCentralWidget(_qv);

    setMinimumSize(menuBar()->sizeHint().width(), menuBar()->sizeHint().width() / 2);
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

void Gui::fileQuit()
{
    close();
}

void Gui::frameToggleInfo()
{
    _qv->toggleOverlayInfo();
}

void Gui::frameToggleValue()
{
    _qv->toggleOverlayValue();
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

void Gui::channelToggleStatistics()
{
    _qv->toggleOverlayStatistics();
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

void Gui::viewToggleApplyCurrentParametersToAllFiles()
{
    _qv->toggleApplyCurrentParametersToAllFiles();
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
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    Parameters p;
    std::string dummy;
    if (_set.currentParameters()) {
        p = *(_set.currentParameters());
    }

    //_fileOpenAction;
    _fileCloseAction->setEnabled(file);
    _fileReloadAction->setEnabled(file);
    _fileSaveCurrentViewAction->setEnabled(file);
    _fileSaveViewAction->setEnabled(file);
    _fileCopyCurrentViewAction->setEnabled(file);
    _fileCopyViewAction->setEnabled(file);
    _fileNextAction->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() < _set.fileCount() - 1);
    _filePrevAction->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() > 0);
    _fileNext10Action->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() < _set.fileCount() - 1);
    _filePrev10Action->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() > 0);
    _fileNext100Action->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() < _set.fileCount() - 1);
    _filePrev100Action->setEnabled(file && _set.fileCount() > 1 && _set.fileIndex() > 0);
    //_fileQuitAction;
    _frameToggleInfoAction->setEnabled(frame);
    _frameToggleInfoAction->setChecked(_qv->overlayInfoActive);
    _frameToggleValueAction->setEnabled(frame);
    _frameToggleValueAction->setChecked(_qv->overlayValueActive);
    _frameNextAction->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() < file->frameCount(dummy) - 1);
    _framePrevAction->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() > 0);
    _frameNext10Action->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() < file->frameCount(dummy) - 1);
    _framePrev10Action->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() > 0);
    _frameNext100Action->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() < file->frameCount(dummy) - 1);
    _framePrev100Action->setEnabled(file && file->frameCount(dummy) > 1 && file->frameIndex() > 0);
    _channelToggleStatisticsAction->setEnabled(frame);
    _channelToggleStatisticsAction->setChecked(_qv->overlayStatisticActive);
    _channelColorAction->setEnabled(frame && frame->colorSpace() != ColorSpaceNone);
    _channelColorAction->setChecked(frame && frame->channelIndex() == ColorChannelIndex);
    _channel0Action->setEnabled(frame);
    _channel0Action->setChecked(frame && frame->channelIndex() == 0);
    _channel0Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 0 ? QString(frame->channelName(0).c_str()) : QString::number(0)));
    _channel1Action->setEnabled(frame && frame->channelCount() > 1);
    _channel1Action->setChecked(frame && frame->channelIndex() == 1);
    _channel1Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 1 ? QString(frame->channelName(1).c_str()) : QString::number(1)));
    _channel2Action->setEnabled(frame && frame->channelCount() > 2);
    _channel2Action->setChecked(frame && frame->channelIndex() == 2);
    _channel2Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 2 ? QString(frame->channelName(2).c_str()) : QString::number(2)));
    _channel3Action->setEnabled(frame && frame->channelCount() > 3);
    _channel3Action->setChecked(frame && frame->channelIndex() == 3);
    _channel3Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 3 ? QString(frame->channelName(3).c_str()) : QString::number(3)));
    _channel4Action->setEnabled(frame && frame->channelCount() > 4);
    _channel4Action->setChecked(frame && frame->channelIndex() == 4);
    _channel4Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 4 ? QString(frame->channelName(4).c_str()) : QString::number(4)));
    _channel5Action->setEnabled(frame && frame->channelCount() > 5);
    _channel5Action->setChecked(frame && frame->channelIndex() == 5);
    _channel5Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 5 ? QString(frame->channelName(5).c_str()) : QString::number(5)));
    _channel6Action->setEnabled(frame && frame->channelCount() > 6);
    _channel6Action->setChecked(frame && frame->channelIndex() == 6);
    _channel6Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 6 ? QString(frame->channelName(6).c_str()) : QString::number(6)));
    _channel7Action->setEnabled(frame && frame->channelCount() > 7);
    _channel7Action->setChecked(frame && frame->channelIndex() == 7);
    _channel7Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 7 ? QString(frame->channelName(7).c_str()) : QString::number(7)));
    _channel8Action->setEnabled(frame && frame->channelCount() > 8);
    _channel8Action->setChecked(frame && frame->channelIndex() == 8);
    _channel8Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 8 ? QString(frame->channelName(8).c_str()) : QString::number(8)));
    _channel9Action->setEnabled(frame && frame->channelCount() > 9);
    _channel9Action->setChecked(frame && frame->channelIndex() == 9);
    _channel9Action->setText(QString("Show channel %1 of this frame").arg(frame && frame->channelCount() > 9 ? QString(frame->channelName(9).c_str()) : QString::number(9)));
    _rangeToggleOverlayAction->setEnabled(frame);
    _rangeToggleOverlayAction->setChecked(_qv->overlayHistogramActive);
    _rangeDecLoAction->setEnabled(frame);
    _rangeIncLoAction->setEnabled(frame);
    _rangeDecHiAction->setEnabled(frame);
    _rangeIncHiAction->setEnabled(frame);
    _rangeShiftLeftAction->setEnabled(frame);
    _rangeShiftRightAction->setEnabled(frame);
    _rangeResetAction->setEnabled(frame);
    _rangeDRRToggleAction->setEnabled(frame);
    _rangeDRRToggleAction->setChecked(p.dynamicRangeReduction);
    _rangeDRRDecBrightnessAction->setEnabled(frame);
    _rangeDRRIncBrightnessAction->setEnabled(frame);
    _rangeDRRResetBrightnessAction->setEnabled(frame);
    _colorMapToggleOverlayAction->setEnabled(frame);
    _colorMapToggleOverlayAction->setChecked(_qv->overlayColorMapActive);
    _colorMapDisableAction->setEnabled(frame);
    _colorMapCycleSequentialAction->setEnabled(frame);
    _colorMapCycleDivergingAction->setEnabled(frame);
    _colorMapQualitativeAction->setEnabled(frame);
    _colorMapCustomAction->setEnabled(frame);
    //_viewToggleFullscreenAction;
    _viewZoomInAction->setEnabled(frame);
    _viewZoomOutAction->setEnabled(frame);
    _viewZoomResetAction->setEnabled(frame);
    _viewRecenterAction->setEnabled(frame);
    _viewToggleLinearInterpolationAction->setEnabled(frame);
    _viewToggleLinearInterpolationAction->setChecked(p.magInterpolation);
    _viewToggleGridAction->setEnabled(frame);
    _viewToggleGridAction->setChecked(p.magGrid);
    _viewToggleApplyCurrentParametersToAllFilesAction->setEnabled(file && _set.fileCount() > 1);
    _viewToggleApplyCurrentParametersToAllFilesAction->setChecked(_set.applyCurrentParametersToAllFiles());
    //_helpAboutAction;
}

#ifndef QT_NO_CONTEXTMENU
void Gui::contextMenuEvent(QContextMenuEvent* event)
{
    _contextMenu->exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU
