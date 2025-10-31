// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"
#include <algorithm>
#include <unordered_set>
#include <stdexcept>

#include <QString>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QSet>
#include <QtMath>
#include <QStringList>
#include <QPair>
#include "QuestionHistory.h"

#if __has_include(<QMessageBox>)
    #include <QMessageBox>
    #define Q_MESSAGE_BOX_EXISTED_FOR_QUESTION 1
#else 
    #define Q_MESSAGE_BOX_EXISTED_FOR_QUESTION 0
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QVector>
#elif QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #include <QList>
    using QVector = QList;
#else
    #error Current Qt Version is unsupported. please use Qt 5.10 or later
#endif

#include "Question.h"
#include "sha512.h"

namespace QK {

    static inline QString loadFileName(const QString& fileName) {
        return QString(DEFAULT_DATA_DIRECTORY + fileName + DATA_SUFFIX);
    }

    namespace QKRandom {
        QVector<quint32> generateRandoms(const quint32 min, const quint32 max, const quint32 count) 
        {
            // 检查参数正确性
            if (min > max) {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
                QMessageBox::critical(nullptr, "Error", "[[QVector<quint32> generateRandoms(const quint32 min, const quint32 max, const quint32 count)]]: min > max");
#else
                qCritical() << "[[QVector<quint32> generateRandoms(const quint32 min, const quint32 max, const quint32 count)]]: min > max");
#endif
                return {};
            }
            if (count == 0) return {};
            if (min == max) return QVector<quint32>{ min };
            const quint64 range{ max - min + 1 };
            if (count >= range) return {};

            // 创建随机数种子
            QRandomGenerator generator(QRandomGenerator::securelySeeded());
            std::unordered_set<quint32> uniqueSet;
            uniqueSet.reserve(count);

            // 过大的指定数字，将会修改算法，否则有大概率陷入无限循环
            while (uniqueSet.size() < count) {
                quint32 num = generator.bounded(min, max + 1);

                // std::pair::second 返回的是 bool 类型，表示插入是否成功
                auto [_, inserted] = uniqueSet.insert(num); // 结构化绑定

                if (static_cast<long double>(uniqueSet.size()) > static_cast<long double>(range) * 0.9) {
                    break;
                }
            }

            if (uniqueSet.size() < count) { // 没有达到指定数量，即指定数字过大。为了防止进入死循环，则采用下面的算法。
                QVector<quint32> allValues(range); 

                // std::iota: 生成连续递增的序列，min 为起始值。
                std::iota(allValues.begin(), allValues.end(), min);
                for (quint32 i = 0; i < range; ++i) {
                    quint32 j = generator.bounded(i, range);
                    std::swap(allValues[i], allValues[j]); // 将数字打乱。
                }
                for (quint32 val : allValues) {
                    if (uniqueSet.size() >= count) {
                        break;
                    }
                    uniqueSet.insert(val); // 如果这个 val 已经存在了，那么插入操作会失败。但是因为 allValues 包含所有的值，因此不会进入死循环。
                }
            }
            QVector<quint32> resultValues;
            for (const quint32& val : uniqueSet) {
                resultValues.push_back(val);
            }
            return resultValues;
        }

        QVector<Student> generateRandomsStudentsWithWeight(const quint32 count, const QVector<Student>& students) 
        {
            QVector<QPair<Student, int> > candidates;
            int totalWeight{ 0 };
            for (const Student& student : students) {
                const static QMap<StudentType, int> studentTypeWeight{ {StudentType::Excluded, 0}, { StudentType::Junior, 1 }, {StudentType::Common, 3}, {StudentType::Senior, 6} };
                int weight{ studentTypeWeight.value(student.getType())};
                candidates.append(qMakePair(student, weight)); // 绑定学生和数字权重
                totalWeight += weight;
            }

            if (candidates.size() < count) {
                return {};
            }

            QVector<Student> result;
            for (quint32 i = 0; i < count; ++i) {
                int randValue{ QRandomGenerator::global()->bounded(totalWeight) };
                int cumulative{ 0 };
                int index{ 0 };
                for (; index < candidates.size(); ++index) {
                    cumulative += candidates[index].second;
                    if (cumulative > randValue) {
                        break;
                    }
                }
                result.append(candidates[index].first);
                totalWeight -= candidates[index].second;
                candidates.remove(index);
            }
            return result;
        }

