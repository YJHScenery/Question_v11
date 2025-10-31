// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#pragma once
#include "global.h"
#include <utility>

#include <QString>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QVector>
#elif QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #include <QList>
    using QVector = QList;
#else
    #error Current Qt Version is unsupported. please use Qt 5.10 or later
#endif
#include <QDebug>
#include <QDataStream>
#include <QSet>

#include "Student.h"

namespace QK {

#ifdef QUESTION_11_XLSX_ENABLED
    enum class CGSerState : quint32 {
        // 定义了序列化和反序列化操作的状态，用于指示操作的结果。
        UnknownError,
        Success,
        FileError,
        FormatError,
        VersionError,
	};

    enum class XlsxIEState : quint32 {
        // 定义了 Xlsx 数据的导入导出状态，用于指示导入导出操作的结果。
        UnknownError,
        ImportSuccess,
        ImportFileError,
        ImportReachMax,
        ExportSuccess,
        ExportFileError,
    };
#endif // QUESTION_11_XLSX_ENABLED

    class ClassGroup {
        // 包含班级信息：名称、介绍、密码、学生列表
        // 如果需要修改学生信息，请使用函数 "getStudentRef()"
    public:
        // 默认构造函数，受到类逻辑的影响，其后必须使用 ".loadFromFile()"。否则将会出现问题
        ClassGroup() = default;

        // 此构造函数用于在代码中直接创建班级，你可能会在临时班级中创建它。
        ClassGroup(QString name, QString description, const QString& password, const QVector<Student>& studentList = {});

        // 深拷贝
        ClassGroup(const ClassGroup& cg);

        ClassGroup& operator=(const ClassGroup& otherCG);

        [[nodiscard]] 
        bool checkUnlock() const;

        [[nodiscard]] 
        size_t size() const;

        [[nodiscard]]
        size_t getBoyCount() const;

        [[nodiscard]]
		size_t getBoyWithExcludedCount() const;

		[[nodiscard]]
		size_t getGirlCount() const;

        [[nodiscard]]
		size_t getGirlWithExcludedCount() const;

        [[nodiscard]]
		size_t getExcludedCount() const;

        [[nodiscard]]
        size_t getIncludedCount() const;

        [[nodiscard]]
        size_t getSpecificCount(const QString &property) const;

        [[nodiscard]]
        QVector<QString> getAllProperties() const;

        void lock();

        // 如果密码正确，则将 m_unlocked 设置为 true
        bool unlock(const QString &password);

        // 序列化对象，使之能被 QDataStream 读取
        CGSerState serialize(QDataStream &out) const;

        // 反序列化，从 QDataStream 读取对象
        CGSerState deserialize(QDataStream &in);

        // 将对象保存到文件中
        [[nodiscard]] 
        bool saveToFile(const QString &fileName) const;

        // 从文件中加载班级。推荐在默认构造函数后使用，否则该对象的内容将被覆盖
        [[nodiscard]] 
        bool loadFromFile(const QString &fileName);

#ifdef QUESTION_11_XLSX_ENABLED
        
        // 从表格中加载班级
        [[nodiscard]]
        XlsxIEState importFromXlsx(const QString& xlsxFileName, const QString& password);

		[[nodiscard]]
        static XlsxIEState exportToXlsx(const QString& className, const QVector<Student>& studentList, const QString& xlsxFileName, bool includeTip = true, const QString& description = {});

        [[nodiscard]]
        XlsxIEState exportToXlsx(const QString& xlsxFileName, bool includeTip = true) const;

#endif // QUESTION_11_XLSX_ENABLED
        [[nodiscard]]
        const QString &getName() const;

        [[nodiscard]] 
        const QString &getDescription() const;

        [[nodiscard]] 
        const QString &getPasswordHash() const;

        [[nodiscard]] 
        const QVector<Student> &getAllStudents() const;

        [[nodiscard]] 
        const QVector<QString> getIDs() const;

        [[nodiscard]] 
        QVector<Student> getExcludedStudents() const;

        [[nodiscard]] 
        QVector<Student> getIncludedStudents() const;

        [[nodiscard]] 
        QVector<Student> getBoyStudents() const;

        [[nodiscard]]
        QVector<Student> getBoysWithExcluded() const;

        [[nodiscard]] 
        QVector<Student> getGirlStudents() const;

        [[nodiscard]]
        QVector<Student> getGirlsWithExcluded() const;

        [[nodiscard]] 
        QVector<Student> getSpecificStudents(const QString &property) const;

        [[nodiscard]] 
        Student &getStudentRef(const QString &id);

        [[nodiscard]] 
        Student& getStudentRef(int index);

        [[nodiscard]]
        Student getStudentCopy(int index);

        [[nodiscard]]
        Student getStudentCopy(const QString& id);

        void reviseDescription(const QString &description);

        bool addStudent(const Student &student);

        bool addStudentsVector(const QVector<Student>& students);
        
        bool changeStudents(const QVector<Student>& students); 

        bool changePassword(const QString& oldP, const QString& newP);

        bool removeStudent(const Student &student);

        bool removeStudent(const QString &id);

        void removeAllStudents();

        [[nodiscard]] bool id_exists(const QString &id) const;

        const Student& operator[](size_t index);

        friend QDebug operator<<(QDebug debug, const ClassGroup &classGroup);

        friend bool operator==(const ClassGroup &classGroup1, const ClassGroup &classGroup2);

    private:
        bool m_unlocked{ false };
        QString m_name; // The Class Group's unique identifier, which is not permitted to be revised
        QString m_description;
        QString m_passwordHash;
        QVector<Student> m_students; // Store all students
        QSet<QString> m_studentIDs;
    };

    QDebug operator<<(QDebug debug, const ClassGroup &classGroup);
    bool operator==(const ClassGroup &classGroup1, const ClassGroup &classGroup2);
}

