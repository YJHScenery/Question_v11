// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "QuestionHistory.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include "ClassGroup.h"

namespace QK {

    // 单次记录序列化
    QJsonObject QuestionRecord::toJson() const {
        QJsonObject obj;
        obj["timestamp"] = timestamp.toString("yyyy-MM-dd HH:mm:ss");
        obj["className"] = className;
        obj["count"] = static_cast<int>(count);
        obj["mode"] = mode;

        QJsonArray studentsArr;
        for (const auto& stu : results) {
            QJsonArray stuArr;
            stuArr.append(stu.getName());
            stuArr.append(stu.getID());
            stuArr.append(stu.getTypeString());
            studentsArr.append(stuArr);
        }
        obj["results"] = studentsArr;
        obj["note"] = customNote;
        return obj;
    }

    // 从JSON反序列化
    QuestionRecord QuestionRecord::fromJson(const QJsonObject& obj) {
        QuestionRecord record;
        record.timestamp = QDateTime::fromString(obj["timestamp"].toString(), "yyyy-MM-dd HH:mm:ss");
        record.className = obj["className"].toString();
        record.count = obj["count"].toInt();
        record.mode = obj["mode"].toString();
        record.customNote = obj["note"].toString();

        auto studentsArr = obj["results"].toArray();
        for (const auto& stuVal : studentsArr) {
            auto stuArr = stuVal.toArray();
            Student stu(
                stuArr[0].toString(),
                stuArr[1].toString(),
                Student::stringToType(stuArr[2].toString()),
                StudentGender::Unknown
            );
            record.results.append(stu);
        }
        return record;
    }

    // 历史记录管理类
    QuestionHistory::QuestionHistory(const QString& dataPath):
    m_historyFilePath(dataPath){
        if (!m_historyFilePath.endsWith("/") && !m_historyFilePath.endsWith("\\")){
            m_historyFilePath += "/";
        }
        QVector<QString> classNameBase64Code {processHistoryFiles(dataPath)};
        for (const QString& b64code: classNameBase64Code){
            const QByteArray byteArray = QByteArray::fromBase64(b64code.toUtf8());
            m_recordsClassFileNames.insert(QString(byteArray));
        }
        loadFromJson(); // 初始化时加载历史记录
    }

    void QuestionHistory::loadFromJson() {
        m_records.clear();
        for (const QString& className : m_recordsClassFileNames){
            QFile file(this->processFilePath(className));
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) return;

            const QByteArray data = file.readAll();
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isNull()) return;

