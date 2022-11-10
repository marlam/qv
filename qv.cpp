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

#include "qv.hpp"
#include "gl.hpp"


QV::QV(Set& set, QWidget* parent) :
    QOpenGLWidget(parent),
    _set(set),
    _dragMode(false),
    overlayInfoActive(false),
    overlayValueActive(false),
    overlayStatisticActive(false),
    overlayHistogramActive(false),
    overlayColorMapActive(false)
{
    setMouseTracking(true);
    window()->setWindowIcon(QIcon(":aux/qv-logo-512.png"));
    updateTitle();

    setMinimumSize(_overlayFallback.size());
    File* file = _set.currentFile();
    Frame* frame = (file ? file->currentFrame() : nullptr);
    if (frame) {
        QSize frameSize(frame->width(), frame->height());
        QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
        QSize maxSize = 0.9f * screenSize;
        if (frameSize.width() < maxSize.width() && frameSize.height() < maxSize.height())
            _sizeHint = frameSize;
        else
            _sizeHint = frameSize.scaled(maxSize, Qt::KeepAspectRatio);
    } else {
        _sizeHint = minimumSize();
    }
}

void QV::updateView()
{
    emit parametersChanged();
    this->update();
}

void QV::updateTitle()
{
    std::string s = _set.currentDescription();
    if (s.size() == 0)
        s = "qv";
    else
        s += " - qv";
    window()->setWindowTitle(s.c_str());
}

QSize QV::sizeHint() const
{
    return _sizeHint;
}

// Helper: read file into string (for shader loading)
static QString readFile(QString fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    return in.readAll();
}

void QV::initializeGL()
{
    ASSERT_GLCHECK();

    auto gl = getGlFunctionsFromCurrentContext();

    bool contextIsOk = (context()->isValid() && context()->format().majorVersion() >= 3);
    if (contextIsOk) {
        GLint maxTexSize = 0;
        gl->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        contextIsOk = (maxTexSize >= Frame::requiredMaxTextureSize);
    }
    if (!contextIsOk) {
        QMessageBox::critical(this, "Error", "Insufficient OpenGL capabilities.");
        std::exit(1);
    }
#if 0
    int red_bits, green_bits, blue_bits;
    gl->glGetIntegerv(GL_RED_BITS, &red_bits);
    gl->glGetIntegerv(GL_GREEN_BITS, &green_bits);
    gl->glGetIntegerv(GL_BLUE_BITS, &blue_bits);
    fprintf(stderr, "%d %d %d\n", red_bits, green_bits, blue_bits);
#endif

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
    xFactor = arFactorX / _set.currentParameters()->zoom;
    yFactor = arFactorY / _set.currentParameters()->zoom;
    xOffset = 2.0f * _set.currentParameters()->xOffset / widgetWidth;
    yOffset = 2.0f * _set.currentParameters()->yOffset / widgetHeight;
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

void QV::prepareQuadRendering(Frame* frame, int quadTreeLevel,
        float xFactor, float yFactor,
        float xOffset, float yOffset)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    gl->glUseProgram(_viewPrg.programId());
    // Quadtree limits
    _viewPrg.setUniformValue("quadCoveredDataWidth", std::pow(2.0f, float(quadTreeLevel)) * frame->quadWidth());
    _viewPrg.setUniformValue("quadCoveredDataHeight", std::pow(2.0f, float(quadTreeLevel)) * frame->quadHeight());
    _viewPrg.setUniformValue("dataWidth", float(frame->width()));
    _viewPrg.setUniformValue("dataHeight", float(frame->height()));
    // Navigation and zoom
    _viewPrg.setUniformValue("xFactor", xFactor);
    _viewPrg.setUniformValue("yFactor", yFactor);
    _viewPrg.setUniformValue("xOffset", xOffset);
    _viewPrg.setUniformValue("yOffset", yOffset);
    _viewPrg.setUniformValue("magGrid", _set.currentParameters()->magGrid);
    // Min/max values
    float visMinVal = _set.currentParameters()->visMinVal(frame->channelIndex());
    float visMaxVal = _set.currentParameters()->visMaxVal(frame->channelIndex());
    if (!std::isfinite(visMinVal) || !std::isfinite(visMaxVal)) {
        visMinVal = frame->visMinVal(frame->channelIndex());
        visMaxVal = frame->visMaxVal(frame->channelIndex());
        _set.currentParameters()->setVisMinVal(frame->channelIndex(), visMinVal);
        _set.currentParameters()->setVisMaxVal(frame->channelIndex(), visMaxVal);
    }
    _viewPrg.setUniformValue("visMinVal", visMinVal);
    _viewPrg.setUniformValue("visMaxVal", visMaxVal);
    // Dynamic Range Reduction
    _viewPrg.setUniformValue("dynamicRangeReduction", _set.currentParameters()->dynamicRangeReduction);
    _viewPrg.setUniformValue("drrBrightness", _set.currentParameters()->drrBrightness);
    // Color and data information
    _viewPrg.setUniformValue("colorMap", _set.currentParameters()->colorMap().type() != ColorMapNone);
    _viewPrg.setUniformValue("showColor", frame->channelIndex() == ColorChannelIndex);
    _viewPrg.setUniformValue("colorSpace", int(frame->colorSpace()));
    _viewPrg.setUniformValue("channelCount", frame->channelCount());
    _viewPrg.setUniformValue("dataChannelIndex", frame->channelCount() <= 4 ? frame->channelIndex() : 0);
    _viewPrg.setUniformValue("colorChannel0Index", frame->colorChannelIndex(0));
    _viewPrg.setUniformValue("colorChannel1Index", frame->colorChannelIndex(1));
    _viewPrg.setUniformValue("colorChannel2Index", frame->colorChannelIndex(2));
    _viewPrg.setUniformValue("alphaChannelIndex", frame->alphaChannelIndex());
    _viewPrg.setUniformValue("colorWas8Bit", frame->type() == TGD::uint8);
    _viewPrg.setUniformValue("colorWas16Bit", frame->type() == TGD::uint16);
    _viewPrg.setUniformValue("texIsSRGB", frame->channelCount() <= 4 && frame->type() == TGD::uint8
            && (frame->colorSpace() == ColorSpaceSGray || frame->colorSpace() == ColorSpaceSRGB));
    // Textures
    _viewPrg.setUniformValue("tex0", 0);
    _viewPrg.setUniformValue("tex1", 1);
    _viewPrg.setUniformValue("tex2", 2);
    _viewPrg.setUniformValue("alphaTex", 3);
    _viewPrg.setUniformValue("colorMapTex", 4);
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
    gl->glBindVertexArray(_vao);
    ASSERT_GLCHECK();
}

