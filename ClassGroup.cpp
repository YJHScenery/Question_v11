// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include <utility>
#include <stdexcept>

#include <QSet>
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
#include <QByteArray>
#include <QtTypes>
#include <QFile>
#include <QStringList>

#if __has_include(<QMessageBox>)
    #include <QMessageBox>
    #define Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP 1
#else 
    #define Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP 0
#endif

#include "ClassGroup.h"
#include "Student.h"
#include "sha512.h"


#ifdef QUESTION_11_XLSX_ENABLED
    #include "QXlsx/xlsxdocument.h"
#else
    #warning "QUESTION_11_PRO_XLSX_REVISE_PREMISSION is not defined, so functions related to XLSX will be ignored. If you have import the \"QXlsx\" Widget, Please define thie macro and include related headers. "
#endif


namespace QK {
    ClassGroup::ClassGroup(QString name, QString description, const QString &password, const QVector<Student>& studentList): m_name(std::move(name)),
        m_description(std::move(description)) 
    {
        QByteArray salt;
        m_passwordHash = sha512WithSalt(password, salt);

        if (!studentList.empty()) {
            m_unlocked = true;
            for (const Student& stu : studentList) {
                this->addStudent(stu);
            }
            this->lock();
        }
    }

    ClassGroup::ClassGroup(const ClassGroup& cg):
        m_name(cg.m_name), 
        m_description(cg.m_description),
        m_passwordHash(cg.m_passwordHash), 
        m_unlocked(cg.m_unlocked),
        m_students(cg.m_students)
    {    }

    ClassGroup& ClassGroup::operator=(const ClassGroup& otherCG)
    {
        if (this != &otherCG) {
            ClassGroup tempCG(otherCG);
            swap(m_name, tempCG.m_name);
            swap(m_description, tempCG.m_description);
            swap(m_passwordHash, tempCG.m_passwordHash);
            std::swap(m_unlocked, tempCG.m_unlocked);
            std::swap(m_students, tempCG.m_students);
        }
        return *this;
    }

    bool ClassGroup::checkUnlock() const 
    {
        return m_unlocked;
    }

    size_t ClassGroup::size() const 
    {
        return m_students.size();
    }

    size_t ClassGroup::getBoyCount() const
    {
        return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getGender() == StudentGender::Boy && stu.getType() != StudentType::Excluded; });
    }

    size_t ClassGroup::getBoyWithExcludedCount() const
    {
        return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getGender() == StudentGender::Boy; });
    }

    size_t ClassGroup::getGirlCount() const
    {
        return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getGender() == StudentGender::Girl && stu.getType() != StudentType::Excluded; });
    }

    size_t ClassGroup::getGirlWithExcludedCount() const
    {
        return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getGender() == StudentGender::Girl; });
    }

    size_t ClassGroup::getExcludedCount() const
    {
		return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getType() == StudentType::Excluded; });
    }

    size_t ClassGroup::getIncludedCount() const
    {
        return std::ranges::count_if(m_students, [](const Student& stu) { return stu.getType() != StudentType::Excluded; });
    }

    size_t ClassGroup::getSpecificCount(const QString& property) const
    {
        return std::ranges::count_if(m_students, [&property](const Student& stu) { return stu.getProperties().contains(property); });
    }

    QVector<QString> ClassGroup::getAllProperties() const
    {
        QSet<QString> propertySet;
        for (const Student& stu : m_students) {
            for (QString property : stu.getProperties()) {
                property = property.trimmed();
                propertySet.insert(property);
            }
        }
        // 避免容器本身的复制
        return QVector<QString>(propertySet.begin(), propertySet.end());
    }

    void ClassGroup::lock()
    {
        m_unlocked = false;
    }

    bool ClassGroup::unlock(const QString &password) 
    {
        m_unlocked = verifyPassword(password, m_passwordHash);
        return m_unlocked;
    }

    CGSerState ClassGroup::serialize(QDataStream &out) const
    {
        out << CLASS_GROUP_DATA_MAGIC_NUMBER;
        out << CLASS_GROUP_DATA_VERSION;
        out << m_name << m_description << m_passwordHash;
        out << static_cast<quint32>(m_students.size());
        for (const Student &student: m_students) {
            out << student.serialize();
        }
        if (out.status() != QDataStream::Ok) {
            return CGSerState::FileError;
		}
        return CGSerState::Success;
    }

    CGSerState ClassGroup::deserialize(QDataStream &in)
    {
        quint32 magicNumber;
        quint8 version;
        in >> magicNumber;
        in >> version;
        if (magicNumber != CLASS_GROUP_DATA_MAGIC_NUMBER) {
            in.setStatus(QDataStream::ReadCorruptData);
            return CGSerState::FormatError;
        }
        if (version > CLASS_GROUP_DATA_VERSION)
        {
            in.setStatus(QDataStream::ReadCorruptData);
			return CGSerState::VersionError;
        }
        in >> m_name >> m_description >> m_passwordHash;
        quint32 size;
        in >> size;
        m_students.clear();
        m_students.reserve(size);
        for (quint32 i = 0; i < size; ++i) {
            QVector<QString> studentSerialization;
            in >> studentSerialization;
            m_students.append(Student(studentSerialization));
        }
        if (in.status() != QDataStream::Ok) {
            return CGSerState::FileError;
        }
		return CGSerState::Success;
    }

    bool ClassGroup::saveToFile(const QString &fileName) const
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            return false;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_10);
        const CGSerState state{ this->serialize(out) };
        const bool success{ file.flush() };
        file.close();
        if (state == CGSerState::Success) {
            return success;
        }
        return false & success;
    }

    bool ClassGroup::loadFromFile(const QString &fileName) 
    {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            return false;
        }
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_10);
        const CGSerState state{ this->deserialize(in) };
        file.close();
        return (in.status() == QDataStream::Ok) && (state == CGSerState::Success);
    }

