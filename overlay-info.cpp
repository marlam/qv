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

#include <filesystem>
#include <QImage>
#include <QPainter>

#include "overlay-info.hpp"


static void addTagList(const TAD::TagList& tl, QString& line)
{
    for (auto it = tl.cbegin(); it != tl.cend(); it++) {
        if (it != tl.cbegin())
            line += ", ";
        if (QString(it->first.c_str()) == "INTERPRETATION") {
            line += QString(it->second.c_str());
        } else {
            line += QString("%1=%2").arg(it->first.c_str()).arg(it->second.c_str());
        }
    }
}

void OverlayInfo::update(int widthInPixels, Set& set)
{
    File* file = set.currentFile();
    Frame* frame = file->currentFrame();
    const TAD::ArrayContainer& array = frame->array();

    std::string errMsg;
    QStringList sl;
    QString line;

    line = QString(" ") + std::filesystem::path(file->fileName()).filename().string().c_str();
    if (set.fileCount() > 1)
        line.prepend(QString(" file %1/%2:").arg(set.fileIndex()).arg(set.fileCount()));
    sl << line;
    line = QString(" %1x%2, %3 x %4").arg(frame->width()).arg(frame->height())
        .arg(frame->channelCount()).arg(TAD::typeToString(frame->type()));
    if (file->frameCount(errMsg) > 1) {
        line.prepend(QString(" frame %1/%2:").arg(file->frameIndex()).arg(file->frameCount(errMsg)));
    }
    sl << line;
    sl << QString(" current channel: %1").arg(frame->currentChannelName().c_str());
    if (array.globalTagList().size() > 0) {
        QString line(" global: ");
        addTagList(array.globalTagList(), line);
        sl << line;
    }
    for (size_t i = 0; i < 2; i++) {
        if (array.dimensionTagList(i).size() > 0) {
            QString line = QString(" %1 axis: ").arg(i == 0 ? 'x' : 'y');
            addTagList(array.dimensionTagList(i), line);
            sl << line;
        }
    }
    for (size_t i = 0; i < array.componentCount(); i++) {
        if (array.componentTagList(i).size() > 0) {
            QString line = QString(" channel %1: ").arg(i);
            addTagList(array.componentTagList(i), line);
            sl << line;
        }
    }

    prepare(widthInPixels, _painter->fontInfo().pixelSize() * (sl.size() + 0.5f));

    float xOffset = 0.0f;
    for (int line = 0; line < sl.size(); line++) {
        float yOffset = (line + 1.25f) * _painter->fontInfo().pixelSize();
        _painter->drawText(xOffset, yOffset, sl[line]);
    }

    fixFormat();
    uploadImageToTexture();
}