void QV::renderQuad(Frame* frame, int quadTreeLevel, int qx, int qy,
        float quadFactorX, float quadFactorY,
        float quadOffsetX, float quadOffsetY)
{
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    _viewPrg.setUniformValue("quadFactorX", quadFactorX);
    _viewPrg.setUniformValue("quadFactorY", quadFactorY);
    _viewPrg.setUniformValue("quadOffsetX", quadOffsetX);
    _viewPrg.setUniformValue("quadOffsetY", quadOffsetY);
    bool showColor = (frame->channelIndex() == ColorChannelIndex);
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
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _set.currentParameters()->magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE1);
    gl->glBindTexture(GL_TEXTURE_2D, t1);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _set.currentParameters()->magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE2);
    gl->glBindTexture(GL_TEXTURE_2D, t2);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _set.currentParameters()->magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glActiveTexture(GL_TEXTURE3);
    gl->glBindTexture(GL_TEXTURE_2D, t3);
    gl->glActiveTexture(GL_TEXTURE4);
    gl->glBindTexture(GL_TEXTURE_2D, _set.currentParameters()->colorMap().texture());
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _set.currentParameters()->magInterpolation ? GL_LINEAR : GL_NEAREST);
    gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    ASSERT_GLCHECK();
}

void QV::renderFrame(Frame* frame, int quadTreeLevel,
        float xFactor, float yFactor,
        float xOffset, float yOffset)
{
    prepareQuadRendering(frame, quadTreeLevel, xFactor, yFactor, xOffset, yOffset);
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
            renderQuad(frame, quadTreeLevel, qx, qy, quadFactorX, quadFactorY, quadOffsetX, quadOffsetY);
        }
    }
}

