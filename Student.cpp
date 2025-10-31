// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

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
#include <QMap>
#include <QDebug>
#include <utility>

#include "Student.h"

namespace QK {
    Student::Student(QString name, QString id, const StudentType type, const StudentGender gender):
        m_name(std::move(name)), m_id(std::move(id)),m_type(type), m_gender(gender) { }

    Student::Student(QString name, QString id) :
        Student(std::move(name), std::move(id), StudentType::Excluded, StudentGender::Unknown) { }

    Student::Student(const QVector<QString> &serialization): 
        m_name(serialization.at(STUDENT_NAME_INDEX)),
        m_id(serialization.at(STUDENT_ID_INDEX))
    {
        if (serialization[STUDENT_TYPE_INDEX].toInt() >= 0 && serialization[STUDENT_TYPE_INDEX].toInt() <= 3) {
            m_type = static_cast<StudentType>(serialization[STUDENT_TYPE_INDEX].toInt());
        }
        if (serialization[STUDENT_GENDER_INDEX].toInt() >= 0 && serialization[STUDENT_GENDER_INDEX].toInt() <= 2) {
            m_gender = static_cast<StudentGender>(serialization[STUDENT_GENDER_INDEX].toInt());
        }
        const auto size = serialization.size();
        for (int i = STUDENT_FIRST_PROPERTY_INDEX; i < size; ++i) {
            m_properties.append(serialization.at(i));
        }
    }

    const QString &Student::getName() const {
        return m_name;
    }

    const QString &Student::getID() const {
        return m_id;
    }

    StudentType Student::getType() const {
        return m_type;
    }

    QString Student::getTypeString() const {
        static const QMap<StudentType, QString> studentTypeMap {
            {StudentType::Excluded, "Excluded"},
            {StudentType::Junior, "Junior"},
            {StudentType::Common, "Common"},
            {StudentType::Senior, "Senior"}
        };
        return studentTypeMap.value(m_type);
    }

    StudentGender Student::getGender() const {
        return m_gender;
    }

    QString Student::getGenderString() const {
        static const QMap<StudentGender, QString> studentGenderMap{
            {StudentGender::Unknown, "Unknown"}, 
            {StudentGender::Boy, "Boy"},
            {StudentGender::Girl, "Girl"}
        };
        return studentGenderMap.value(m_gender);
    }

    QVector<QString> Student::getProperties() const{
        return m_properties;
    }

    QVector<QString> &Student::getPropertiesRef() {
        return m_properties;
    }

    void Student::setName(const QString &name) {
        m_name = name;
    }

    void Student::setType(const StudentType studentType) {
        m_type = studentType;
    }

    void Student::setGender(const StudentGender gender) {
        m_gender = gender;
    }

    void Student::addProperty(const QString &property) {
        if (!(m_properties.contains(property))) {
            m_properties.append(property);
        }
    }

    void Student::deleteProperty(const QString &property) {
        m_properties.removeAll(property);
    }

    void Student::clearAllProperties() {
        m_properties.clear();
    }

    StudentType Student::stringToType(const QString& typeString)
    {
        const static QMap<QString, StudentType> types {
            {"Excluded", StudentType::Excluded},
            {"Junior", StudentType::Junior},
            {"Common", StudentType::Common},
            {"Senior", StudentType::Senior}
        };
        return types.value(typeString);
    }

    QVector<QString> Student::serialize() const {
        QVector<QString> serialization;
        serialization.append(m_name);
        serialization.append(m_id);
        serialization.append(QString::number(static_cast<int>(m_type)));
        serialization.append(QString::number(static_cast<int>(m_gender)));
        for (const QString& prop : m_properties) {
            serialization.append(prop);
        }
        return serialization;
    }

    QDebug operator<<(QDebug debug, const Student &student) {
        const QVector<QString> serialization = student.serialize();
        for (const QString &data: serialization) {
            debug << data;
        }
        return debug;
    }

    bool operator==(const Student &lhs, const Student &rhs) {
        return lhs.getID() == rhs.getID();
    }

    QDataStream & operator<<(QDataStream &out, const Student &s) {
        out << s.serialize();
        return out;
    }

    QDataStream & operator>>(QDataStream &in, Student &s) {
        QVector<QString> serialization;
        in >> serialization;
        s = Student(serialization);
        return in;
    }
}