        QVector<Student> generateRandomsStudents(const quint32 count, const QVector<Student>& students) 
        {
            auto max{ static_cast<quint32>(students.size() - 1) };
            QVector<quint32> randomNumbers{
                generateRandoms(0, max, count)
            };
            QVector<Student> result;
            for (const auto& randomNumber : randomNumbers) {
                result.append(students[randomNumber]);
            }
            return result;
        }

        // 获取一个长为 length 的的随机十进制数字字符串
        QString generateDigitalString(quint32 length)
        {
            QString result;
            for (quint32 i = 0; i < length; ++i) {
                quint32 digit = QRandomGenerator::global()->bounded(10); // 生成 0~9
                result.append(QString::number(digit));
            }
            return result;
        }
    }


    Question::Question(const QVector<ClassGroup>& classGroups) 
    {
        for (const ClassGroup& classGroup : classGroups) {
            this->append(classGroup);
        }
    }

    Question::Question(const QString& directory) 
    {                                                                                                                                                                                                                      
        const QDir dataDir(directory);
        QStringList slist = dataDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for ( const QString & fileName: slist) {
            ClassGroup cg;
            bool success{ cg.loadFromFile(directory + fileName) };
            if (success) {
                // m_classGroups.append(cg);
                this->append(cg);
            }
        }
        if (m_classGroups.size() != 0) {
            this->initialize();
        }
    }

    QString Question::getCurrentFileNameHash() const 
    {
        if (m_crtCgPtrOptional.has_value()){
            const QByteArray data {m_crtCgPtrOptional.value()->getName().toUtf8()};
            return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
        }
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
        QMessageBox::critical(nullptr, "Error", "Question Object Has not initialized!");
#else
        qCritical() << "Question Object Has not initialized!";
#endif
        return QString();
    }


    void Question::initialize() 
    {
        if (!m_classGroups.empty()) {
            m_crtCgPtrOptional = &(m_classGroups.back());
            // m_currentClassGroupPtr = &m_classGroups.back();
        }
        else {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
            QMessageBox::critical(nullptr, "Question::initialize()", "ClassGroup list is empty");
#elif
            qCritical() << "ClassGroup list is empty";
#endif
        }
    }

    void Question::initialize(const qsizetype index) 
    {
        if (index < m_classGroups.size() && index >= 0) {
            m_crtCgPtrOptional = &(m_classGroups[index]);
            // m_currentClassGroupPtr = &m_classGroups[index];
        }
        else {
            this->initialize();
        }
    }

    void Question::initialize(const QString& className)
    {
		auto existed{ std::ranges::find(m_classGroups, ClassGroup(className, "", "")) };
        if (existed != m_classGroups.end()) {
            m_crtCgPtrOptional = &(*existed);
			return;
        }
        this->initialize();
    }

    bool Question::append(const ClassGroup& classGroup) 
    {
        auto existed{ std::ranges::find(m_classGroups, classGroup) };
        if (existed != m_classGroups.end()) {
            return false;
        }
        else {
            m_classGroups.append(classGroup);
            this->initialize(classGroup.getName());
            return true;
        }
    }

    void Question::choose(const qsizetype index) {
        if (index < m_classGroups.size() && index >= 0) 
        {
            m_crtCgPtrOptional = &(m_classGroups[index]);
        }
    }

    void Question::choose(const QString& className) 
    {
        auto existed{ std::ranges::find(m_classGroups, ClassGroup(className, "", ""))};
        if (existed != m_classGroups.end()) {
            m_crtCgPtrOptional = &(*existed);
            return;
		}
    }

    void Question::removePlaceHolder()
    {
        const auto it{std::ranges::find(m_classGroups, PLACEHOLDER_CLASS_DEFILE)};
        if (it != m_classGroups.end()){
            m_classGroups.erase(it);
            this->initialize();
        }
    }

    bool Question::saveCurrentToFile() const
    {
        if (this->isInitialized()) {
            const QString fileNameHash{ this->getCurrentFileNameHash() };
            const QString wholeFileName{ loadFileName(fileNameHash) };
            if (m_crtCgPtrOptional.has_value()){
                const bool save{m_crtCgPtrOptional.value()->saveToFile(wholeFileName)};
                return save;
            }
            return false;
        }
        return false;
    }

    bool Question::isInitialized() const
    {
        return m_crtCgPtrOptional.has_value();
    }

    void Question::lockCurrent()
    {
        if (m_crtCgPtrOptional.has_value()){
            m_crtCgPtrOptional.value()->lock();
        }
    }

    bool Question::removeCurrent() {
        if (m_crtCgPtrOptional.has_value() && m_crtCgPtrOptional.value()->checkUnlock()) {
            const QString clsName{ m_crtCgPtrOptional.value()->getName() };
            const QString fileNameHash{ this->getCurrentFileNameHash() };
            m_classGroups.removeAll(ClassGroup(clsName, "", ""));
            const bool successRemoveData{ QFile::remove(loadFileName(fileNameHash)) };
            const bool successRemoveTraversal{ QFile::remove(this->getTraversalFileName(TRAVERSAL_DIRECTORY)) };
            const bool successRemoveHistory{ QFile::remove(QuestionHistory::processFilePath(DEFAULT_HISTORY_DIRECTORY, clsName)) };
            // m_currentClassGroupPtr = nullptr;
            m_crtCgPtrOptional.reset();
            return successRemoveData || ( successRemoveTraversal && successRemoveHistory);
        }
        return false;
    }

    qsizetype Question::getSize() const
    {
        return m_classGroups.size();
    }

    QVector<QString> Question::getClassGroupNameList() const
    {
        QVector<QString> resultList;
        for (const ClassGroup& cg : m_classGroups) {
            resultList.append(cg.getName());
        }
        return resultList;
    }

    QVector<Student> Question::question(quint32 count, QuestionMode mode)
    {
        QVector<Student> studentsList;
        if (m_crtCgPtrOptional.has_value()) {
            const QMap<QuestionMode, std::function<QVector<Student>()>> modeOptionalMap{
                {QuestionMode::General, [this]() { return this->getCurrentPtr()->getIncludedStudents(); }},
                {QuestionMode::All, [this]() { return this->getCurrentPtr()->getAllStudents(); }},
                {QuestionMode::Boys, [this]() { return this->getCurrentPtr()->getBoyStudents(); }},
                {QuestionMode::Girls, [this]() { return this->getCurrentPtr()->getGirlStudents(); }},
                {QuestionMode::AllBoys, [this]() { return this->getCurrentPtr()->getBoysWithExcluded(); }},
                {QuestionMode::AllGirls, [this]() { return this->getCurrentPtr()->getGirlsWithExcluded(); }}

            };
            studentsList = modeOptionalMap.value(mode)();
        }
        return QKRandom::generateRandomsStudents(count, studentsList);
    }

    ClassGroup* Question::getCurrentPtr() const {
        if (m_crtCgPtrOptional.has_value()){
            return m_crtCgPtrOptional.value();
        }
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
        QMessageBox::critical(nullptr, "NULL pointer Error", "[ClassGroup* Question::getCurrentPtr() const]: return a nullptr");
#else
        qCritical() << "[ClassGroup* Question::getCurrentPtr() const]: return a nullptr";
#endif
        return nullptr;
    }

    QVector<Student> Question::questionWithWeight(const quint32 count, const QuestionModeWithWeight mode) const
    {
        QVector<Student> studentsList;
        if (m_crtCgPtrOptional.has_value()){
            const QMap<QuestionModeWithWeight, std::function<QVector<Student>()>> modeOptionalMap{
                {QuestionModeWithWeight::General, [this]() { return this->getCurrentPtr()->getIncludedStudents(); }},
                {QuestionModeWithWeight::Boys, [this]() { return this->getCurrentPtr()->getBoyStudents(); }},
                {QuestionModeWithWeight::Girls, [this]() { return this->getCurrentPtr()->getGirlStudents(); }},
            };
            studentsList = modeOptionalMap.value(mode)();
        }
        return QKRandom::generateRandomsStudentsWithWeight(count, studentsList);
    }

    QVector<Student> Question::questionBasedOnProperty(const quint32 count, const QString& property) const 
    {
        QVector<Student> confirms;
        if (m_crtCgPtrOptional.has_value()){
            confirms = m_crtCgPtrOptional.value()->getSpecificStudents(property) ;
        }
        QVector<Student> confirmsIncluded;
        for (const Student& stu : confirms) {
            if (stu.getType() != StudentType::Excluded) {
                confirmsIncluded.append(stu);
            }
        }
        return QKRandom::generateRandomsStudents(count, confirms);
    }

    QVector<Student> Question::questionBasedOnPropertyAll(const quint32 count, const QString& property) const 
    {
        if (m_crtCgPtrOptional.has_value()){
            return QKRandom::generateRandomsStudents(count, m_crtCgPtrOptional.value()->getSpecificStudents(property));
        }
        return {};
    }

    QVector<Student> Question::questionBasedOnPropertyWithWeight(const quint32 count, const QString& property) const 
    {
        QVector<Student> confirms;
        if (m_crtCgPtrOptional.has_value())
        {
            confirms = m_crtCgPtrOptional.value()->getSpecificStudents(property) ;
        }
        QVector<Student> confirmsIncluded;
        for (const Student& stu : confirms) {
            if (stu.getType() != StudentType::Excluded) {
                confirmsIncluded.append(stu);
            }
        }
        return QKRandom::generateRandomsStudentsWithWeight(count, confirms);
    }