QImage QV::renderFrameToImage(Frame* frame)
{
    QGuiApplication::processEvents();
    makeCurrent();
    // We render into tiles that have the same size as the frame's quads,
    // and then combine the tiles into a QImage.
    ASSERT_GLCHECK();
    auto gl = getGlFunctionsFromCurrentContext();
    int _fboBak;
    gl->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_fboBak);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    gl->glBindTexture(GL_TEXTURE_2D, _fboTex);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->quadWidth(), frame->quadHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fboTex, 0);
    gl->glViewport(0, 0, frame->quadWidth(), frame->quadHeight());
    QImage img(frame->width(), frame->height(), QImage::Format_RGB888);
    TGD::Array<uint8_t> tmpArray({ size_t(frame->quadWidth()), size_t(frame->quadHeight()) }, 3);
    size_t lineSize = tmpArray.dimension(0) * tmpArray.elementSize();
    if (lineSize % 4 == 0)
        gl->glPixelStorei(GL_PACK_ALIGNMENT, 4);
    else if (lineSize % 2 == 0)
        gl->glPixelStorei(GL_PACK_ALIGNMENT, 2);
    else
        gl->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    prepareQuadRendering(frame, 0, 1.0f, 1.0f, 0.0f, 0.0f);
    for (int tileY = 0; tileY < frame->quadTreeLevelHeight(0); tileY++) {
        for (int tileX = 0; tileX < frame->quadTreeLevelWidth(0); tileX++) {
            renderQuad(frame, 0, tileX, tileY, 1.0f, 1.0f, 0.0f, 0.0f);
            gl->glReadPixels(0, 0, frame->quadWidth(), frame->quadHeight(), GL_RGB, GL_UNSIGNED_BYTE, tmpArray.data());
            int copyableLines = frame->quadHeight();
            if (tileY * frame->quadHeight() + copyableLines > frame->height())
                copyableLines = frame->height() - tileY * frame->quadHeight();
            int copyableColumns = frame->quadWidth();
            if (tileX * frame->quadWidth() + copyableColumns > frame->width())
                copyableColumns = frame->width() - tileX * frame->quadWidth();
            for (int y = 0; y < copyableLines; y++) {
                const uint8_t* src = tmpArray.get<uint8_t>({ 0, size_t(y) });
                int dstX = tileX * frame->quadWidth();
                int dstY = tileY * frame->quadHeight() + y;
                dstY = frame->height() - 1 - dstY; // reverse y!
                uint8_t* dst = img.scanLine(dstY) + dstX * tmpArray.elementSize();
                std::memcpy(dst, src, copyableColumns * tmpArray.elementSize());
            }
        }
    }
    gl->glBindFramebuffer(GL_FRAMEBUFFER, _fboBak);
    ASSERT_GLCHECK();
    QGuiApplication::processEvents();
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
    ASSERT_GLCHECK();
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (!frame) {
        _overlayFallback.update(_w);
        int overlayYOffset = std::max((_h - _overlayFallback.heightInPixels()) / 2, 0);
        gl->glViewport(0, overlayYOffset, _w, _overlayFallback.heightInPixels());
        gl->glUseProgram(_overlayPrg.programId());
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, _overlayFallback.texture());
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    } else {
        int overlayYOffset = 0;
        if (overlayColorMapActive) {
            _overlayColorMap.update(_w, *(_set.currentParameters()));
            gl->glViewport(0, overlayYOffset, _w, _overlayColorMap.heightInPixels());
            gl->glUseProgram(_overlayPrg.programId());
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, _overlayColorMap.texture());
            gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            overlayYOffset += _overlayColorMap.heightInPixels();
        }
        if (overlayHistogramActive) {
            _overlayHistogram.update(_w, dataCoords, _set);
            gl->glViewport(0, overlayYOffset, _w, _overlayHistogram.heightInPixels());
            gl->glUseProgram(_overlayPrg.programId());
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, _overlayHistogram.texture());
            gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            overlayYOffset += _overlayHistogram.heightInPixels();
        }
        if (overlayStatisticActive) {
            _overlayStatistic.update(_w, _set);
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
    }
    gl->glDisable(GL_BLEND);
    QGuiApplication::restoreOverrideCursor();
    ASSERT_GLCHECK();
}

