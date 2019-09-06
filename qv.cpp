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

#include <limits>
#include <cmath>

#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "utils.hpp"
#include "qv.hpp"
#include "gl.hpp"


QV::QV(Set& set, Parameters& parameters) :
    _set(set), _parameters(parameters),
    _dragMode(false),
    _overlayHelpActive(false),
    _overlayInfoActive(false),
    _overlayStatisticActive(false),
    _overlayHistogramActive(false),
    _overlayColorMapActive(false)
{
    setIcon(QIcon(":cg-logo.png"));
    setMinimumSize(QSize(500, 500));
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    QSize frameSize = (frame ? QSize(frame->width(), frame->height()) : QSize(0, 0));
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    QSize maxSize = 0.9f * screenSize;
    resize(frameSize.boundedTo(maxSize));
    updateTitle();
}

void QV::updateTitle()
{
    std::string s = _set.currentDescription();
    if (s.size() == 0)
        s = "qv";
    else
        s += " - qv";
    setTitle(s.c_str());
}

void QV::initializeGL()
{
    ASSERT_GLCHECK();

    auto gl = getGlFunctionsFromCurrentContext();
    if (!gl) {
        qFatal("No valid OpenGL context.");
    }

    const float quadPositions[] = {
        -1.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,
        +1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f
    };
    const float quadTexCoords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    static const unsigned short quadIndices[] = {
        0, 3, 1, 1, 3, 2
    };
    gl->glGenVertexArrays(1, &_vao);
    gl->glBindVertexArray(_vao);
    GLuint quadPositionBuf;
    gl->glGenBuffers(1, &quadPositionBuf);
    gl->glBindBuffer(GL_ARRAY_BUFFER, quadPositionBuf);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions), quadPositions, GL_STATIC_DRAW);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->glEnableVertexAttribArray(0);
    GLuint quadTexCoordBuf;
    gl->glGenBuffers(1, &quadTexCoordBuf);
    gl->glBindBuffer(GL_ARRAY_BUFFER, quadTexCoordBuf);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCoords), quadTexCoords, GL_STATIC_DRAW);
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    gl->glEnableVertexAttribArray(1);
    GLuint quadIndexBuf;
    gl->glGenBuffers(1, &quadIndexBuf);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuf);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    ASSERT_GLCHECK();

    QString viewVsSource = readFile(":shader-view-vertex.glsl");
    QString viewFsSource  = readFile(":shader-view-fragment.glsl");
    if (isOpenGLES()) {
        viewVsSource.prepend("#version 300 es\n");
        viewFsSource.prepend("precision highp float;\n");
        viewFsSource.prepend("#version 300 es\n");
    } else {
        viewVsSource.prepend("#version 330\n");
        viewFsSource.prepend("#version 330\n");
    }
    _viewPrg.addShaderFromSourceCode(QOpenGLShader::Vertex, viewVsSource);
    _viewPrg.addShaderFromSourceCode(QOpenGLShader::Fragment, viewFsSource);
    _viewPrg.link();

    QString overlayVsSource = readFile(":shader-overlay-vertex.glsl");
    QString overlayFsSource  = readFile(":shader-overlay-fragment.glsl");
    if (isOpenGLES()) {
        overlayVsSource.prepend("#version 300 es\n");
        overlayFsSource.prepend("precision highp float;\n");
        overlayFsSource.prepend("#version 300 es\n");
    } else {
        overlayVsSource.prepend("#version 330\n");
        overlayFsSource.prepend("#version 330\n");
    }
    _overlayPrg.addShaderFromSourceCode(QOpenGLShader::Vertex, overlayVsSource);
    _overlayPrg.addShaderFromSourceCode(QOpenGLShader::Fragment, overlayFsSource);
    _overlayPrg.link();

    ASSERT_GLCHECK();

    gl->glDisable(GL_DEPTH_TEST);
}

