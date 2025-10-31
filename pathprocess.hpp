#pragma once
// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include <QDir>
#include <QString>
#include <QVector>

inline QString processDataPath(const QVector<QString>& args) {
    const size_t argc = args.size();
    if (argc == 2) {
        QString tempPath{ args[1] };
        tempPath = tempPath.trimmed();
        if (tempPath.size() >= 2) {
            if (tempPath[1] != QChar(':') && (tempPath.left(2) != "./" && tempPath.left(2) != ".\\")) {
                tempPath = "./" + tempPath;
            }
        }
        else {
            tempPath = "./" + tempPath;
        }
        if ((!tempPath.endsWith("/")) && (!tempPath.endsWith("\\"))) {
            tempPath += "/";
        }
        return tempPath;
    }
    return DEFAULT_DATA_DIRECTORY;
}