bool QV::haveCurrentFile() const
{
    return (_set.fileIndex() >= 0);
}

void QV::openFile()
{
    int previousFileCount = _set.fileCount();
    std::string errMsg;
    QStringList names = QFileDialog::getOpenFileNames(this);
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
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
    QGuiApplication::restoreOverrideCursor();
    this->updateTitle();
    this->updateView();
}

void QV::closeFile()
{
    if (haveCurrentFile()) {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        _set.removeFile(_set.fileIndex());
        QGuiApplication::restoreOverrideCursor();
        this->updateTitle();
        this->updateView();
    }
}

void QV::reloadFile()
{
    if (haveCurrentFile()) {
        std::string errMsg;
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        if (!_set.currentFile()->reload(errMsg)) {
            QMessageBox::critical(this, "Error", errMsg.c_str());
        }
        QGuiApplication::restoreOverrideCursor();
        this->updateTitle();
        this->updateView();
    }
}

void QV::adjustFileIndex(int offset)
{
    if (!haveCurrentFile())
        return;

    int i = _set.fileIndex();
    int ni = i + offset;
    if (ni < 0)
        ni = 0;
    else if (ni >= _set.fileCount())
        ni = _set.fileCount() - 1;
    if (ni != i) {
        std::string errMsg;
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        if (!_set.setFileIndex(ni, errMsg)) {
            QMessageBox::critical(this, "Error", (errMsg
                        + ".\n\nClosing this file.").c_str());
            _set.removeFile(ni);
        }
        QGuiApplication::restoreOverrideCursor();
        this->updateTitle();
        this->updateView();
    }
}

void QV::adjustFrameIndex(int offset)
{
    if (!haveCurrentFile())
        return;

    File* file = _set.currentFile();
    std::string errMsg;
    int fc = file->frameCount(errMsg);
    if (fc == 0) {
        QMessageBox::critical(this, "Error", (errMsg
                    + ".\n\nClosing this file.").c_str());
        _set.removeFile(_set.fileIndex());
        return;
    }
    int i = file->frameIndex();
    int ni = i + offset;
    if (ni < 0)
        ni = 0;
    else if (fc > 0 && ni >= fc)
        ni = fc - 1;
    if (ni != i) {
        if (fc > 0 || ni < i || ni <= file->maxFrameIndexSoFar()) {
            // we know the frame exists so we can jump right to it
            if (file->setFrameIndex(ni, errMsg)) {
                this->updateTitle();
                this->updateView();
            } else {
                QMessageBox::critical(this, "Error", (errMsg
                            + ".\n\nClosing this file.").c_str());
                _set.removeFile(_set.fileIndex());
                return;
            }
        } else {
            // we do not know the frame exists so we need to go
            // forward as long as there are more frames
            // first jump to the highest known frame index
            if (i < file->maxFrameIndexSoFar()) {
                if (!file->setFrameIndex(file->maxFrameIndexSoFar(), errMsg)) {
                    QMessageBox::critical(this, "Error", (errMsg
                                + ".\n\nClosing this file.").c_str());
                    _set.removeFile(_set.fileIndex());
                    return;
                }
                i = file->maxFrameIndexSoFar();
            }
            // then go forward frame by frame
            while (i < ni && file->hasMore()) {
                if (!file->setFrameIndex(i + 1, errMsg)) {
                    QMessageBox::critical(this, "Error", (errMsg
                                + ".\n\nClosing this file.").c_str());
                    _set.removeFile(_set.fileIndex());
                    return;
                }
                i++;
            }
            this->updateTitle();
            this->updateView();
        }
    }
}

void QV::setChannelIndex(int index)
{
    if (!haveCurrentFile())
        return;

    Frame* frame = _set.currentFile()->currentFrame();
    if (index == ColorChannelIndex) {
        if (frame->colorSpace() != ColorSpaceNone)
            frame->setChannelIndex(index);
    } else {
        if (index >= 0 && index < frame->channelCount())
            frame->setChannelIndex(index);
    }
    this->updateTitle();
    this->updateView();
}