QPoint QV::renderFrame(Frame* frame, int w, int h, QPoint mousePos)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glUseProgram(_viewPrg.programId());
    // Aspect ratio
    float windowAR = float(w) / h;
    float frameAR = float(frame->width()) / frame->height();
    float arFactorX = 1.0f;
    float arFactorY = 1.0f;
    if (windowAR > frameAR) {
        arFactorX = frameAR / windowAR;
    } else if (frameAR > windowAR) {
        arFactorY = windowAR / frameAR;
    }
    // Navigation and zoom
    float xFactor = arFactorX / _parameters.zoom;
    float yFactor = arFactorY / _parameters.zoom;
    float xOffset = 2.0f * _parameters.xOffset / w;
    float yOffset = 2.0f * _parameters.yOffset / h;
    _viewPrg.setUniformValue("xFactor", xFactor);
    _viewPrg.setUniformValue("yFactor", yFactor);
    _viewPrg.setUniformValue("xOffset", xOffset);
    _viewPrg.setUniformValue("yOffset", yOffset);
    _viewPrg.setUniformValue("magGrid", _parameters.magGrid);
    // Min/max values
    float visMinVal = _parameters.visMinVal(frame->channelIndex());
    float visMaxVal = _parameters.visMaxVal(frame->channelIndex());
    if (!std::isfinite(visMinVal) || !std::isfinite(visMaxVal)) {
        visMinVal = frame->visMinVal(frame->channelIndex());
        visMaxVal = frame->visMaxVal(frame->channelIndex());
        _parameters.setVisMinVal(frame->channelIndex(), visMinVal);
        _parameters.setVisMaxVal(frame->channelIndex(), visMaxVal);
    }
    _viewPrg.setUniformValue("visMinVal", visMinVal);
    _viewPrg.setUniformValue("visMaxVal", visMaxVal);
    // Color and data information
    bool showColor = (frame->channelIndex() == ColorChannelIndex);
    _viewPrg.setUniformValue("colorMap", _parameters.colorMap().type() != ColorMapNone);
    _viewPrg.setUniformValue("showColor", showColor);
    _viewPrg.setUniformValue("colorSpace", int(frame->colorSpace()));
    _viewPrg.setUniformValue("channelCount", frame->channelCount());
    _viewPrg.setUniformValue("dataChannelIndex", frame->channelIndex());
    _viewPrg.setUniformValue("colorChannel0Index", frame->colorChannelIndex(0));
    _viewPrg.setUniformValue("colorChannel1Index", frame->colorChannelIndex(1));
    _viewPrg.setUniformValue("colorChannel2Index", frame->colorChannelIndex(2));
    _viewPrg.setUniformValue("alphaChannelIndex", frame->alphaChannelIndex());
    // Textures
    _viewPrg.setUniformValue("tex0", 0);
    _viewPrg.setUniformValue("tex1", 1);
    _viewPrg.setUniformValue("tex2", 2);
    _viewPrg.setUniformValue("alphaTex", 3);
    _viewPrg.setUniformValue("colorMapTex", 4);
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, showColor ? frame->texture(frame->colorChannelIndex(0)) : frame->texture(frame->channelIndex()));
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE1);
    gl->glBindTexture(GL_TEXTURE_2D, showColor ? frame->texture(frame->colorChannelIndex(1)) : 0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE2);
    gl->glBindTexture(GL_TEXTURE_2D, showColor ? frame->texture(frame->colorChannelIndex(2)) : 0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE3);
    gl->glBindTexture(GL_TEXTURE_2D, (showColor && frame->hasAlpha()) ? frame->texture(frame->alphaChannelIndex()) : 0);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE4);
    gl->glBindTexture(GL_TEXTURE_2D, _parameters.colorMap().texture());
    // Draw the image
    gl->glBindVertexArray(_vao);
    gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    ASSERT_GLCHECK();
    // Return array coordinates
    float wx = (float(mousePos.x()) / w - 0.5f) * 2.0f;
    float wy = (float(h - 1 - mousePos.y()) / h - 0.5f) * 2.0f;
    float px = (wx - xOffset) / xFactor;
    float py = (wy - yOffset) / yFactor;
    float dx = 0.5f * (px + 1.0f) * frame->width();
    float dy = 0.5f * (py + 1.0f) * frame->height();
    int dataX = dx;
    int dataY = dy;
    if (dx < 0.0f || dataX >= frame->width())
        dataX = -1;
    if (dy < 0.0f || dataY >= frame->height())
        dataY = -1;
    return QPoint(dataX, dataY);
}

void QV::resizeGL(int w, int h)
{
    _w = w;
    _h = h;
}

