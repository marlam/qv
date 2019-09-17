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
#include <QClipboard>
#include <QScreen>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>

#include "utils.hpp"
#include "qv.hpp"
#include "gl.hpp"


QV::QV(Set& set, Parameters& parameters) :
    _set(set), _parameters(parameters),
    _dragMode(false),
    _overlayHelpActive(false),
    _overlayInfoActive(false),
    _overlayValueActive(false),
    _overlayStatisticActive(false),
    _overlayHistogramActive(false),
    _overlayColorMapActive(false)
{
    setMouseTracking(true);
    setWindowIcon(QIcon(":cg-logo.png"));
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
    setWindowTitle(s.c_str());
}

void QV::initializeGL()
{
    ASSERT_GLCHECK();

    auto gl = getGlFunctionsFromCurrentContext();

    gl->glGenFramebuffers(1, &_fbo);
    gl->glGenTextures(1, &_fboTex);

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

    QString quadTreeVsSource = readFile(":shader-quadtree-vertex.glsl");
    QString quadTreeFsSource  = readFile(":shader-quadtree-fragment.glsl");
    if (isOpenGLES()) {
        quadTreeVsSource.prepend("#version 300 es\n");
        quadTreeFsSource.prepend("precision highp float;\n");
        quadTreeFsSource.prepend("#version 300 es\n");
    } else {
        quadTreeVsSource.prepend("#version 330\n");
        quadTreeFsSource.prepend("#version 330\n");
    }
    _quadTreePrg.addShaderFromSourceCode(QOpenGLShader::Vertex, quadTreeVsSource);
    _quadTreePrg.addShaderFromSourceCode(QOpenGLShader::Fragment, quadTreeFsSource);
    _quadTreePrg.link();

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

void QV::navigationParameters(Frame* frame,
        int widgetWidth, int widgetHeight,
        float& xFactor, float& yFactor,
        float& xOffset, float& yOffset)
{
    // Aspect ratio
    float windowAR = float(widgetWidth) / widgetHeight;
    float frameAR = float(frame->width()) / frame->height();
    float arFactorX = 1.0f;
    float arFactorY = 1.0f;
    if (windowAR > frameAR) {
        arFactorX = frameAR / windowAR;
    } else if (frameAR > windowAR) {
        arFactorY = windowAR / frameAR;
    }
    // Navigation and zoom
    xFactor = arFactorX / _parameters.zoom;
    yFactor = arFactorY / _parameters.zoom;
    xOffset = 2.0f * _parameters.xOffset / widgetWidth;
    yOffset = 2.0f * _parameters.yOffset / widgetHeight;
}

QPoint QV::dataCoordinates(QPoint widgetCoordinates,
        int widgetWidth, int widgetHeight,
        int frameWidth, int frameHeight,
        float xFactor, float yFactor, float xOffset, float yOffset)
{
    float wx = (float(widgetCoordinates.x()) / widgetWidth - 0.5f) * 2.0f;
    float wy = (float(widgetHeight - 1 - widgetCoordinates.y()) / widgetHeight - 0.5f) * 2.0f;
    float px = (wx - xOffset) / xFactor;
    float py = (wy - yOffset) / yFactor;
    float dx = 0.5f * (px + 1.0f) * frameWidth;
    float dy = 0.5f * (py + 1.0f) * frameHeight;
    int dataX = dx;
    int dataY = dy;
    return QPoint(dataX, dataY);
}

void QV::renderFrame(Frame* frame, int quadTreeLevel,
        float xFactor, float yFactor,
        float xOffset, float yOffset)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glUseProgram(_viewPrg.programId());
    // Navigation and zoom
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
    // Dynamic Range Reduction
    _viewPrg.setUniformValue("dynamicRangeReduction", _parameters.dynamicRangeReduction);
    _viewPrg.setUniformValue("drrBrightness", _parameters.drrBrightness);
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
    _viewPrg.setUniformValue("colorMapTex", 4);
    gl->glActiveTexture(GL_TEXTURE4);
    gl->glBindTexture(GL_TEXTURE_2D, _parameters.colorMap().texture());
    _viewPrg.setUniformValue("tex0", 0);
    _viewPrg.setUniformValue("tex1", 1);
    _viewPrg.setUniformValue("tex2", 2);
    _viewPrg.setUniformValue("alphaTex", 3);
    float quadWidthWithBorder = frame->quadWidth() + 2 * frame->quadBorderSize(quadTreeLevel);
    float quadHeightWithBorder = frame->quadHeight() + 2 * frame->quadBorderSize(quadTreeLevel);
    float texCoordFactorX = frame->quadWidth() / quadWidthWithBorder;
    float texCoordFactorY = frame->quadHeight() / quadHeightWithBorder;
    float texCoordOffsetX = frame->quadBorderSize(quadTreeLevel) / quadWidthWithBorder;
    float texCoordOffsetY = frame->quadBorderSize(quadTreeLevel) / quadHeightWithBorder;
    _viewPrg.setUniformValue("texCoordFactorX", texCoordFactorX);
    _viewPrg.setUniformValue("texCoordFactorY", texCoordFactorY);
    _viewPrg.setUniformValue("texCoordOffsetX", texCoordOffsetX);
    _viewPrg.setUniformValue("texCoordOffsetY", texCoordOffsetY);
    // Loop over the quads on the requested level
    int maxQuadTreeLevelSize = 1;
    for (int l = frame->quadTreeLevels() - 1; l > quadTreeLevel; l--)
        maxQuadTreeLevelSize *= 2;
    int coveredWidth = frame->quadWidth();
    int coveredHeight = frame->quadHeight();
    for (int l = frame->quadTreeLevels() - 1; l > 0; l--) {
        coveredWidth *= 2;
        coveredHeight *= 2;
    }
    float quadCompensationFactorX = 1.0f / (float(frame->width()) / coveredWidth);
    float quadCompensationFactorY = 1.0f / (float(frame->height()) / coveredHeight);
    gl->glBindVertexArray(_vao);
    const QRectF frustum2D(-1.0f, -1.0f, 2.0f, 2.0f);
    for (int qy = 0; qy < frame->quadTreeLevelHeight(quadTreeLevel); qy++) {
        for (int qx = 0; qx < frame->quadTreeLevelWidth(quadTreeLevel); qx++) {
            float quadFactorX = quadCompensationFactorX / maxQuadTreeLevelSize;
            float quadFactorY = quadCompensationFactorY / maxQuadTreeLevelSize;
            float quadOffsetX = qx;
            float quadOffsetY = qy;
            // "view frustum" culling
            float quadVertexMinX = (2.0f * quadOffsetX * quadFactorX - 1.0f) * xFactor + xOffset;
            float quadVertexMinY = (2.0f * quadOffsetY * quadFactorY - 1.0f) * yFactor + yOffset;
            float quadVertexMaxX = (2.0f * (1.0f + quadOffsetX) * quadFactorX - 1.0f) * xFactor + xOffset;
            float quadVertexMaxY = (2.0f * (1.0f + quadOffsetY) * quadFactorY - 1.0f) * yFactor + yOffset;
            const QRectF quadRect(quadVertexMinX, quadVertexMinY, quadVertexMaxX - quadVertexMinX, quadVertexMaxY - quadVertexMinY);
            if (!quadRect.intersects(frustum2D))
                continue;
            // render quad
            _viewPrg.setUniformValue("quadFactorX", quadFactorX);
            _viewPrg.setUniformValue("quadFactorY", quadFactorY);
            _viewPrg.setUniformValue("quadOffsetX", quadOffsetX);
            _viewPrg.setUniformValue("quadOffsetY", quadOffsetY);
            unsigned int t0 = showColor ?
                  frame->quadTexture(quadTreeLevel, qx, qy, frame->colorChannelIndex(0), _fbo, _vao, _quadTreePrg)
                : frame->quadTexture(quadTreeLevel, qx, qy, frame->channelIndex(), _fbo, _vao, _quadTreePrg);
            unsigned int t1 = showColor ?
                frame->quadTexture(quadTreeLevel, qx, qy, frame->colorChannelIndex(1), _fbo, _vao, _quadTreePrg) : 0;
            unsigned int t2 = showColor ?
                frame->quadTexture(quadTreeLevel, qx, qy, frame->colorChannelIndex(2), _fbo, _vao, _quadTreePrg) : 0;
            unsigned int t3 = (showColor && frame->hasAlpha()) ?
                frame->quadTexture(quadTreeLevel, qx, qy, frame->alphaChannelIndex(), _fbo, _vao, _quadTreePrg) : 0;
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, t0);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
            gl->glActiveTexture(GL_TEXTURE1);
            gl->glBindTexture(GL_TEXTURE_2D, t1);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
            gl->glActiveTexture(GL_TEXTURE2);
            gl->glBindTexture(GL_TEXTURE_2D, t2);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
            gl->glActiveTexture(GL_TEXTURE3);
            gl->glBindTexture(GL_TEXTURE_2D, t3);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _parameters.magInterpolation ? GL_LINEAR : GL_NEAREST);
            gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            ASSERT_GLCHECK();
        }
    }
}