void QV::adjustZoom(int steps)
{
    if (!haveCurrentFile())
        return;

    float adjustmentPerStep = std::max(0.000001f, _set.currentParameters()->zoom * 0.05f);
    float oldZoom = _set.currentParameters()->zoom;
    float newZoom = std::max(0.000001f, _set.currentParameters()->zoom - steps * adjustmentPerStep);
    _set.currentParameters()->xOffset = _set.currentParameters()->xOffset * oldZoom / newZoom;
    _set.currentParameters()->yOffset = _set.currentParameters()->yOffset * oldZoom / newZoom;
    _set.currentParameters()->zoom = newZoom;
    this->updateView();
}

void QV::adjustVisInterval(int minSteps, int maxSteps)
{
    if (!haveCurrentFile())
        return;

    Frame* frame = _set.currentFile()->currentFrame();
    float defaultVisMin = frame->currentVisMinVal();
    float defaultVisMax = frame->currentVisMaxVal();
    float adjustment = (defaultVisMax - defaultVisMin) / 100.0f;
    float oldMinVal = _set.currentParameters()->visMinVal(frame->channelIndex());
    float oldMaxVal = _set.currentParameters()->visMaxVal(frame->channelIndex());
    float newMinVal = oldMinVal + minSteps * adjustment;
    float newMaxVal = oldMaxVal + maxSteps * adjustment;
    if (newMinVal < defaultVisMin)
        newMinVal = defaultVisMin;
    else if (_set.currentParameters()->visMaxVal(frame->channelIndex()) - newMinVal < adjustment)
        newMinVal = _set.currentParameters()->visMaxVal(frame->channelIndex()) - adjustment;
    else if (newMinVal > defaultVisMax - adjustment)
        newMinVal = defaultVisMax - adjustment;
    if (newMaxVal < defaultVisMin + adjustment)
        newMaxVal = defaultVisMin + adjustment;
    else if (newMaxVal - _set.currentParameters()->visMinVal(frame->channelIndex()) < adjustment)
        newMaxVal = _set.currentParameters()->visMinVal(frame->channelIndex()) + adjustment;
    else if (newMaxVal > defaultVisMax)
        newMaxVal = defaultVisMax;
    _set.currentParameters()->setVisMinVal(frame->channelIndex(), newMinVal);
    _set.currentParameters()->setVisMaxVal(frame->channelIndex(), newMaxVal);
    this->updateView();
}

void QV::resetVisInterval()
{
    if (!haveCurrentFile())
        return;

    Frame* frame = _set.currentFile()->currentFrame();
    _set.currentParameters()->setVisMinVal(frame->channelIndex(), std::numeric_limits<float>::quiet_NaN());
    _set.currentParameters()->setVisMaxVal(frame->channelIndex(), std::numeric_limits<float>::quiet_NaN());
    this->updateView();
}

void QV::changeColorMap(ColorMapType type)
{
    if (!haveCurrentFile())
        return;

    ColorMapType oldType = _set.currentParameters()->colorMap().type();
    if (oldType != type) {
        _set.currentParameters()->colorMap().setType(type);
    } else {
        _set.currentParameters()->colorMap().cycle();
    }
    this->updateView();
}

void QV::saveView(bool pure)
{
    if (!haveCurrentFile())
        return;

    Frame* frame = _set.currentFile()->currentFrame();
    QString name = QFileDialog::getSaveFileName(this, QString(), QString(), "PNG images (*.png)");
    if (!name.isEmpty()) {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QImage img = (pure ? renderFrameToImage(frame) : grabFramebuffer());
        if (!img.save(name, "png")) {
            QMessageBox::critical(this, "Error", "Saving failed.");
        }
        QGuiApplication::restoreOverrideCursor();
    }
}

void QV::copyView(bool pure)
{
    if (!haveCurrentFile())
        return;

    Frame* frame = _set.currentFile()->currentFrame();
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::clipboard()->setImage(
            pure ? renderFrameToImage(frame) : grabFramebuffer());
    QGuiApplication::restoreOverrideCursor();
}

void QV::toggleLinearInterpolation()
{
    if (!haveCurrentFile())
        return;

    _set.currentParameters()->magInterpolation = !_set.currentParameters()->magInterpolation;
    this->updateView();
}

void QV::toggleGrid()
{
    if (!haveCurrentFile())
        return;

    _set.currentParameters()->magGrid = !_set.currentParameters()->magGrid;
    this->updateView();
}