#if TRAVERSAL_QUESTION_VERSION == 1
    Student Question::questionTraversal(const QString& dataPath, const QString& customID)
    {
        /*
         * 作者本想写一个支持多人同时的函数，但是会出现各种奇怪的问题，所以作者进行了修改，但是保留了大多数使用 QVector 的痕迹。
         */
        if (m_crtCgPtrOptional.has_value() && m_crtCgPtrOptional.value()->size() == 0) {
            return {"Empty List", "emptyxxx", StudentType::Excluded, StudentGender::Unknown};
        }
        quint32 count{ 1 };
        QString formatSign{};
        if (dataPath.back() != '/') {
            formatSign.append('/');
        }
        const QString filename{ dataPath + formatSign + this->getCurrentFileNameHash() + TRAVERSAL_SUFFIX };
        QFile readFile(filename);
        if (!readFile.open(QFile::ReadOnly)) {
            readFile.close();
            QFile writeFile(filename);
            if (!writeFile.open(QFile::WriteOnly)) {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
                QMessageBox::critical(nullptr, ("Error"), ("Failed to open file for writing"));
#else
                throw std::runtime_error("Failed to open file for writing");
#endif
            }
            QDataStream out(&writeFile);
            out.setVersion(QDataStream::Qt_5_10);
            Student resultStudent{ this->question(count, QuestionMode::All)[0]};
            if (!customID.isEmpty() && m_crtCgPtrOptional.has_value()) {
                for (const Student& stu : m_crtCgPtrOptional.value()->getAllStudents()) {
                    if (customID == stu.getID()) {
                        resultStudent = stu;
                    }
                }
            }
            QVector<QString> resultIDs{};
            resultIDs.append(resultStudent.getID());
            out << QUESTION_TRAVERSE_MAGIC_NUMBER;
            out << QUESTION_TRAVERSE_DATA_VERSION;
            out << resultIDs;
            writeFile.close();
            return resultStudent;
        }
        qDebug()<< "TRAVERSALDATAFILE: " << filename;
        QDataStream in(&readFile);
        in.setVersion(QDataStream::Qt_5_10);
        quint32 magicNumber{};
        quint8 version{};
        QVector<QString> haveQuestionedIDs;
        in >> magicNumber >> version >> haveQuestionedIDs;
        if (magicNumber != QUESTION_TRAVERSE_MAGIC_NUMBER || version > QUESTION_TRAVERSE_DATA_VERSION) {
            in.setStatus(QDataStream::ReadCorruptData);
            readFile.close();
            return Student{ "ERROR", "ERROR DATA", StudentType::Excluded, StudentGender::Unknown };
        }
        // qDebug() << in.status();
        qDebug() << "TRAVERSAL: " << haveQuestionedIDs;
        if (m_crtCgPtrOptional.has_value() && count > m_crtCgPtrOptional.value()->getAllStudents().size()) {
            readFile.close();
            return Student{ "ERROR", "ERROR NUMBER", StudentType::Excluded, StudentGender::Unknown };
        }
        if (m_crtCgPtrOptional.has_value() && haveQuestionedIDs.size() + count > m_crtCgPtrOptional.value()->getAllStudents().size()) {
            QFile writeFile(filename);
            if (!writeFile.open(QFile::WriteOnly)) {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
                QMessageBox::critical(nullptr, ("Error"), ("Failed to open file for writing"));
#else
                throw std::runtime_error("Failed to open file for writing");
#endif
            }
            QDataStream out(&writeFile);
            out.setVersion(QDataStream::Qt_5_10);
            Student resultStudent{ this->question(count, QuestionMode::All)[0]};
            if (m_crtCgPtrOptional.has_value() && !customID.isEmpty()) {
                for (const Student& stu : m_crtCgPtrOptional.value()->getAllStudents()) {
                    if (customID == stu.getID()) {
                        resultStudent = stu;
                    }
                }
            }

            QVector<QString> resultIDs{};
            resultIDs.append(resultStudent.getID());
            out << QUESTION_TRAVERSE_MAGIC_NUMBER;
            out << QUESTION_TRAVERSE_DATA_VERSION;
            out << resultIDs;
            writeFile.close();
            return resultStudent;
        }
        readFile.close();
        QVector<Student> haveNotQuestionedStudents{};
        if (m_crtCgPtrOptional.has_value()) {
            for (const Student& stu : m_crtCgPtrOptional.value()->getAllStudents()) {
                auto existed{ std::ranges::find(haveQuestionedIDs, stu.getID()) };
                if (existed == haveQuestionedIDs.end()) {
                    haveNotQuestionedStudents.append(stu);
                }
            }
        }
        QFile writeFileNew(filename);
        if (!writeFileNew.open(QFile::WriteOnly)) {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
            QMessageBox::critical(nullptr, ("Error"), ("Failed to open file for writing"));
#else
            qCritical() << "Failed to open file for writing";
#endif
        }
        Student result{ QKRandom::generateRandomsStudents(count, haveNotQuestionedStudents)[0]};
        if (m_crtCgPtrOptional.has_value() && !customID.isEmpty()) {
			QVector<Student> allStudents{ m_crtCgPtrOptional.value()->getAllStudents() };
            auto existed{ std::ranges::find(allStudents, Student("", customID, StudentType::Excluded, StudentGender::Unknown)) };
            if (existed != allStudents.end()) {
                result = *existed;
			}
        }
        haveQuestionedIDs.append(result.getID());
        QDataStream outNew(&writeFileNew);
        outNew.setVersion(QDataStream::Qt_5_10);
        outNew << QUESTION_TRAVERSE_MAGIC_NUMBER;
        outNew << QUESTION_TRAVERSE_DATA_VERSION;
        outNew << haveQuestionedIDs;
        writeFileNew.close();
        return result;
    }

    QVector<Student> Question::getNotQuestioned(const QString& dataPath) const
    {
        QString formatSign{};
        if (dataPath.back() != '/') {
            formatSign.append('/');
        }
        const QString filename{ dataPath + formatSign + this->getCurrentFileNameHash() + TRAVERSAL_SUFFIX };
        QFile readFile(filename);
        if (m_crtCgPtrOptional.has_value() && !readFile.open(QFile::ReadOnly)) {
            return m_crtCgPtrOptional.value()->getAllStudents();
        }
        QDataStream in(&readFile);
        in.setVersion(QDataStream::Qt_5_10);
        quint32 magicNumber{};
        quint8 version{};
        in >> magicNumber >> version;
        if (m_crtCgPtrOptional.has_value() && magicNumber != QUESTION_TRAVERSE_MAGIC_NUMBER || version > QUESTION_TRAVERSE_DATA_VERSION) {
            in.setStatus(QDataStream::ReadCorruptData);
            readFile.close();
            return m_crtCgPtrOptional.value()->getAllStudents();
        }
        QVector<QString> haveQuestionedIDs{};
        in >> haveQuestionedIDs;
        readFile.close();
        QVector<Student> haveNotQuestionedStudents{};
        if (m_crtCgPtrOptional.has_value()) {
            for (const Student& stu : m_crtCgPtrOptional.value()->getAllStudents()) {
                auto existed{ std::ranges::find(haveQuestionedIDs, stu.getID()) };
                if (existed == haveQuestionedIDs.end()) {
                    haveNotQuestionedStudents.append(stu);
                }
            }
        }
        return haveNotQuestionedStudents;
    }
    
