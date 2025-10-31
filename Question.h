// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include <random>

#include "global.h"
#include <QString>
#include <optional>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QVector>
#elif QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #include <QList>
    using QVector = QList;
#else
    #error Current Qt Version is unsupported. please use Qt 5.10 or later
#endif

#include <QtTypes>
#include "ClassGroup.h"
#include "Student.h"

#define TRAVERSAL_QUESTION_VERSION 2 

namespace QK {
    namespace QKRandom {
        // 在 [min, max] 之间生成 count 个随机数
        extern QVector<quint32> generateRandoms(quint32 min, quint32 max, quint32 count);

        // 在 students 中随机抽取 count 个学生，使用 StudentType 属性作为权重
        extern QVector<Student> generateRandomsStudentsWithWeight(
            quint32 count, const QVector<Student> &students);

        // 在 students 中随机抽取 count 个学生，平均分布
        extern QVector<Student> generateRandomsStudents(
            quint32 count, const QVector<Student> &students);

        // 生成一个长度为 length 的由数字组成的随机符串
        extern QString generateDigitalString(quint32 length);
    }

    // 定义了非加权抽取时的模式枚举类型
    enum class QuestionMode: quint32 {
        General,
        All,
        Boys,
        Girls,
        AllBoys, 
        AllGirls
    };

    // 定义了加权抽取时的模式枚举类型
    enum class QuestionModeWithWeight : quint32 {
        General,
        Boys, 
        Girls
    };

    class Question {
    public:
        // 禁用默认构造函数
        Question() = delete;

        ~Question() = default;

        explicit Question(const QVector<ClassGroup>& classGroups);

        explicit Question(const QString& directory);

        // 每个班级的默认文件名是此班级的 m_name 通过 utf-8 编码后的 SHA256 值，加上 .qcg 后缀。
        // 此函数用于生成这个字符串
        [[nodiscard]] QString getCurrentFileNameHash() const;

        // 初始化，若 m_classGroups 不为空，则将 m_currentClassGroupPtr 指向第 0 个元素，否则弹出警告窗口
        void initialize();

        // 初始化，将 m_currentClassGroupPtr 指向第 index 个元素，若该元素不存在，则抛出异常
        void initialize(qsizetype index);

        // 初始化，将将 m_currentClassGroupPtr 指向 className 对应的 ClassGroup，若不存在，则抛出异常
        void initialize(const QString& className);

        // 追加一个班级，并将 m_currentClassGroupPtr 指向新追加的班级
        bool append(const ClassGroup& classGroup);

        // 选择下标为 index 的班级，并将 m_currentClassGroupPtr 指向该班级，若不存在则不进行任何操作
        void choose(qsizetype index);

        // 选择 className 对应的班级，并将 m_currentClassGroupPtr 指向该班级，若不存在则不进行任何操作
        void choose(const QString& className);

        void removePlaceHolder();

        // 将 m_currentClassGroupPtr 指向的班级保存到文件中。若 m_currentClassGroupPtr 为空，则不进行任何操作
        [[nodiscard]] bool saveCurrentToFile() const;

        // 检查是否初始化，即检查 m_currentClassGroupPtr 是否为空，若为空则返回 false，否则返回 true
        [[nodiscard]] bool isInitialized() const;

        // 锁定当前班级
        void lockCurrent();

        // 从内存和文件中删除该班级，并将 m_currentClassGroupPtr 设置为空。
        [[nodiscard]]
        bool removeCurrent();

        [[nodiscard]]
        // 获取 m_classGroups 的大小
        qsizetype getSize() const;

        [[nodiscard]]
        // 获取 m_currentClassGroupPtr 的拷贝，可以通过它对当前指向的班级进行操作
        ClassGroup* getCurrentPtr() const;

        [[nodiscard]]
        // 获取 m_classGroups 的拷贝
        QVector<QString> getClassGroupNameList() const;

        [[nodiscard]]
        // 平均分布，使用 mode 提供模式
        QVector<Student> question(quint32 count, QuestionMode mode);

        [[nodiscard]]
        // 加权抽取，使用 mode 提供模式
        QVector<Student> questionWithWeight(quint32 count, QuestionModeWithWeight mode) const;

        // 依照属性抽取，该方法是均匀随机
        [[nodiscard]]
        QVector<Student> questionBasedOnProperty(quint32 count, const QString& property) const;

        // 依照属性抽取，该方法是均匀随机，且包含 Excluded 
        [[nodiscard]]
        QVector<Student> questionBasedOnPropertyAll(quint32 count, const QString& property) const;

        // 依照属性抽取，该方法是加权随机
        [[nodiscard]]
        QVector<Student> questionBasedOnPropertyWithWeight(quint32 count, const QString& property) const;

#if TRAVERSAL_QUESTION_VERSION == 1
        // 遍历抽取，除非手动重置，否则在一个周期内，每个学生只能被抽取一次
        // 每次使用此函数都将重写文件信息。
        [[nodiscard]]
        Student questionTraversal(const QString& dataPath, const QString& customID);

        // 获得遍历数据
        [[nodiscard]]
        QVector<Student> getNotQuestioned(const QString& dataPath) const;
#elif TRAVERSAL_QUESTION_VERSION == 2

        [[nodiscard]]
        Student questionTraversal(const QString& dataPath, const QString& customID);

        [[nodiscard]]
        QVector<Student> getNotQuestioned(const QString& dataPath) const;

        [[nodiscard]]
        QString getTraversalFileName(const QString &dataPath) const;
#endif
    private:
        // 储存所有班级
        QVector<ClassGroup> m_classGroups{};

        // 当前指向的班级
        std::optional<ClassGroup*> m_crtCgPtrOptional{ nullptr };

#if TRAVERSAL_QUESTION_VERSION == 2
        QVector<QString> readTraversalData(const QString& filename, bool& fileValid) const;

        void writeTraversalData(const QString& filename, const QVector<QString>& questionedIDs) const;

        QVector<Student> getUnquestionedStudents(const ClassGroup* cg, const QVector<QString>& questionedIDs) const;
#endif
    };
}
