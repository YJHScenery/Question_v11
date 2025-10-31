// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"
#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QString>
#include <QStyleFactory>
#include <QScopedPointer>

#include "Question.h"
#include "QuestionHistory.h"
#include "questionmainwindow.h"
#include "pathprocess.hpp"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    const QString path{ processDataPath(app.arguments()) };
    QK::Question questionObject(path);
    QK::QuestionHistory historyObject(DEFAULT_HISTORY_DIRECTORY);
    QTranslator translator;
    if (translator.load("Chinese_zh_CN.qm")) {
        app.installTranslator(&translator);
    }
    app.setWindowIcon(QIcon(":/icons/resources/QuestionIcon11.png"));
    QScopedPointer<QuestionMainWindow> window{new QuestionMainWindow(questionObject, historyObject)};
    window->show();
    return app.exec(); 
}