#elif TRAVERSAL_QUESTION_VERSION == 2
    // 主函数：遍历抽取学生
    Student Question::questionTraversal(const QString& dataPath, const QString& customID) {
        // 检查当前班级是否有效
        if (!m_crtCgPtrOptional.has_value()) {
            return { "Invalid Class", "invalid_class", StudentType::Excluded, StudentGender::Unknown };
        }
        ClassGroup* cg{ m_crtCgPtrOptional.value() };
        if (cg->size() == 0) {
            return { "Empty Class", "empty_class", StudentType::Excluded, StudentGender::Unknown };
        }

        // 处理文件路径
        const QString filename{ this->getTraversalFileName(dataPath)};

        // 读取已抽取数据
        bool fileValid;
        QVector<QString> questionedIDs{ readTraversalData(filename, fileValid) };

        // 检查是否需要重置（已抽取所有学生）
        if (questionedIDs.size() == cg->size()) {
            questionedIDs.clear();
        }

        // 获取未抽取学生列表
        QVector<Student> unquestioned{ getUnquestionedStudents(cg, questionedIDs) };

        // 确定抽取的学生
        std::optional<Student> selectedStudent;
        bool found = false;

        // 处理customID
        if (!customID.isEmpty()) {
            for (const Student& stu : unquestioned) {
                if (stu.getID() == customID) {
                    selectedStudent = stu;
                    found = true;
                    break;
                }
            }
        }

        // 未找到自定义ID或无自定义ID时，随机抽取（包含excluded学生）
        if (!found) {
            QVector<Student> randomResult{ QKRandom::generateRandomsStudents(1, unquestioned) };
            if (randomResult.isEmpty()) {
                return { "抽取失败", "error", StudentType::Excluded, StudentGender::Unknown };
            }
            selectedStudent = randomResult.first();
        }

        // 更新已抽取列表
        questionedIDs.append(selectedStudent.value().getID());

        // 写入文件（无论文件是否存在，都重新创建/更新）
        writeTraversalData(filename, questionedIDs);

        return selectedStudent.value();
    }

    QString Question::getTraversalFileName(const QString& dataPath) const
    {
        return { dataPath + (dataPath.back() == '/' ? "" : "/") + getCurrentFileNameHash() + TRAVERSAL_SUFFIX };
    }

    QVector<Student> Question::getNotQuestioned(const QString& dataPath) const
    {
        // 检查当前班级是否有效
        if (!m_crtCgPtrOptional.has_value()) {
            return {};
        }
        ClassGroup* cg{ m_crtCgPtrOptional.value() };
        const QVector<Student>& allStudents = cg->getAllStudents();

        // 处理文件路径
        const QString filename{ this->getTraversalFileName(dataPath) };

        // 读取已抽取的学生ID列表
        QVector<QString> haveQuestionedIDs;
        bool fileValid{ false };

        QFile readFile(filename);
        if (readFile.open(QFile::ReadOnly)) {
            QDataStream in(&readFile);
            in.setVersion(QDataStream::Qt_5_10);

            quint32 magicNumber;
            quint8 version;
            in >> magicNumber >> version;

            // 验证文件有效性
            if (magicNumber == QUESTION_TRAVERSE_MAGIC_NUMBER && version <= QUESTION_TRAVERSE_DATA_VERSION) {
                in >> haveQuestionedIDs;
                fileValid = true;
            }
            readFile.close();
        }

        // 情况1：文件无效、未找到或已抽取列表为空 → 返回所有学生
        // 情况2：已抽取列表包含所有学生 → 返回所有学生
        if (!fileValid || haveQuestionedIDs.isEmpty() || haveQuestionedIDs.size() == allStudents.size()) {
            return allStudents;
        }

        // 优化查找效率：将ID列表转为QSet
        QSet<QString> questionedSet(haveQuestionedIDs.begin(), haveQuestionedIDs.end());

        // 筛选未被抽取的学生
        QVector<Student> notQuestioned;
        for (const Student& stu : allStudents) {
            if (!questionedSet.contains(stu.getID())) {
                notQuestioned.append(stu);
            }
        }
        return notQuestioned;
    }

    // 辅助函数：读取遍历数据文件，返回已抽取的学生ID列表
    QVector<QString> Question::readTraversalData(const QString& filename, bool& fileValid) const {
        fileValid = false;
        QFile file(filename);
        if (!file.open(QFile::ReadOnly)) {
            return {}; // 文件不存在或无法打开，返回空列表
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_10);

        quint32 magicNumber;
        quint8 version;
        QVector<QString> questionedIDs;

        in >> magicNumber >> version >> questionedIDs;

        // 验证魔法数和版本
        if (magicNumber != QUESTION_TRAVERSE_MAGIC_NUMBER || version > QUESTION_TRAVERSE_DATA_VERSION) {
            file.close();
            return {};
        }

        fileValid = true;
        file.close();
        return questionedIDs;
    }

    // 辅助函数：写入遍历数据到文件
    void Question::writeTraversalData(const QString& filename, const QVector<QString>& questionedIDs) const {
        QFile file(filename);
        if (!file.open(QFile::WriteOnly)) {
#if Q_MESSAGE_BOX_EXISTED_FOR_QUESTION
            QMessageBox::critical(nullptr, "Error", "Failed to open file for writing");
#else
            throw std::runtime_error("Failed to open file for writing");
#endif
            return;
        }

        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_10);
        out << QUESTION_TRAVERSE_MAGIC_NUMBER;
        out << QUESTION_TRAVERSE_DATA_VERSION;
        out << questionedIDs;

        file.close();
    }

    // 辅助函数：获取未被抽取的学生列表
    QVector<Student> Question::getUnquestionedStudents(const ClassGroup* cg, const QVector<QString>& questionedIDs) const {
        QVector<Student> result;
        for (const Student& stu : cg->getAllStudents()) {
            if (!questionedIDs.contains(stu.getID())) {
                result.append(stu);
            }
        }
        return result;
    }

    
#endif
}