void QV::resetZoom()
{
    if (!haveCurrentFile())
        return;

    _set.currentParameters()->zoom = 1.0f;
    this->updateView();
}

void QV::recenter()
{
    if (!haveCurrentFile())
        return;

    _set.currentParameters()->xOffset = 0.0f;
    _set.currentParameters()->yOffset = 0.0f;
    this->updateView();
}

void QV::toggleDRR()
{
    if (!haveCurrentFile())
        return;

    _set.currentParameters()->dynamicRangeReduction = !_set.currentParameters()->dynamicRangeReduction;
    this->updateView();
}

void QV::adjustDRRBrightness(int direction)
{
    if (!haveCurrentFile())
        return;

    if (direction == 0) { // reset
        _set.currentParameters()->drrBrightness = Parameters().drrBrightness;
    } else if (direction > 0) { // increase
        _set.currentParameters()->drrBrightness = _set.currentParameters()->drrBrightness * 2.0f;
    } else { // decrease
        _set.currentParameters()->drrBrightness = std::max(2.0f, _set.currentParameters()->drrBrightness / 2.0f);
    }
    this->updateView();
}

static bool frameIsPrettyBig(const Frame* frame)
{
    // This is of course just a heuristic since we cannot know how fast the machine
    // really is. I checked with my not entirely new laptop.
    return frame->quadTreeLevels() > 1
        || (size_t(frame->width()) * size_t(frame->height()) >
                size_t(Frame::requiredMaxTextureSize) * size_t(Frame::requiredMaxTextureSize) / 4);
}

void QV::toggleOverlayInfo()
{
    if (!haveCurrentFile())
        return;

    overlayInfoActive = !overlayInfoActive;
    this->updateView();
}

void QV::toggleOverlayStatistics()
{
    if (!haveCurrentFile())
        return;

    overlayStatisticActive = !overlayStatisticActive;
    Frame* frame = _set.currentFile()->currentFrame();
    if (overlayStatisticActive && frameIsPrettyBig(frame)
            && !frame->haveStatistic(frame->channelIndex())) {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    this->updateView();
}

void QV::toggleOverlayValue()
{
    if (!haveCurrentFile())
        return;

    overlayValueActive = !overlayValueActive;
    Frame* frame = _set.currentFile()->currentFrame();
    if (overlayValueActive && frameIsPrettyBig(frame)
            && frame->colorSpace() != ColorSpaceNone
            && !frame->haveLightness()) {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    this->updateView();
}

void QV::toggleOverlayHistogram()
{
    if (!haveCurrentFile())
        return;

    overlayHistogramActive = !overlayHistogramActive;
    Frame* frame = _set.currentFile()->currentFrame();
    if (overlayHistogramActive && frameIsPrettyBig(frame)
            && !frame->haveHistogram(frame->channelIndex())) {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    this->updateView();
}

void QV::toggleOverlayColormap()
{
    if (!haveCurrentFile())
        return;

    overlayColorMapActive = !overlayColorMapActive;
    this->updateView();
}

void QV::toggleApplyCurrentParametersToAllFiles()
{
    if (!haveCurrentFile())
        return;

    _set.toggleApplyCurrentParametersToAllFiles();
}

void QV::mouseMoveEvent(QMouseEvent* e)
{
    if (haveCurrentFile()) {
        _mousePos = e->pos();
        if (overlayValueActive || overlayHistogramActive)
            this->updateView();
        if (_dragMode) {
            QPoint dragEnd = e->pos();
            _set.currentParameters()->xOffset += dragEnd.x() - _dragStart.x();
            _set.currentParameters()->yOffset -= dragEnd.y() - _dragStart.y();
            _dragStart = dragEnd;
            this->updateView();
        }
    }
}

void QV::mousePressEvent(QMouseEvent* e)
{
    if (haveCurrentFile() && e->button() == Qt::LeftButton) {
        _dragMode = true;
        _dragStart = e->pos();
    }
}

void QV::mouseReleaseEvent(QMouseEvent* e)
{
    if (haveCurrentFile() && e->button() == Qt::LeftButton) {
        _dragMode = false;
    }
}

void QV::wheelEvent(QWheelEvent* e)
{
    if (haveCurrentFile()) {
        adjustZoom(e->angleDelta().y() / 120);
    }
}
