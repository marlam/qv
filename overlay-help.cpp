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

#include <QImage>
#include <QPainter>

#include "overlay-help.hpp"


OverlayHelp::OverlayHelp()
{
    _helpText << " General:                  View, zoom, move:";
    _helpText << " F1: toggle help           l: toggle lin. interpolation";
    _helpText << " Q: quit                   g: toggle magnification grid";
    _helpText << " F11: toggle fullscreen    -,+,=: zoom out/in/reset";
    _helpText << " ESC: end fullscr./quit    mouse,␣: move/reset";
    _helpText << " r: reload current file";
    _helpText << "";
    _helpText << " Switch files and frames:  Switch channels:";
    _helpText << " <,>: previous/next file   c: color channel";
    _helpText << " ←,→: 1 frame back/fwd     0-9: this channel";
    _helpText << " ↑,↓: 10 frames back/fwd";
    _helpText << " ⇞,⇟: 100 frames back/fwd";
    _helpText << "";
    _helpText << " Analysis tools:           Adjust displayed interval:";
    _helpText << " i: toggle info            (,): shift left/right";
    _helpText << " s: toggle statistics      {,}: dec./inc. lower bound";
    _helpText << " h: toggle histogram       [,]: dec./inc. upper bound";
    _helpText << " m: toggle color map       \\: reset";
    _helpText << "";
    _helpText << " Choose color map:";
    _helpText << " F4: disable color map";
    _helpText << " F5: use sequential color map (press again to cycle)";
    _helpText << " F6: use diverging color map (press again to cycle)";
    _helpText << " F7: use qualitative color map";
    _helpText << " F8: use color map from clipboard in CSV format";

    int lines = _helpText.size();
    heightInPixels = painter->fontInfo().pixelSize() * (lines + 0.5f);
}

OverlayHelp::~OverlayHelp()
{
}

void OverlayHelp::update(int widthInPixels)
{
    Overlay::prepare(widthInPixels);

    float xOffset = 0.0f;
    for (int line = 0; line < _helpText.size(); line++) {
        float yOffset = (line + 1.25f) * painter->fontInfo().pixelSize();
        painter->drawText(xOffset, yOffset, _helpText[line]);
    }

    setAlpha(0, 0, image->width(), image->height(), 2 * 255 / 3);
    Overlay::uploadImageToTexture();
}