#ifdef QUESTION_11_XLSX_ENABLED
    XlsxIEState ClassGroup::importFromXlsx(const QString& xlsxFileName, const QString& password)
    {
        const QXlsx::Document xlsx(xlsxFileName);
        if (!xlsx.load()) { 
            return XlsxIEState::ImportFileError;
        }
        m_students.clear(); 
        QString tempName{ xlsx.read("A1").toString() };
        if (tempName.isEmpty()) {
            tempName = "UnnamedClass-count-" + QString::number(xlsx.dimension().rowCount() - 2);
        }
        m_name = tempName;
        m_description = xlsx.read("E1").toString();
        QByteArray array;
        m_passwordHash = sha512WithSalt(password, array);
        int row = 3;
        m_unlocked = true;
        while (true) {
            // Student s;
            QString name{ xlsx.read(row, 1).toString() };
            if (name.isEmpty()) {
                break; 
            }
            QString id{ xlsx.read(row, 2).toString() };
            int typeInt{ xlsx.read(row, 3).toInt() };
            int genderInt{ xlsx.read(row, 4).toInt() };
            QVector<QString> properties;
            int propertyColumn{ 5 };
            while (true) {
                QString property{xlsx.read(row, propertyColumn).toString()};
                ++propertyColumn;
                if (property.isEmpty()) {
                    break;
                }
                properties.append(property);
            }
            StudentType type{ StudentType::Excluded };
            StudentGender gender{ StudentGender::Unknown };
            if (typeInt >= 0 && typeInt <= 3) {
                type = static_cast<StudentType>(typeInt);
            }
            if (genderInt >= 0 && genderInt <= 2) {
                gender = static_cast<StudentGender>(genderInt);
            }
            Student s(name, id, type, gender);
            for (const QString& property : properties) {
                s.addProperty(property);
            }
            if (s.getID().isEmpty()) {
                continue;
            }
            ++row;
            bool addSuccess{ this->addStudent(s) };
            if (!addSuccess && this->size() == MAX_CLASS_GROUP_SIZE) {
                m_unlocked = false;
                return XlsxIEState::ImportReachMax;
            }
        }
        m_unlocked = false;
        return XlsxIEState::ImportSuccess;
    }

    XlsxIEState ClassGroup::exportToXlsx(const QString& xlsxFileName, const bool includeTip) const
    {
        return ClassGroup::exportToXlsx(m_name, m_students, xlsxFileName, includeTip, m_description);
    }

    XlsxIEState ClassGroup::exportToXlsx(const QString& className, const QVector<Student>& studentList, const QString& xlsxFileName, const bool includeTip, const QString& description)
    {

		QString tempClassName = className;
        if (tempClassName.isEmpty())
        {
			tempClassName = "EmptyNameListCount_" + QString::number(studentList.size());
        }
        const QString suffix {".xlsx"};
        QString fileNameWithXlsx{ xlsxFileName };
        if (!fileNameWithXlsx.endsWith(suffix, Qt::CaseInsensitive)) {
            fileNameWithXlsx.append(suffix);
        }
        QXlsx::Document xlsx;
        QXlsx::Format titleFormat;
        titleFormat.setFontBold(true);
        titleFormat.setFontSize(12);
        xlsx.write("A1", tempClassName, titleFormat);
        if (!description.isEmpty()) {
            xlsx.write("E1", description);
        }
        if (includeTip) {
            QString tipContent;
            QFile tipFile(":/text/resources/export_tip.txt");
            if (tipFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&tipFile);
                in.setEncoding(QStringConverter::Utf8); // 设置UTF-8编码（防中文乱码）
                tipContent = in.readAll(); // 读取全部内容
                tipFile.close();
                // content = QString();
            }
            xlsx.write("F1", tipContent);

            // xlsx.mergeCells("E1:E30");
        }
        xlsx.mergeCells("A1:D1"); // 合并首行
        // xlsx.setColumnWidth(5, 100);
        QStringList headers = { "Name", "ID", "Type", "Gender" };
        for (int col = 0; col < headers.size(); ++col) {
            xlsx.write(2, col + 1, headers[col]);
        }
        int row = 3;
        for (const Student& student : studentList) {
            xlsx.write(row, 1, student.getName());
            xlsx.write(row, 2, student.getID());
            xlsx.write(row, 3, static_cast<int>(student.getType()));
            xlsx.write(row, 4, static_cast<int>(student.getGender()));
            QVector<QString> properties{ student.getProperties() };
            const qsizetype propertySize{properties.size()};
            for (qsizetype i = 0; i < propertySize; ++i) {
                xlsx.write(row, i + 5, properties[i]);
            }
            ++row;
        }
        for (int col = 1; col <= 4; ++col) {
            xlsx.setColumnWidth(col, 15);
        }
        return xlsx.saveAs(fileNameWithXlsx) ? XlsxIEState::ExportSuccess : XlsxIEState::ExportFileError;
    }
