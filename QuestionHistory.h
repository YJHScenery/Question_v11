// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#pragma once
#include "global.h"
#include <QDateTime>
#include <QVector>
#include <QJsonObject>
#include "ClassGroup.h"
#include "Student.h"

namespace QK {

    // 单次抽取记录
    struct QuestionRecord {
        QDateTime timestamp;       // 抽取时间
        QString className;         // 班级名称
        quint32 count;             // 抽取人数
        QString mode;              // 抽取模式
        QVector<Student> results;  // 抽取结果
        QString customNote;        // 备注

        QJsonObject toJson() const;

        static QuestionRecord fromJson(const QJsonObject& obj);
    };

    class QuestionHistory {
    public:
        explicit QuestionHistory(const QString& dataPath = DEFAULT_HISTORY_DIRECTORY);

        void addRecord(const QuestionRecord& record);

        [[nodiscard]]
        QVector<QuestionRecord> getAllRecords() const;

        [[nodiscard]]
        QVector<QuestionRecord> getRecordsByClass(const QString& className) const;

        bool clearAllRecords();

        bool clearAllRecordsByClass(const QString& className);

        XlsxIEState exportToXlsx(const QString& filePath) const;

        XlsxIEState exportToXlsx(const QString& className, const QString& filePath) const;

        static void processExportResult(QVector<Student>& studentsRef, const QuestionRecord& record);

        static QVector<QString> processHistoryFiles(const QString& folderPath);

        static QString processFilePath(const QString& historyPath, const QString& className);

        QString processFilePath(const QString& className) const;

    private:
        QString m_historyFilePath;

        QSet<QString> m_recordsClassFileNames;

        QVector<QuestionRecord> m_records;

        void loadFromJson();

        void saveAllToJson() const;

        void saveToJson(const QString& className) const;
    };

} // namespace QK