void QV::paintGL()
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glViewport(0, 0, _w, _h);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GLCHECK();

    // Draw the frame
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    QPoint arrayCoordinates(-1, -1);
    if (frame) {
        arrayCoordinates = renderFrame(frame, _w, _h, _mousePos);
    }

    // Draw the overlays
    bool overlayHelpActive = _overlayHelpActive;
    bool overlayInfoActive = _overlayInfoActive;
    bool overlayStatisticActive = _overlayStatisticActive;
    bool overlayHistogramActive = _overlayHistogramActive;
    bool overlayColorMapActive = _overlayColorMapActive;
    if (!frame) {
        overlayHelpActive = true;
        overlayInfoActive = false;
        overlayStatisticActive = false;
        overlayHistogramActive = false;
        overlayColorMapActive = false;
    }
    ASSERT_GLCHECK();
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int overlayYOffset = 0;
    if (overlayColorMapActive) {
        _overlayColorMap.update(_w, _parameters);
        gl->glViewport(0, overlayYOffset, _w, _overlayColorMap.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayColorMap.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayColorMap.heightInPixels();
    }
    if (overlayHistogramActive) {
        _overlayHistogram.update(_w, arrayCoordinates, _set, _parameters);
        gl->glViewport(0, overlayYOffset, _w, _overlayHistogram.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayHistogram.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayHistogram.heightInPixels();
    }
    if (overlayStatisticActive) {
        _overlayStatistic.update(_w, _set, _parameters);
        gl->glViewport(0, overlayYOffset, _w, _overlayStatistic.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayStatistic.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayStatistic.heightInPixels();
    }
    if (overlayInfoActive) {
        _overlayInfo.update(_w, arrayCoordinates, _set);
        gl->glViewport(0, overlayYOffset, _w, _overlayInfo.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayInfo.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayInfo.heightInPixels();
    }
    if (overlayHelpActive) {
        _overlayHelp.update(_w);
        gl->glViewport(0, overlayYOffset, _w, _overlayHelp.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayHelp.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayHelp.heightInPixels();
    }
    gl->glDisable(GL_BLEND);
    ASSERT_GLCHECK();
}

void QV::openFile()
{
    int previousFileCount = _set.fileCount();
    std::string errMsg;
    QStringList names = QFileDialog::getOpenFileNames();
    for (int i = 0; i < names.size(); i++) {
        if (!_set.addFile(qPrintable(names[i]), errMsg)) {
            QMessageBox::critical(nullptr, "Error", errMsg.c_str());
        }
    }
    while (_set.fileCount() > previousFileCount && !_set.setFileIndex(previousFileCount, errMsg)) {
        QMessageBox::critical(nullptr, "Error", (errMsg
                    + ".\n\nClosing this file.").c_str());
        _set.removeFile(previousFileCount);
    }
    if (previousFileCount == 0 && _set.fileCount() > 0) {
        _parameters = Parameters();
        _parameters.magInterpolation = (_set.currentFile()->currentFrame()->channelIndex() == ColorChannelIndex);
    }
    this->updateTitle();
    this->update();
}

void QV::closeFile()
{
    if (_set.fileIndex() >= 0) {
        _set.removeFile(_set.fileIndex());
        this->updateTitle();
        this->update();
    }
}

void QV::reloadFile()
{
    File* file = _set.currentFile();
    if (!file)
        return;
    std::string errMsg;
    if (!file->reload(errMsg)) {
        QMessageBox::critical(nullptr, "Error", errMsg.c_str());
    }
    this->updateTitle();
    this->update();
}

void QV::adjustFileIndex(int offset)
{
    int i = _set.fileIndex();
    if (i < 0)
        return;
    int ni = i + offset;
    if (ni < 0)
        ni = 0;
    else if (ni >= _set.fileCount())
        ni = _set.fileCount() - 1;
    if (ni != i) {
        std::string errMsg;
        if (!_set.setFileIndex(ni, errMsg)) {
            QMessageBox::critical(nullptr, "Error", (errMsg
                        + ".\n\nClosing this file.").c_str());
            _set.removeFile(ni);
        }
        this->updateTitle();
        this->update();
    }
}

void QV::adjustFrameIndex(int offset)
{
    File* file = _set.currentFile();
    if (!file)
        return;

    std::string errMsg;
    int fc = file->frameCount(errMsg);
    if (fc < 1) {
        QMessageBox::critical(nullptr, "Error", errMsg.c_str());
        return;
    }

    int i = file->frameIndex();
    int ni = i + offset;
    if (ni < 0)
        ni = 0;
    else if (ni >= fc)
        ni = fc - 1;
    if (ni != i) {
        if (file->setFrameIndex(ni, errMsg)) {
            this->updateTitle();
            this->update();
        } else {
            QMessageBox::critical(nullptr, "Error", errMsg.c_str());
        }
    }
}

void QV::setChannelIndex(int index)
{
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (!frame)
        return;
    if (index == ColorChannelIndex) {
        if (frame->colorSpace() != ColorSpaceNone)
            frame->setChannelIndex(index);
    } else {
        if (index >= 0 && index < frame->channelCount())
            frame->setChannelIndex(index);
    }
    this->updateTitle();
    this->update();
}

void QV::adjustZoom(int steps)
{
    float adjustmentPerStep = std::max(0.01f, _parameters.zoom * 0.05f);
    float oldZoom = _parameters.zoom;
    float newZoom = std::max(0.01f, _parameters.zoom - steps * adjustmentPerStep);
    _parameters.xOffset = _parameters.xOffset * oldZoom / newZoom;
    _parameters.yOffset = _parameters.yOffset * oldZoom / newZoom;
    _parameters.zoom = newZoom;
    this->update();
}

void QV::adjustVisInterval(int minSteps, int maxSteps)
{
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (!frame)
        return;
    float defaultVisMin = frame->currentVisMinVal();
    float defaultVisMax = frame->currentVisMaxVal();
    float adjustment = (defaultVisMax - defaultVisMin) / 100.0f;
    float oldMinVal = _parameters.visMinVal(frame->channelIndex());
    float oldMaxVal = _parameters.visMaxVal(frame->channelIndex());
    float newMinVal = oldMinVal + minSteps * adjustment;
    float newMaxVal = oldMaxVal + maxSteps * adjustment;
    if (newMinVal < defaultVisMin)
        newMinVal = defaultVisMin;
    else if (_parameters.visMaxVal(frame->channelIndex()) - newMinVal < adjustment)
        newMinVal = _parameters.visMaxVal(frame->channelIndex()) - adjustment;
    else if (newMinVal > defaultVisMax - adjustment)
        newMinVal = defaultVisMax - adjustment;
    if (newMaxVal < defaultVisMin + adjustment)
        newMaxVal = defaultVisMin + adjustment;
    else if (newMaxVal - _parameters.visMinVal(frame->channelIndex()) < adjustment)
        newMaxVal = _parameters.visMinVal(frame->channelIndex()) + adjustment;
    else if (newMaxVal > defaultVisMax)
        newMaxVal = defaultVisMax;
    _parameters.setVisMinVal(frame->channelIndex(), newMinVal);
    _parameters.setVisMaxVal(frame->channelIndex(), newMaxVal);
    this->update();
}

void QV::resetVisInterval()
{
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (!frame)
        return;
    _parameters.setVisMinVal(frame->channelIndex(), std::numeric_limits<float>::quiet_NaN());
    _parameters.setVisMaxVal(frame->channelIndex(), std::numeric_limits<float>::quiet_NaN());
    this->update();
}

void QV::changeColorMap(ColorMapType type)
{
    ColorMapType oldType = _parameters.colorMap().type();
    if (oldType != type) {
        _parameters.colorMap().setType(type);
    } else {
        _parameters.colorMap().cycle();
    }
    this->update();
}

void QV::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Q || e->matches(QKeySequence::Quit)) {
        this->close();
    } else if (e->key() == Qt::Key_Escape) {
        if (this->windowStates() & Qt::WindowFullScreen)
            this->showNormal();
        else
            this->close();
    } else if (e->key() == Qt::Key_F11 || e->matches(QKeySequence::FullScreen)) {
        if (this->windowStates() & Qt::WindowFullScreen)
            this->showNormal();
        else
            this->showFullScreen();
    } else if (e->key() == Qt::Key_O || e->matches(QKeySequence::Open)) {
        openFile();
    } else if (e->key() == Qt::Key_W || e->matches(QKeySequence::Close)) {
        closeFile();
    } else if (e->key() == Qt::Key_R) {
        reloadFile();
    } else if (e->key() == Qt::Key_Less || e->matches(QKeySequence::PreviousChild)) {
        adjustFileIndex(-1);
    } else if (e->key() == Qt::Key_Greater || e->matches(QKeySequence::NextChild)) {
        adjustFileIndex(+1);
    } else if (e->key() == Qt::Key_Left) {
        adjustFrameIndex(-1);
    } else if (e->key() == Qt::Key_Right) {
        adjustFrameIndex(+1);
    } else if (e->key() == Qt::Key_Down) {
        adjustFrameIndex(+10);
    } else if (e->key() == Qt::Key_Up) {
        adjustFrameIndex(-10);
    } else if (e->key() == Qt::Key_PageDown) {
        adjustFrameIndex(+100);
    } else if (e->key() == Qt::Key_PageUp) {
        adjustFrameIndex(-100);
    } else if (e->key() == Qt::Key_C) {
        setChannelIndex(ColorChannelIndex);
    } else if (e->key() == Qt::Key_0) {
        setChannelIndex(0);
    } else if (e->key() == Qt::Key_1) {
        setChannelIndex(1);
    } else if (e->key() == Qt::Key_2) {
        setChannelIndex(2);
    } else if (e->key() == Qt::Key_3) {
        setChannelIndex(3);
    } else if (e->key() == Qt::Key_4) {
        setChannelIndex(4);
    } else if (e->key() == Qt::Key_5) {
        setChannelIndex(5);
    } else if (e->key() == Qt::Key_6) {
        setChannelIndex(6);
    } else if (e->key() == Qt::Key_7) {
        setChannelIndex(7);
    } else if (e->key() == Qt::Key_8) {
        setChannelIndex(8);
    } else if (e->key() == Qt::Key_9) {
        setChannelIndex(9);
    } else if (e->key() == Qt::Key_L) {
        _parameters.magInterpolation = !_parameters.magInterpolation;
        this->update();
    } else if (e->key() == Qt::Key_G) {
        _parameters.magGrid = !_parameters.magGrid;
        this->update();
    } else if (e->key() == Qt::Key_Equal) {
        _parameters.zoom = 1.0f;
        this->update();
    } else if (e->key() == Qt::Key_Minus || e->matches(QKeySequence::ZoomOut)) {
        adjustZoom(-1);
    } else if (e->key() == Qt::Key_Plus || e->matches(QKeySequence::ZoomIn)) {
        adjustZoom(+1);
    } else if (e->key() == Qt::Key_Space) {
        _parameters.xOffset = 0.0f;
        _parameters.yOffset = 0.0f;
        this->update();
    } else if (e->key() == Qt::Key_BraceLeft) {
        adjustVisInterval(-1, 0);
    } else if (e->key() == Qt::Key_BraceRight) {
        adjustVisInterval(+1, 0);
    } else if (e->key() == Qt::Key_BracketLeft) {
        adjustVisInterval(0, -1);
    } else if (e->key() == Qt::Key_BracketRight) {
        adjustVisInterval(0, +1);
    } else if (e->key() == Qt::Key_ParenLeft) {
        adjustVisInterval(-1, -1);
    } else if (e->key() == Qt::Key_ParenRight) {
        adjustVisInterval(+1, +1);
    } else if (e->key() == Qt::Key_Backslash) {
        resetVisInterval();
    } else if (e->key() == Qt::Key_F4) {
        changeColorMap(ColorMapNone);
    } else if (e->key() == Qt::Key_F5) {
        changeColorMap(ColorMapSequential);
    } else if (e->key() == Qt::Key_F6) {
        changeColorMap(ColorMapDiverging);
    } else if (e->key() == Qt::Key_F7) {
        changeColorMap(ColorMapQualitative);
    } else if (e->key() == Qt::Key_F8) {
        changeColorMap(ColorMapCustom);
    } else if (e->key() == Qt::Key_F1 || e->matches(QKeySequence::HelpContents)) {
        _overlayHelpActive = !_overlayHelpActive;
        this->update();
    } else if (e->key() == Qt::Key_I) {
        _overlayInfoActive = !_overlayInfoActive;
        this->update();
    } else if (e->key() == Qt::Key_S) {
        _overlayStatisticActive = !_overlayStatisticActive;
        this->update();
    } else if (e->key() == Qt::Key_H) {
        _overlayHistogramActive = !_overlayHistogramActive;
        this->update();
    } else if (e->key() == Qt::Key_M) {
        _overlayColorMapActive = !_overlayColorMapActive;
        this->update();
    } else {
        QOpenGLWindow::keyPressEvent(e);
    }
}

void QV::mouseMoveEvent(QMouseEvent* e)
{
    _mousePos = e->pos();
    if (_overlayInfoActive || _overlayHistogramActive)
        this->update();
    if (_dragMode) {
        QPoint dragEnd = e->pos();
        _parameters.xOffset += dragEnd.x() - _dragStart.x();
        _parameters.yOffset -= dragEnd.y() - _dragStart.y();
        _dragStart = dragEnd;
        this->update();
    }
}

void QV::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        _dragMode = true;
        _dragStart = e->pos();
    }
}

void QV::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
	_dragMode = false;
    }
}

void QV::wheelEvent(QWheelEvent* e)
{
    adjustZoom(e->delta() / 120);
}
