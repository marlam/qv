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

#include <filesystem>
#include <QImage>
#include <QPainter>
#include <QStringList>

#include "overlay-info.hpp"


static std::string humanReadableMemsize(unsigned long long size)
{
    const double dsize = size;
    const unsigned long long u1024 = 1024;
    char s[32];

    if (size >= u1024 * u1024 * u1024 * u1024) {
        std::snprintf(s, sizeof(s), "%.2f TiB", dsize / (u1024 * u1024 * u1024 * u1024));
    } else if (size >= u1024 * u1024 * u1024) {
        std::snprintf(s, sizeof(s), "%.2f GiB", dsize / (u1024 * u1024 * u1024));
    } else if (size >= u1024 * u1024) {
        std::snprintf(s, sizeof(s), "%.2f MiB", dsize / (u1024 * u1024));
    } else if (size >= u1024) {
        std::snprintf(s, sizeof(s), "%.2f KiB", dsize / u1024);
    } else if (size > 1 || size == 0) {
        std::snprintf(s, sizeof(s), "%d bytes", int(size));
    } else {
        std::strcpy(s, "1 byte");
    }
    return s;
}

static QStringList createList(const TAD::TagList& tl, QString& interpretation)
{
    QStringList list;
    for (auto it = tl.cbegin(); it != tl.cend(); it++) {
        if (QString(it->first.c_str()) == "INTERPRETATION") {
            interpretation = it->second.c_str();
        } else {
            list.append(QString("  %1=%2").arg(it->first.c_str()).arg(it->second.c_str()));
        }
    }
    return list;
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
        line.prepend(QString(" file %1/%2:").arg(set.fileIndex() + 1).arg(set.fileCount()));
    sl << line;
    line = QString(" %1x%2, %3 x %4 (%5)").arg(frame->width()).arg(frame->height())
        .arg(frame->channelCount()).arg(TAD::typeToString(frame->type()))
        .arg(humanReadableMemsize(frame->array().dataSize()).c_str());
    if (file->frameCount(errMsg) != 1) {
        QString frameDesc = QString(" frame %1/").arg(file->frameIndex() + 1);
        if (file->frameCount(errMsg) > 1) {
            frameDesc += QString("%1").arg(file->frameCount(errMsg));
        } else {
            if (!file->haveSeenLastFrame())
                frameDesc += QString(">=");
            frameDesc += QString("%1").arg(file->maxFrameIndexSoFar() + 1);
        }
        frameDesc += QString(":");
        line.prepend(frameDesc);
    }
    sl << line;
    sl << QString(" current channel: %1").arg(frame->currentChannelName().c_str());
    if (array.globalTagList().size() > 0) {
        QString line = " global: ";
        QString interpretation;
        QStringList list = createList(array.globalTagList(), interpretation);
        if (interpretation.length() > 0) {
            line.append(QString("INTERPRETATION="));
            line.append(interpretation);
        }
        sl << line << list;
    }
    for (size_t i = 0; i < 2; i++) {
        if (array.dimensionTagList(i).size() > 0) {
            QString line = QString(" %1 axis: ").arg(i == 0 ? 'x' : 'y');
            QString interpretation;
            QStringList list = createList(array.dimensionTagList(i), interpretation);
            if (interpretation.length() > 0) {
                line.append(QString("INTERPRETATION="));
                line.append(interpretation);
            }
            sl << line << list;
        }
    }
    for (size_t i = 0; i < array.componentCount(); i++) {
        if (array.componentTagList(i).size() > 0) {
            QString line = QString(" channel %1: ").arg(i);
            QString interpretation;
            QStringList list = createList(array.componentTagList(i), interpretation);
            if (interpretation.length() > 0) {
                line.append(QString("INTERPRETATION="));
                line.append(interpretation);
            }
            sl << line << list;
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