            QJsonArray arr = doc.array();
            for (const QJsonValueRef& val : arr) {
                const QK::QuestionRecord record {QuestionRecord::fromJson(val.toObject())};
                m_records.append(record);
                m_recordsClassFileNames.insert(record.className);
            }
            file.close();
        }
    }

    void QuestionHistory::saveAllToJson() const {
        if (!QDir().mkpath(QFileInfo(m_historyFilePath).path())){
            qDebug() << "Create Path:" << m_historyFilePath;
        }
        for (const QString& className : m_recordsClassFileNames){
            this->saveToJson(className);
        }
    }

    void QuestionHistory::saveToJson(const QString& className) const
    {
        // if (std::ranges::count_if(m_records, [&](const auto& record) { return record.className == className; }) == 0 ){
        //     QFile::remove(this->processFilePath(className));
        //     return;
        // }
        QFile file(this->processFilePath(className));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
        QJsonArray arr;
        for (const auto& record : m_records) {
            arr.append(record.toJson());
        }
        const QJsonDocument doc(arr);
        file.write(doc.toJson());
        file.close();
    }

    void QuestionHistory::processExportResult(QVector<Student>& studentsRef, const QuestionRecord& record)
    {
        for (const auto& stu : record.results) {
            Student temp = stu;
            temp.addProperty(QString("班级：%1").arg(record.className));
            temp.addProperty(QString("抽取时间: %1").arg(record.timestamp.toString()));
            temp.addProperty(QString("模式: %1").arg(record.mode));
            studentsRef.append(temp);
        }
    }

    QString QuestionHistory::processFilePath(const QString& className) const
    {
        return processFilePath(m_historyFilePath, className);
    }

    QVector<QString> QuestionHistory::processHistoryFiles(const QString& folderPath)
    {
        QVector<QString> result;
        QDir targetDir(folderPath);
        if (!targetDir.exists()) {
            return result;
        }

        QFileInfoList fileList = targetDir.entryInfoList(
            QDir::Files | QDir::NoDotAndDotDot,  // 过滤器
            QDir::Name                           // 排序方式（按名称）
        );

        // 定义格式前缀和后缀
        const QString prefix = HISTORY_FILE_PREFIX;
        const QString suffix = HISTORY_FILE_SUFFIX;
        const int prefixLen = prefix.length();
        const int suffixLen = suffix.length();

        // 遍历所有文件
        for (const auto& fileInfo : fileList) {
            QString fileName = fileInfo.fileName();

            // 判断文件名是否符合格式 "HISTORY_*.history"
            if (fileName.startsWith(prefix) && fileName.endsWith(suffix)) {
                // 计算中间内容长度（确保有有效内容）
                if (fileName.length() > prefixLen + suffixLen) {
                    // 提取*部分内容（去掉前缀和后缀）
                    QString content = fileName.mid(prefixLen);  // 去掉前缀
                    content.chop(suffixLen);                    // 去掉后缀
                    result.append(content);
                } else {
                    // 处理特殊情况：中间无内容（如"HISTORY_.history"）
                    result.append("");
                }
            } else {
                // 不符合格式，删除文件
                QFile::remove(fileInfo.absoluteFilePath());
            }
        }

        return result;
    }

    QString QuestionHistory::processFilePath(const QString& historyPath, const QString& className)
    {
        QString historyPathStr = historyPath;
        if (!historyPath.endsWith('/')) {
            historyPathStr.append('/');
        }
        return QString("%1%2%3%4").arg(historyPathStr, HISTORY_FILE_PREFIX, QString(className.toUtf8().toBase64()), QString(HISTORY_FILE_SUFFIX));
    }

    void QuestionHistory::addRecord(const QuestionRecord& record) {
        m_records.prepend(record); // 最新记录放前面
        if (m_records.size() > MAX_QUESTION_HISTORY_SIZE) {
            m_records.resize(MAX_QUESTION_HISTORY_SIZE); // 限制最大记录数
        }
        m_recordsClassFileNames.insert(record.className);
        saveToJson(record.className);
    }

    QVector<QuestionRecord> QuestionHistory::getAllRecords() const {
        return m_records;
    }

    QVector<QuestionRecord> QuestionHistory::getRecordsByClass(const QString& className) const {
        QVector<QuestionRecord> res;
        for (const auto& record : m_records) {
            if (record.className == className) res.append(record);
        }
        return res;
    }

    bool QuestionHistory::clearAllRecords() {
        m_records.clear();
        saveAllToJson();
        return true;
    }

    bool QuestionHistory::clearAllRecordsByClass(const QString& className)
    {

        const qsizetype removeSuccess {m_records.removeIf([&className](const QuestionRecord& record){
            return record.className == className;
        })};
        return QFile::remove(this->processFilePath(className)) && removeSuccess;
    }

    XlsxIEState QuestionHistory::exportToXlsx(const QString& filePath) const {
#ifdef QUESTION_11_XLSX_ENABLED
        QVector<Student> allStudents;
        for (const auto& record : m_records) {
            processExportResult(allStudents, record);
        }
        return ClassGroup::exportToXlsx("抽取历史", allStudents, filePath, false);
#else
        return XlsxIEState::ExportFileError;
#endif
    }

    XlsxIEState QuestionHistory::exportToXlsx(const QString& className, const QString& filePath) const
    {
        QVector<Student> allStudents;
        for (const auto& record : m_records) {
            if (record.className == className){
                processExportResult(allStudents, record);
            }
        }
        return ClassGroup::exportToXlsx("抽取历史", allStudents, filePath, false);
#ifdef QUESTION_11_XLSX_ENABLED
#else
    return XlsxIEState::ExportFileError;
#endif
    }

} // namespace QK