QImage QV::renderFrameToImage(Frame* frame)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    int _fboBak;
    gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_fboBak);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    gl->glBindTexture(GL_TEXTURE_2D, _fboTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width(), frame->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fboTex, 0);
    gl->glViewport(0, 0, frame->width(), frame->height());
    renderFrame(frame, 0, 1.0f, 1.0f, 0.0f, 0.0f);
    QImage rawImg(frame->width(), frame->height(), QImage::Format_RGB32);
    gl->glReadPixels(0, 0, frame->width(), frame->height(), GL_RGBA, GL_UNSIGNED_BYTE, rawImg.bits());
    gl->glBindFramebuffer(GL_FRAMEBUFFER, _fboBak);
    ASSERT_GLCHECK();
    QImage img(frame->width(), frame->height(), QImage::Format_RGB32);
    for (int l = 0; l < img.height(); l++) {
        const unsigned char* srcLine = rawImg.scanLine(img.height() - 1 - l);
        unsigned char* dstLine = img.scanLine(l);
        for (int c = 0; c < img.width(); c++) {
            unsigned char v0 = srcLine[4 * c + 0];
            unsigned char v1 = srcLine[4 * c + 1];
            unsigned char v2 = srcLine[4 * c + 2];
            dstLine[4 * c + 0] = v2;
            dstLine[4 * c + 1] = v1;
            dstLine[4 * c + 2] = v0;
        }
    }
    return img;
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
    QPoint dataCoords(-1, -1);
    if (frame) {
        float xFactor, yFactor, xOffset, yOffset;
        navigationParameters(frame, _w, _h, xFactor, yFactor, xOffset, yOffset);
        // which part of the data do we cover?
        QPoint dataA = dataCoordinates(QPoint(0, 0), _w, _h,
                frame->width(), frame->height(),
                xFactor, yFactor, xOffset, yOffset);
        QPoint dataO = dataCoordinates(QPoint(_w, _h), _w, _h,
                frame->width(), frame->height(),
                xFactor, yFactor, xOffset, yOffset);
        int dataWidth = std::max(dataA.x(), dataO.x()) - std::min(dataA.x(), dataO.x()) + 1;
        int dataHeight = std::max(dataA.y(), dataO.y()) - std::min(dataA.y(), dataO.y()) + 1;
        float widthRatio = float(dataWidth) / _w;
        float heightRatio = float(dataHeight) / _h;
        float ratio = std::min(widthRatio, heightRatio);
        int qLevel = 0;
        if (ratio > 1.0f)
            qLevel = std::log2(ratio);
        if (qLevel >= frame->quadTreeLevels())
            qLevel = frame->quadTreeLevels() - 1;
        // render
        renderFrame(frame, qLevel, xFactor, yFactor, xOffset, yOffset);
        dataCoords = dataCoordinates(_mousePos, _w, _h,
                frame->width(), frame->height(),
                xFactor, yFactor, xOffset, yOffset);
        if (dataCoords.x() < 0 || dataCoords.x() >= frame->width()
                || dataCoords.y() < 0 || dataCoords.y() >= frame->height()) {
            dataCoords = QPoint(-1, -1);
        }
    }

    // Draw the overlays
    bool overlayHelpActive = _overlayHelpActive;
    bool overlayInfoActive = _overlayInfoActive;
    bool overlayValueActive = _overlayValueActive;
    bool overlayStatisticActive = _overlayStatisticActive;
    bool overlayHistogramActive = _overlayHistogramActive;
    bool overlayColorMapActive = _overlayColorMapActive;
    if (!frame) {
        overlayHelpActive = true;
        overlayInfoActive = false;
        overlayValueActive = false;
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
        _overlayHistogram.update(_w, dataCoords, _set, _parameters);
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
    if (overlayValueActive) {
        _overlayValue.update(_w, dataCoords, _set);
        gl->glViewport(0, overlayYOffset, _w, _overlayValue.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayValue.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        overlayYOffset += _overlayValue.heightInPixels();
    }
    if (overlayInfoActive) {
        _overlayInfo.update(_w, _set);
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
    QStringList names = QFileDialog::getOpenFileNames(this);
    for (int i = 0; i < names.size(); i++) {
        if (!_set.addFile(qPrintable(names[i]), errMsg)) {
            QMessageBox::critical(this, "Error", errMsg.c_str());
        }
    }
    while (_set.fileCount() > previousFileCount && !_set.setFileIndex(previousFileCount, errMsg)) {
        QMessageBox::critical(this, "Error", (errMsg
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
        QMessageBox::critical(this, "Error", errMsg.c_str());
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
            QMessageBox::critical(this, "Error", (errMsg
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
        QMessageBox::critical(this, "Error", errMsg.c_str());
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
            QMessageBox::critical(this, "Error", errMsg.c_str());
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
    float adjustmentPerStep = std::max(0.000001f, _parameters.zoom * 0.05f);
    float oldZoom = _parameters.zoom;
    float newZoom = std::max(0.000001f, _parameters.zoom - steps * adjustmentPerStep);
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

void QV::saveView(bool pure)
{
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (!frame)
        return;
    QString name = QFileDialog::getSaveFileName(this, QString(), QString(), "PNG images (*.png)");
    if (!name.isEmpty()) {
        QImage img = (pure ? renderFrameToImage(frame) : grabFramebuffer());
        if (!img.save(name, "png")) {
            QMessageBox::critical(this, "Error", "Saving failed.");
        }
    }
}

void QV::copyView(bool pure)
{
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (!frame)
        return;
    QGuiApplication::clipboard()->setImage(
            pure ? renderFrameToImage(frame) : grabFramebuffer());
}

void QV::keyReleaseEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Q || e->matches(QKeySequence::Quit)) {
        this->close();
    } else if (e->key() == Qt::Key_Escape) {
        if (this->windowState() & Qt::WindowFullScreen)
            this->showNormal();
        else
            this->close();
    } else if (e->key() == Qt::Key_F11 || e->matches(QKeySequence::FullScreen)) {
        if (this->windowState() & Qt::WindowFullScreen)
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
    } else if (e->key() == Qt::Key_D) {
        _parameters.dynamicRangeReduction = !_parameters.dynamicRangeReduction;
        this->update();
    } else if (e->key() == Qt::Key_Comma) {
        _parameters.drrBrightness = std::max(1.0f, _parameters.drrBrightness - 1.0f);
        this->update();
    } else if (e->key() == Qt::Key_Period) {
        _parameters.drrBrightness = _parameters.drrBrightness + 1.0f;
        this->update();
    } else if (e->key() == Qt::Key_Slash) {
        _parameters.drrBrightness = Parameters().drrBrightness;
        this->update();
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
    } else if (e->key() == Qt::Key_V) {
        _overlayValueActive = !_overlayValueActive;
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
    } else if (e->key() == Qt::Key_F2) {
        saveView(false);
    } else if (e->key() == Qt::Key_F3) {
        saveView(true);
    } else if (e->key() == Qt::Key_F9) {
        copyView(false);
    } else if (e->key() == Qt::Key_F10) {
        copyView(true);
    } else {
        QOpenGLWidget::keyPressEvent(e);
    }
}

void QV::mouseMoveEvent(QMouseEvent* e)
{
    _mousePos = e->pos();
    if (_overlayValueActive || _overlayHistogramActive)
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
