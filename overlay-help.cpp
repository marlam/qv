/*
 * Copyright (C) 2019, 2020 Computer Graphics Group, University of Siegen
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

#include "version.hpp"
#include "overlay-help.hpp"


OverlayHelp::OverlayHelp()
{
    _helpText << " General:                  Manage files:";
    _helpText << " F1: toggle help           o: open file(s)";
    _helpText << " Q: quit                   w: close current file";
    _helpText << " F11: toggle fullscreen    r: reload current file";
    _helpText << " ESC: end fullscreen/quit";
    _helpText << "";
    _helpText << " Switch files:             Switch frames:";
    _helpText << " ←,→: 1 file back/fwd      ⇧←,⇧→: 1 frame back/fwd";
    _helpText << " ↑,↓: 10 files back/fwd    ⇧↑,⇧↓: 10 frames back/fwd";
    _helpText << " ⇞,⇟: 100 files back/fwd   ⇧⇞,⇧⇟: 100 frames back/fwd";
    _helpText << "";
    _helpText << " Switch channels:          Zoom, move:";
    _helpText << " c: color channel          -,+,=: zoom out/in/reset";
    _helpText << " 0-9: this channel         mouse,␣: move/reset";
    _helpText << "";
    _helpText << " Magnified views:          Dynamic Range Reduction:";
    _helpText << " l: toggle interpolation   d: toggle DRR";
    _helpText << " g: toggle grid            ,,.: dec/inc DRR brightness";
    _helpText << "                           /: reset DRR brightness";
    _helpText << " Analysis tools:";
    _helpText << " i: toggle info            Select range for display:";
    _helpText << " v: toggle value inspect   {,}: dec/inc lower bound";
    _helpText << " s: toggle statistics      [,]: dec/inc upper bound";
    _helpText << " h: toggle histogram/range (,): shift left/right";
    _helpText << " m: toggle color map       \\: reset";
    _helpText << "";
    _helpText << " Color map:                Save/copy results:";
    _helpText << " F4: disable               F2: save current view";
    _helpText << " F5: sequential (cycle)    F3: save 1:1 view";
    _helpText << " F6: diverging (cycle)     F9: copy current view";
    _helpText << " F7: qualitative           F10: copy 1:1 view";
    _helpText << " F8: from clipboard (image or CSV)";
    _helpText << "";
    _helpText << " a: apply current view parameters to all files (toggle)";
    _helpText << "";
    _helpText << " qv " QV_VERSION " -- https://marlam.de/qv";
}

void OverlayHelp::update(int widthInPixels)
{
    prepare(widthInPixels, _painter->fontInfo().pixelSize() * (_helpText.size() + 0.5f));

    float xOffset = 0.0f;
    for (int line = 0; line < _helpText.size(); line++) {
        float yOffset = (line + 1.25f) * _painter->fontInfo().pixelSize();
        _painter->drawText(xOffset, yOffset, _helpText[line]);
    }

    fixFormat();
    uploadImageToTexture();
}

QSize OverlayHelp::size() const
{
    int heightInPixels = _painter->fontInfo().pixelSize() * (_helpText.size() + 0.5f);
    int longestLineLength = -1;
    for (int i = 0; i < _helpText.size(); i++) {
        if (_helpText[i].length() > longestLineLength)
            longestLineLength = _helpText[i].length();
    }
    QFontMetricsF fontMetrics(_painter->font(), _painter->device());
    int widthInPixels = fontMetrics.horizontalAdvance(' ') * (longestLineLength + 1 + 0.5f);
    return QSize(widthInPixels, heightInPixels);
}
