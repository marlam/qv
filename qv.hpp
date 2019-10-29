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

#ifndef QV_HPP
#define QV_HPP

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>

#include "set.hpp"
#include "overlay-help.hpp"
#include "overlay-info.hpp"
#include "overlay-value.hpp"
#include "overlay-statistic.hpp"
#include "overlay-histogram.hpp"
#include "overlay-colormap.hpp"


class QV : public QOpenGLWidget
{
Q_OBJECT

private:
    Set& _set;
    int _w, _h;
    unsigned int _fbo;
    unsigned int _fboTex;
    unsigned int _vao;
    QOpenGLShaderProgram _viewPrg;
    QOpenGLShaderProgram _overlayPrg;
    QOpenGLShaderProgram _quadTreePrg;
    bool _dragMode;
    QPoint _dragStart;
    QPoint _mousePos;
    bool _overlayHelpActive;
    OverlayHelp _overlayHelp;
    bool _overlayInfoActive;
    OverlayInfo _overlayInfo;
    bool _overlayValueActive;
    OverlayValue _overlayValue;
    bool _overlayStatisticActive;
    OverlayStatistic _overlayStatistic;
    bool _overlayHistogramActive;
    OverlayHistogram _overlayHistogram;
    bool _overlayColorMapActive;
    OverlayColorMap _overlayColorMap;

    void updateTitle();
    void navigationParameters(Frame* frame,
            int widgetWidth, int widgetHeight,
            float& xFactor, float& yFactor,
            float& xOffset, float& yOffset);
    QPoint dataCoordinates(QPoint widgetCoordinates,
            int widgetWidth, int widgetHeight,
            int frameWidth, int frameHeight,
            float xFactor, float yFactor,
            float xOffset, float yOffset);
    void prepareQuadRendering(Frame* frame, int quadTreeLevel,
            float xFactor, float yFactor,
            float xOffset, float yOffset);
    void renderQuad(Frame* frame, int quadTreeLevel, int qx, int qy,
            float quadFactorX, float quadFactorY,
            float quadOffsetX, float quadOffsetY);
    void renderFrame(Frame* frame, int quadTreeLevel,
            float xFactor, float yFactor,
            float xOffset, float yOffset);
    QImage renderFrameToImage(Frame* frame);

    void openFile();
    void closeFile();
    void reloadFile();
    void adjustFileIndex(int offset);
    void adjustFrameIndex(int offset);
    void setChannelIndex(int index);
    void adjustZoom(int steps);
    void adjustVisInterval(int minSteps, int maxSteps);
    void resetVisInterval();
    void changeColorMap(ColorMapType type);
    void saveView(bool pure);
    void copyView(bool pure);

public:
    QV(Set& set);

    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;

    virtual void keyReleaseEvent(QKeyEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void wheelEvent(QWheelEvent* e) override;
};

#endif