#endif // QUESTION_11_XLSX_ENABLED

    const QString &ClassGroup::getName() const
    {
        return m_name;
    }

    const QString &ClassGroup::getDescription() const
    {
        return m_description;
    }

    const QString &ClassGroup::getPasswordHash() const
    {
        return m_passwordHash;
    }

    const QVector<Student> &ClassGroup::getAllStudents() const 
    {
        return m_students;
    }

    const QVector<QString> ClassGroup::getIDs() const
    {
        QVector<QString> resultIDs;
        for (const Student& stu : m_students) {
            resultIDs.append(stu.getID());
        }
        return resultIDs;
    }

    QVector<Student> ClassGroup::getExcludedStudents() const 
    {
        QVector<Student> resultStudents;
        for (const Student &student: m_students) {
            if (student.getType() == StudentType::Excluded) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getIncludedStudents() const 
    {
        QVector<Student> resultStudents;
        for (const Student &student: m_students) {
            if (student.getType() != StudentType::Excluded) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getBoyStudents() const 
    {
        QVector<Student> resultStudents;
        for (const Student &student: m_students) {
            if (student.getGender() == StudentGender::Boy && student.getType() != StudentType::Excluded) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getBoysWithExcluded() const
    {
        QVector<Student> resultStudents;
        for (const Student& student : m_students) {
            if (student.getGender() == StudentGender::Boy) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getGirlStudents() const 
    {
        QVector<Student> resultStudents;
        for (const Student &student: m_students) {
            if (student.getGender() == StudentGender::Girl && student.getType() != StudentType::Excluded) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getGirlsWithExcluded() const
    {
        QVector<Student> resultStudents;
        for (const Student& student : m_students) {
            if (student.getGender() == StudentGender::Girl) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    QVector<Student> ClassGroup::getSpecificStudents(const QString &property) const 
    {
        QVector<Student> resultStudents;
        for (const Student &student: m_students) {
            if (const QVector<QString> &allProperties = student.getProperties();
                allProperties.contains(property)) {
                resultStudents.append(student);
            }
        }
        return resultStudents;
    }

    Student &ClassGroup::getStudentRef(const QString &id)
    {
        static Student dummyStudent("Invalid Student Reference", "invalid_id_content", StudentType::Excluded, StudentGender::Unknown);
        if (m_unlocked) {
            for (Student &student: m_students) {
                if (student.getID() == id) {
                    return student;
                }
            }
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Error", "[[Student &ClassGroup::getStudentRef(const QString &id)]]: Student Not Found");
            return dummyStudent;
#else
            throw std::out_of_range("Student Not Found");
#endif
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
            return dummyStudent;
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
    }

    Student& ClassGroup::getStudentRef(int index)
    {
        static Student dummyStudent("Invalid Student Reference", "invalid_id_content", StudentType::Excluded, StudentGender::Unknown);
        if (m_unlocked) {
            if (index < 0 || index >= m_students.size()) {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
                QMessageBox::critical(nullptr, "Error", "[[Student& ClassGroup::getStudentRef(int index)]]: Incorrect index!");
#else
                throw std::out_of_range("Incorrect index!");
#endif
            }
            return m_students[index];
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
            return dummyStudent;
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
            
        }
    }

    Student ClassGroup::getStudentCopy(int index)
    {
        if (index < 0 || index >= m_students.size()) {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Error", "[[Student ClassGroup::getStudentCopy(int index)]]: Incorrect index!");
#else
            throw std::out_of_range("Incorrect index!");
#endif
        }
        return m_students[index];
    }

    Student ClassGroup::getStudentCopy(const QString& id)
    {
        for (Student& student : m_students) {
            if (student.getID() == id) {
                return student;
            }
        }
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
        QMessageBox::critical(nullptr, "Error", "[[Student ClassGroup::getStudentCopy(const QString& id)]]: Student Not Found");
        return Student("Invalid Student Copy", "invalid_id_content", StudentType::Excluded, StudentGender::Unknown);
#else
        throw std::out_of_range("Student Not Found");
#endif
    }

    void ClassGroup::reviseDescription(const QString &description) 
    {
        if (m_unlocked) {
            m_description = description;
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
    }

    bool ClassGroup::addStudent(const Student &student)
    {
        if (m_unlocked) {
            if (m_students.size() >= MAX_CLASS_GROUP_SIZE) {
                return false;
            }
            const QString& id = student.getID();
            if (id.isEmpty()) {
                return false;
            }
            if (m_studentIDs.contains(id)) { // O(1) 查找
                return false;
            }
            m_students.append(student);
            m_studentIDs.insert(id); // 同步添加到哈希表
            return true;
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
            return false;
    }

    bool ClassGroup::addStudentsVector(const QVector<Student>& students)
    {
        // When at least one student is added to "m_students", it returns true, otherwise false
        if (m_unlocked) {
            bool saveSuccess{ false };
            for (const Student& stu : students) {
                saveSuccess |= this->addStudent(stu);
            }
            return saveSuccess;
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
            return false;
    }

    bool ClassGroup::changeStudents(const QVector<Student>& students)
    {
        if (m_unlocked) {
            m_students.clear();
            return this->addStudentsVector(students);
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
            return false;
    }

    bool ClassGroup::changePassword(const QString& oldP, const QString& newP)
    {
        bool success{verifyPassword(oldP, m_passwordHash)};
        if (success) {
            QByteArray array;
            m_passwordHash = sha512WithSalt(newP, array);
        }
        return success;
    }

    bool QK::ClassGroup::removeStudent(const Student& student)
    {
        if (m_unlocked) {
            return this->removeStudent(student.getID());
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
            return false;
    }

    bool QK::ClassGroup::removeStudent(const QString& id)
    {
        if (m_unlocked) {
            int index = -1;
            for (int i = 0; i < m_students.size(); ++i) {
                if (m_students[i].getID() == id) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                m_students.removeAt(index);
                m_studentIDs.remove(id);
                return true;
            }
            return false;
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
            return false;
    }

    void ClassGroup::removeAllStudents()
    {
        if (m_unlocked) {
            m_students.clear();
            m_studentIDs.clear();
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_CLASSGROUP
            QMessageBox::critical(nullptr, "Lock", "ClassGroup is not unlocked!");
#else
            throw std::runtime_error("ClassGroup is not unlocked!");
#endif
        }
    }

    bool ClassGroup::id_exists(const QString &id) const 
    {
        auto exists{ std::ranges::find(m_students, Student("NONE", id)) };
        if (exists != m_students.end()) {
            return true;
        }
        else {
            return false;
        }
    }

    const Student & ClassGroup::operator[](const size_t index) 
    {
        return m_students[index];
    }



    QDebug operator<<(QDebug debug, const ClassGroup &classGroup) 
    {
        debug << "ClassGroup:" << classGroup.getName() << "\t";
        debug << "Description:" << classGroup.getDescription() << "\n";
		debug << "Size: " << classGroup.size() << "\n";
        debug << "Students: \n";
        for (const Student &student: classGroup.getAllStudents()) {
            debug << student.getName() << " " << student.getID() << "\n" << student.getTypeString() << "\n" << student.getGenderString();
        }
        return debug;
    }

    bool operator==(const ClassGroup &classGroup1, const ClassGroup &classGroup2) 
    {
        return classGroup1.getName() == classGroup2.getName();
    }
}
