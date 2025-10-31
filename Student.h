// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
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
#include <QMap>
#include <QDataStream>

namespace QK {
    constexpr quint32 STUDENT_NAME_INDEX{ 0 };
    constexpr quint32 STUDENT_ID_INDEX{ 1 };
    constexpr quint32 STUDENT_TYPE_INDEX{ 2 };
    constexpr quint32 STUDENT_GENDER_INDEX{ 3 };
    constexpr quint32 STUDENT_FIRST_PROPERTY_INDEX{ 4 };

    enum class StudentType: quint32 {
        Excluded,
        Junior,
        Common,
        Senior,
    };

    enum class StudentGender: quint32 {
        Unknown,
        Boy,
        Girl
    };

    class Student {
    public:
        // 由于 ID 是不可缺少的，所以禁止使用默认构造函数
        Student() = delete;

        Student(QString name, QString id, StudentType type, StudentGender gender);

        Student(QString name, QString id);

        // 使用一个 QVector<QString> 来反序列化生成一个 Student 对象
        explicit Student(const QVector<QString> &serialization);

        [[nodiscard]] 
        const QString& getName() const;

        [[nodiscard]] 
        const QString& getID() const;

        [[nodiscard]] 
        StudentType getType() const;

        [[nodiscard]] 
        QString getTypeString() const;

        [[nodiscard]] 
        StudentGender getGender() const;

        [[nodiscard]] 
        QString getGenderString() const;

        [[nodiscard]] 
        QVector<QString> getProperties() const;

        [[nodiscard]] 
        QVector<QString> &getPropertiesRef();

        void setName(const QString &name);

        void setType(StudentType studentType);

        void setGender(StudentGender gender);

        void addProperty(const QString &property);

        void deleteProperty(const QString &property);

        void clearAllProperties();

        static StudentType stringToType(const QString& typeString);

        // 序列化一个 Student 对象
        // m_type 和 m_gender 将会以对应十进制整数的字符串进行存储。
        [[nodiscard]] 
        QVector<QString> serialize() const;

        // friend statement
        friend QDebug operator<<(QDebug debug, const Student &student);

        friend bool operator==(const Student &lhs, const Student &rhs);

    private:
        QString m_name; 
        QString m_id; // 不可被修改
        StudentType m_type{StudentType::Excluded}; 
        StudentGender m_gender{StudentGender::Unknown};
        QVector<QString> m_properties;
    };

    QDebug operator<<(QDebug debug, const Student &student);

    // 当 ID 相同时，认为两个 Student 对象相等
    bool operator==(const Student &lhs, const Student &rhs);
    QDataStream& operator<<(QDataStream& out, const Student& s);
    QDataStream& operator>>(QDataStream& in, Student& s);
}

