// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"
#include "questionmainwindow.h"
#include "ui_questionmainwindow.h"
#include "newclassdialog.h"
#include "managerdialog.h"
#include "enterpassworddialog.h"
#include "xmlrw.hpp"
#include "createpassworddialog.h"
#include "progressbarwindow.h"
#include "showhistorydialog.h"
#include "Question.h"
#include "dashboardWindow.h"

#include <QIntValidator>
#include <QToolBar>
#include <QPixmap>
#include <QMessageBox>
#include <QStandardItem>
#include <QFileDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QDateTime>
#include <QStatusBar>

QuestionMainWindow::QuestionMainWindow(QK::Question& _questionObj, QK::QuestionHistory& _questionHistoryObj, QWidget* parent)
    : m_questionObjectRef(_questionObj),
    m_historyRef(_questionHistoryObj)
    , QMainWindow(parent)
    , ui(new Ui::QuestionMainWindow)
{
    ui->setupUi(this);
    m_modelMainWMain = new QStandardItemModel(0, 2, this) ;
    m_modelMainWTraversal = new QStandardItemModel(0, 2, this) ;
    m_statusLabel = new QLabel(this);
    this->statusBar()->addPermanentWidget(m_statusLabel);
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_statusLabel->setStyleSheet("QLabel { color: #000000; }");

    // 此处的某些代码顺序不可更改
    this->initToolBar();
    this->createHardware(QK::HardwareType::Arduino);
    this->connectHardwareSignals();
    this->initSliderAndSpinBox();
    this->initSet();
    this->initLogo();
    this->initWindowConfig();
    this->flushWidgetAndQuestionObj();
    QVector<QK::Student> haveNotQs{ m_questionObjectRef.getNotQuestioned(TRAVERSAL_DIRECTORY) };
    if (haveNotQs.isEmpty()) {
        haveNotQs = m_questionObjectRef.getCurrentPtr()->getAllStudents();
    }
    this->loadNotQuestionedTableView(haveNotQs, m_modelMainWTraversal);
    this->connectWindowSignals();
}

QuestionMainWindow::~QuestionMainWindow()
{
    const QString chosenClass{ ui->classComboBox->currentText() };
    const int chosenNumber{ ui->chooseCountSlider->value() };
    const int index{ ui->screenComboBox->currentIndex() };
    WindowConfig config{ 
        .m_class = chosenClass, 
        .m_number = chosenNumber, 
        .m_weightChecked = ui->weightModeCheckBox->isChecked(), 
        .m_animateChecked = ui->runAnimationButton->isChecked(), 
    };
    saveWindowConfig(WHOLE_WINDOW_CONFIG_PATH, config);
    delete ui;
}

void QuestionMainWindow::flushWidgetAndQuestionObj()
{
    /*
    * 若列表中不存在班级，则新建一个班级，该班级的定义详见宏（位于"global.h"中）：PLACEHOLDER_CLASS_DEFINE
    * 获得 ComboBox 中指定的班级名
    * 确保 Question 对象已初始化，确保初始化之后检测初始化的对象（current 指针）是否与 ComboBox 中的班级名一致。
    * （班级名在 Question 对象中具有唯一性）
    * 若以上两个条件不能同时满足，isCorrectInit 为 false
    * 若为 false，进入 if...else 语句
    * 尝试对其使用 ComboBox 的内容进行初始化，若失败则执行默认初始化。
    * 若产生任意异常，则 catch，抛出 QMessageBox::critical
    * 确保已正常初始化，获得 current 指针，显示 Total。
    */
    this->setPlaceHolderIfEmpty();

    const QString className{ ui->classComboBox->currentText() };
    const bool isCorrectInit{ m_questionObjectRef.isInitialized() &&
        className == m_questionObjectRef.getCurrentPtr()->getName()
    };
    if (!isCorrectInit) {
        m_questionObjectRef.initialize(className);
        if (!m_questionObjectRef.isInitialized()) {
            m_questionObjectRef.initialize();
        }
    }
    ui->classComboBox->setToolTip(m_questionObjectRef.getCurrentPtr()->getDescription());
    this->loadPropertiesToComboBox();
    this->initTotalCountLabel();
    m_modelMainWMain->removeRows(0, m_modelMainWMain->rowCount());
}


void QuestionMainWindow::loadPropertiesToComboBox() const
{
    ui->screenComboBox->clear();
    ui->screenComboBox->addItems({ tr("Default"), tr("Boy"), tr("Girl"), tr("With Excluded") });
    const QVector<QString> allProperties{ m_questionObjectRef.getCurrentPtr()->getAllProperties() };
    ui->screenComboBox->addItems(allProperties);
}


void QuestionMainWindow::connectWindowSignals()
{

    connect(ui->actionNew_Class, &QAction::triggered, this, &QuestionMainWindow::onActionNewClassTriggered);

    connect(ui->actionManager, &QAction::triggered, this, &QuestionMainWindow::onActionClassManagerTriggered);

    connect(ui->actionHistory, &QAction::triggered, this, &QuestionMainWindow::onActionShowHistoryTriggered);

    connect(ui->actionExport, &QAction::triggered, this, &QuestionMainWindow::onActionExportTriggered);

    connect(ui->actionImport, &QAction::triggered, this, &QuestionMainWindow::onActionImportTriggered);

    connect(ui->actionHelp, &QAction::triggered, this, &QuestionMainWindow::onActionHelpTriggered);

    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);

    connect(ui->actionAbout_Qt6, &QAction::triggered, this, [=, this]() {
        QMessageBox::aboutQt(this, tr("About Qt6"));
        });

    connect(ui->actionAbout_Question, &QAction::triggered, this, &QuestionMainWindow::onActionAboutQuestionTriggered);

    connect(ui->actionAbout_Us, &QAction::triggered, this, &QuestionMainWindow::onActionAboutUsTriggered);

    connect(ui->startButton, &QPushButton::clicked, this, &QuestionMainWindow::onButtonStartQuestionClicked);

    connect(ui->actionOpen_DashBoard, &QAction::triggered, this, &QuestionMainWindow::onActionOpenDashboardTriggered);

    connect(ui->classComboBox, &QComboBox::currentIndexChanged, this, [=, this]() {
        this->flushWidgetAndQuestionObj();
        QVector<QK::Student> haveNotQs{ m_questionObjectRef.getNotQuestioned(TRAVERSAL_DIRECTORY) };
        if (haveNotQs.isEmpty()) {
            haveNotQs = m_questionObjectRef.getCurrentPtr()->getAllStudents();
        }
        this->loadNotQuestionedTableView(haveNotQs, m_modelMainWTraversal);
        });

    connect(ui->screenComboBox, &QComboBox::currentIndexChanged, this, [=, this]() {
        this->initTotalCountLabel();
        });

    connect(ui->chooseCountSpinBox, &QSpinBox::valueChanged, this, [=, this]() {
        ui->countEditor->clear();
        });

    connect(ui->chooseCountSlider, &QSlider::valueChanged, this, [=, this]() {
        ui->countEditor->clear();
        });

    connect(ui->startTraversalButton, &QPushButton::clicked, this, &QuestionMainWindow::onButtonStartTraversalClicked);

    connect(ui->resetButton, &QPushButton::clicked, this, &QuestionMainWindow::onButtonResetClicked);

    connect(ui->exportResultButton, &QPushButton::clicked, this, &QuestionMainWindow::onButtonExportResultClicked);

    connect(ui->noQuestionedStudentsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [&](const QItemSelection& selected, const QItemSelection& deselected)
        {
            Q_UNUSED(deselected);
            QModelIndexList selectedIndexes = selected.indexes();
            if (selectedIndexes.isEmpty()) return;
            const int targetRow {selectedIndexes.first().row()};
            const QModelIndex col2Index {m_modelMainWTraversal->index(targetRow, 1)}; // 第二列索引为1[2,3](@ref)
            const QString col2Data {m_modelMainWTraversal->data(col2Index, Qt::DisplayRole).toString()};
            ui->customIdShow->setText(col2Data);
        });
}

void QuestionMainWindow::setPlaceHolderIfEmpty() const
{
    if (m_questionObjectRef.getSize() == 0) {
        m_questionObjectRef.append(PLACEHOLDER_CLASS_DEFILE);
        m_questionObjectRef.initialize(0);
        ui->classComboBox->addItems(m_questionObjectRef.getClassGroupNameList());
        ui->classComboBox->setCurrentIndex(0);
    }
}

void QuestionMainWindow::loadNotQuestionedTableView(const QVector<QK::Student>& students, QStandardItemModel* model)
{
    // model->clear();
    const int rows {model->rowCount()};
    if (rows > 0) {
        model->removeRows(0, rows); // 从第0行开始删除所有行
    }
    for (const QK::Student& stu : students) {
        auto itemName{ new QStandardItem(stu.getName()) };
        auto itemID{ new QStandardItem(stu.getID()) };
        model->appendRow({ itemName, itemID });
    }
}

void QuestionMainWindow::onActionNewClassTriggered()
{
    const QVector<QString> classListOld{ m_questionObjectRef.getClassGroupNameList() };
    auto newClassDialog{ new NewClassDialog(m_questionObjectRef, this) };
    // newClassDialog->setAttribute(Qt::WA_DeleteOnClose);
    newClassDialog->exec();
    const QVector<QString> classListNew{ m_questionObjectRef.getClassGroupNameList() };
    if (classListOld != classListNew) {
        ui->classComboBox->clear();
        ui->classComboBox->addItems(classListNew);
        ui->classComboBox->setCurrentIndex(classListNew.size() - 1);
        this->flushWidgetAndQuestionObj();
    }
    delete newClassDialog;
}

void QuestionMainWindow::onActionClassManagerTriggered()
{
    if (m_questionObjectRef.isInitialized()) {
        const QK::ClassGroup* cgPointer{ m_questionObjectRef.getCurrentPtr() };
        QString classNameOld{ cgPointer->getName() };
        if (cgPointer->checkUnlock()) {
            // 若班级已解锁则直接打开管理窗口
            // 实际这段代码可能难以触发
            auto managerDialog{ new ManagerDialog(m_historyRef, m_questionObjectRef, this) };
            managerDialog->setAttribute(Qt::WA_DeleteOnClose);
            managerDialog->exec();
            {
                ui->classComboBox->clear();
                QVector<QString> clsNameList{ m_questionObjectRef.getClassGroupNameList() };
                ui->classComboBox->addItems(clsNameList);
                bool existed{ false };
                for (qsizetype i = 0; i < m_questionObjectRef.getSize(); ++i) {
                    if (classNameOld == clsNameList[i]) {
                        ui->classComboBox->setCurrentIndex(i);
                        existed = true;
                        break;
                    }
                }
                if (!existed) {
                    this->flushWidgetAndQuestionObj();
                    // m_questionObjectRef.initialize(0);
                    ui->classComboBox->setCurrentIndex(0);
                }
            }
        }
        else {
            // 如果班级未解锁，则首先打开密码输入窗口，若密码正确，则打开管理窗口
            // 管理窗口结束后，则刷新 ComboBox，并将该班级重新锁定。
            EnterPasswordDialog* enterPwdDialog{ new EnterPasswordDialog(m_questionObjectRef, this) };
            enterPwdDialog->setAttribute(Qt::WA_DeleteOnClose);

            connect(enterPwdDialog, &QDialog::finished, this, [this, &classNameOld](const int result) {
                if (result == QDialog::Accepted) {
                    auto managerDialog{ new ManagerDialog(m_historyRef, m_questionObjectRef, this) };
                    managerDialog->setAttribute(Qt::WA_DeleteOnClose);
                    managerDialog->exec();
                    {
                        ui->classComboBox->clear();
                        QVector<QString> clsNameList{ m_questionObjectRef.getClassGroupNameList() };
                        ui->classComboBox->addItems(clsNameList);
                        bool existed{ false };
                        for (qsizetype i = 0; i < m_questionObjectRef.getSize(); ++i) {
                            if (classNameOld == clsNameList[i]) {
                                ui->classComboBox->setCurrentIndex(i);
                                m_questionObjectRef.initialize(i);
                                existed = true;
                                break;
                            }
                        }
                        if (!existed) {
                            this->setPlaceHolderIfEmpty();
                            m_questionObjectRef.initialize(0);
                            ui->classComboBox->setCurrentIndex(0);
                        }
                    }
                }
                });
            enterPwdDialog->exec();
        }
        this->flushWidgetAndQuestionObj();
        m_questionObjectRef.getCurrentPtr()->lock();
    }
    else {
        QMessageBox::critical(
            this,
            tr("Initialization Exception"),
            tr("The Program is not initialized correctly!"),
            QMessageBox::Ok);
    }
}

void QuestionMainWindow::onActionShowHistoryTriggered()
{
    auto showHistoryDialog{ new
        ShowHistoryDialog(m_questionObjectRef, m_historyRef, this) };
    showHistoryDialog->setAttribute(Qt::WA_DeleteOnClose);
    showHistoryDialog->exec();
}

void QuestionMainWindow::onActionExportTriggered()
{
    const QK::ClassGroup* cgPtr{ m_questionObjectRef.getCurrentPtr() };
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Excel Sheet"),
        QDir::homePath() + "/" + cgPtr->getName() + ".xlsx", // 默认文件名
        tr("Excel Sheet(*.xlsx)")
    );
    if (fileName.isEmpty()) {
        return;
    }
    const bool exportSuccess{ cgPtr->exportToXlsx(fileName) == QK::XlsxIEState::ExportSuccess ? true : false };
    if (exportSuccess) {
        QMessageBox::information(
            this,
            tr("Export Success"),
            tr("Export Successfully!"),
            QMessageBox::Ok
        );
    }
    else {
        QMessageBox::critical(
            this,
            tr("Export Error"),
            tr("Cannot export this class!"),
            QMessageBox::Ok
        );
    }
}

void QuestionMainWindow::onActionImportTriggered()
{
    const QString fileName {QFileDialog::getOpenFileName(
        this,
        tr("Import Data"),
        QDir::homePath(),
        tr("Excel Sheet(*.xlsx)")
    )};
    if (fileName.isEmpty()) {
        return;
    }
    auto pwdDialog{ new CreatePasswordDialog(this) };
    // pwdDialog->setAttribute(Qt::WA_DeleteOnClose);
    const int state{ pwdDialog->exec() };
    if (state == QDialog::Accepted) {
        const QString passwd{ pwdDialog->getPassword() };
        QK::ClassGroup cg;
        const QK::XlsxIEState importState{ cg.importFromXlsx(fileName, passwd) };
        QVector<QString> clsNameList{};
        bool saveCG;
        bool saveSuccess{false};
        switch (importState) {
        case (QK::XlsxIEState::ImportReachMax):
            QMessageBox::warning(this,
                tr("Reach MAX"),
                QString(tr("[[ClassGroup::importFromXlsx()]]: Student CANNOT be added to ClassGroup continuously, the MAX size is: ")) + QString::number(MAX_CLASS_GROUP_SIZE)
            );
            [[fallthrough]];
        case (QK::XlsxIEState::ImportSuccess):
            saveCG = m_questionObjectRef.append(cg);
            if (!saveCG) {
                QMessageBox::critical(
                    this,
                    tr("Existed Class"),
                    tr("This Class already exists. Please modify the class name"),
                    QMessageBox::Ok
                );
                delete pwdDialog;
                return;
            }
            saveSuccess = m_questionObjectRef.saveCurrentToFile() ;
            if (saveSuccess) {
                ui->classComboBox->clear();
                clsNameList = m_questionObjectRef.getClassGroupNameList();
                ui->classComboBox->addItems(clsNameList);
                ui->classComboBox->setCurrentIndex(m_questionObjectRef.getSize() - 1);
                this->flushWidgetAndQuestionObj();
                QMessageBox::information(this, tr("Import Success"), tr("Import Class Sucessfully!"));
            }
            break;
        default:
            QMessageBox::critical(this, tr("Unable to recognize"), tr("Unsupported Excel Document Format!"));
            break;
        }
    }
    delete pwdDialog;
}

void QuestionMainWindow::onActionHelpTriggered()
{
    const QUrl url{ QUrl::fromLocalFile("./Instruction.pdf") };
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::critical(
            this,
            tr("Error"),
            tr("We can not find the Instruction PDF Document.")
        );
    }
}

void QuestionMainWindow::onActionAboutQuestionTriggered()
{
    QString content;
    QFile file(":/text/resources/about_question.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        content = QString();
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8); // 设置UTF-8编码
    content = (in.readAll()).arg(QUESTION_VERSION_STR, KERNEL_VERSION_STR, QT_VERSION_STR);
    file.close();
    QMessageBox::about(this, tr("About Question"), content);
}

void QuestionMainWindow::onActionAboutUsTriggered()
{
    QString content;
    QFile file(":/text/resources/about_us.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        content = QString();
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8); // 设置UTF-8编码
    content = in.readAll();
    file.close();
    QMessageBox::about(this, tr("About Us"), content);
}

void QuestionMainWindow::onButtonStartQuestionClicked()
{
    const int countShow{ ui->countShowLabel->text().toInt() };
    const QString countEditorValue{ui->countEditor->text()};
    const bool validCount_0{countShow > ui->chooseCountSlider->value() && countEditorValue.isEmpty()};
    const bool validCount_1{!countEditorValue.isEmpty() && countShow > countEditorValue.toInt()};
    if (ui->runAnimationButton->isChecked() && (validCount_0 || validCount_1))
    {
        auto animationWindow{ new ProgressBarWindow(this) };
        animationWindow->setAttribute(Qt::WA_DeleteOnClose);
        animationWindow->startProgress();
        animationWindow->exec();
    }
    m_modelMainWMain->removeRows(0, m_modelMainWMain->rowCount());
    if (!m_questionObjectRef.isInitialized()) return;
    m_questionedStudentsList.clear();
    quint32 questionCount{};
    const QString inputNumberString{ ui->countEditor->text() };
    if (inputNumberString.isEmpty()) {
        questionCount = ui->chooseCountSpinBox->value();
    }
    else {
        questionCount = inputNumberString.toInt();
    }
    switch (ui->screenComboBox->currentIndex()) {
    case 0:
        if (ui->weightModeCheckBox->isChecked()) {
            m_questionedStudentsList.append(m_questionObjectRef.questionWithWeight(questionCount, QK::QuestionModeWithWeight::General));
        }
        else {
            m_questionedStudentsList.append(m_questionObjectRef.question(questionCount, QK::QuestionMode::General));
        }
        break;
    case 1:
        if (ui->weightModeCheckBox->isChecked()) {
            m_questionedStudentsList.append(m_questionObjectRef.questionWithWeight(questionCount, QK::QuestionModeWithWeight::Boys));
        }
        else {
            m_questionedStudentsList.append(m_questionObjectRef.question(questionCount, QK::QuestionMode::Boys));
        }
        break;
    case 2:
        if (ui->weightModeCheckBox->isChecked()) {
            m_questionedStudentsList.append(m_questionObjectRef.questionWithWeight(questionCount, QK::QuestionModeWithWeight::Girls));
        }
        else {
            m_questionedStudentsList.append(m_questionObjectRef.question(questionCount, QK::QuestionMode::Girls));
        }
        break;
    case 3:
        m_questionedStudentsList.append(m_questionObjectRef.question(questionCount, QK::QuestionMode::All));
        break;
    default:
        const QString property{ ui->screenComboBox->currentText() };
        m_questionedStudentsList.append(m_questionObjectRef.questionBasedOnProperty(questionCount, property));
        break;
    }
    for (const QK::Student& stu : m_questionedStudentsList) {
        QVector<QString> serialization{ stu.serialize() };
        auto itemName{ new QStandardItem(serialization[QK::STUDENT_NAME_INDEX]) };
        auto itemID{ new QStandardItem(serialization[QK::STUDENT_ID_INDEX]) };
        m_modelMainWMain->appendRow({ itemName, itemID });
    }
    const qsizetype count{ m_questionedStudentsList.size() };
    if (count == 0) {
        ui->countEditor->clear();
        this->statusBar()->setStyleSheet("QStatusBar { color: #FF0000; }");
        this->statusBar()->showMessage(tr("Exceeds the maximum value!"), 3000);
        MAINWINDOW_CALL_WARNING_SOUND
    }
    QString isWeightMode;
    if (ui->weightModeCheckBox->isChecked()) {
        isWeightMode = tr("(Weighted)");
    }
    QK::QuestionRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.className = m_questionObjectRef.getCurrentPtr()->getName();
    record.count = m_questionedStudentsList.size();
    record.mode = ui->screenComboBox->currentText() + isWeightMode;
    const int questionedSize{static_cast<int>(m_questionedStudentsList.size())};
    if (questionedSize == 0) {
        return;
    }
    record.results = m_questionedStudentsList;
    m_historyRef.addRecord(record);
}

void QuestionMainWindow::onButtonStartTraversalClicked() const
{
    const QString lastName{ ui->nameShower->text() };
    const QString lastID{ ui->idShower->text() };
    ui->lastStudentLabel->setText(lastName);
    ui->lastStudentIDLabel->setText(lastID);
    const QString customID{ ui->customIdShow->text() };
    const QK::Student stuResult{ m_questionObjectRef.questionTraversal(TRAVERSAL_DIRECTORY, customID) };
    ui->nameShower->setText(stuResult.getName());
    ui->idShower->setText(stuResult.getID());
    const QVector<QK::Student> haveNotQs{ m_questionObjectRef.getNotQuestioned(TRAVERSAL_DIRECTORY) };
    if (haveNotQs.isEmpty()) {
        this->loadNotQuestionedTableView(m_questionObjectRef.getCurrentPtr()->getAllStudents(), m_modelMainWTraversal);
    }
    else {
        this->loadNotQuestionedTableView(haveNotQs, m_modelMainWTraversal);
    }

    QK::QuestionRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.className = m_questionObjectRef.getCurrentPtr()->getName();
    record.count = 1;
    record.mode = tr("Traversal");
    record.results = { stuResult };
    m_historyRef.addRecord(record);
    ui->customIdShow->clear();
}

void QuestionMainWindow::onButtonResetClicked()
{
    auto enterPwdDialog{ new EnterPasswordDialog(m_questionObjectRef, this) };
    enterPwdDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(enterPwdDialog, &QDialog::finished, this, [this](int result) {
        const bool isCorrectPwd{ result == QDialog::Accepted };
        QString formatSign{};
        if (isCorrectPwd) {
            const bool successReset{ QFile::remove(TRAVERSAL_DIRECTORY + m_questionObjectRef.getCurrentFileNameHash() + TRAVERSAL_SUFFIX) };
            if (successReset) {
                this->loadNotQuestionedTableView(m_questionObjectRef.getCurrentPtr()->getAllStudents(), m_modelMainWTraversal);
                QMessageBox::information(
                    this,
                    tr("Reset Succeed"),
                    tr("Reset Traversal File Successfully!")
                );
                ui->lastStudentLabel->clear();
                ui->lastStudentIDLabel->clear();
                ui->customIdShow->clear();
                ui->nameShower->clear();
                ui->idShower->clear();
                m_questionObjectRef.lockCurrent();
            }
            else {
                QMessageBox::warning(
                    this,
                    tr("Reset Failed"),
                    tr("Reset Failed: The Traversal Configuration File does not exist or was not Deleted Correctly."));
            }
        }
        });
    enterPwdDialog->exec();
}

void QuestionMainWindow::onButtonExportResultClicked()
{
    if (m_questionedStudentsList.isEmpty()) {
        QMessageBox::warning(this, tr("Empty Result"), tr("No result to export!"));
        return;
    }
    const QDateTime now{ QDateTime::currentDateTime() };
    const QString timeString{ now.toString("yyyy-MM-dd_hh-mm-ss") };
    const QString fileName{ QFileDialog::getSaveFileName(
        this,
        tr("Export Excel Sheet"),
        QDir::homePath() + "/" + tr("result-") + timeString + ".xlsx", // 默认文件名
        tr("Excel Sheet(*.xlsx)")
    ) };
    if (fileName.isEmpty()) {
        return;
    }
    const bool exportSuccess{
        QK::ClassGroup::exportToXlsx(tr("Question Result"), m_questionedStudentsList, fileName, false) 
        == QK::XlsxIEState::ExportSuccess 
    };
    if (exportSuccess) {
        QMessageBox::information(
            this,
            tr("Export Success"),
            tr("Export Successfully!"),
            QMessageBox::Ok
        );
    }
    else {
        QMessageBox::critical(
            this,
            tr("Export Error"),
            tr("Cannot export this class!"),
            QMessageBox::Ok
        );
    }
}

void QuestionMainWindow::initToolBar() {
    auto toolbar {new QToolBar(this)};
    addToolBar(toolbar);
    toolbar->addActions({ ui->actionNew_Class, ui->actionManager, ui->actionHistory, ui->actionOpen_DashBoard });
    toolbar->addSeparator();
    toolbar->addActions({ ui->actionImport, ui->actionExport });
    toolbar->addSeparator();
    toolbar->addActions({ ui->actionHelp, ui->actionExit });
}

void QuestionMainWindow::initLogo() const
{
    QPixmap pixmap(":/icons/resources/QuestionIcon11.png");
    QPixmap pixmap_2(":/icons/resources/QuestionIcon11.png");
    pixmap = pixmap.scaled(ui->logoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmap_2 = pixmap_2.scaled(ui->logoLabel_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->logoLabel->setPixmap(pixmap);
    ui->logoLabel->setAlignment(Qt::AlignCenter);
    ui->logoLabel_2->setPixmap(pixmap_2);
    ui->logoLabel_2->setAlignment(Qt::AlignCenter);

}

void QuestionMainWindow::initWindowConfig() const
{
    WindowConfig config{
        .m_number = 1, 
        .m_weightChecked = false, 
        .m_animateChecked = false, 
    };
    loadWindowConfig(WHOLE_WINDOW_CONFIG_PATH, config );
    ui->classComboBox->setCurrentText(config.m_class);
    this->setPlaceHolderIfEmpty();
    m_questionObjectRef.initialize(config.m_class);
    ui->chooseCountSlider->setValue(config.m_number);
    ui->weightModeCheckBox->setChecked(config.m_weightChecked);
    ui->runAnimationButton->setChecked(config.m_animateChecked);
}

void QuestionMainWindow::initSliderAndSpinBox()
{
    connect(ui->chooseCountSlider, &QSlider::valueChanged, ui->chooseCountSpinBox, &QSpinBox::setValue);
    connect(ui->chooseCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->chooseCountSlider, &QSlider::setValue);

    // 初始化值（避免信号循环）
    ui->chooseCountSpinBox->setValue(1); // 初始值设为1
    // classComboBox 初始设置
    ui->classComboBox->addItems(m_questionObjectRef.getClassGroupNameList());
}

void QuestionMainWindow::initTotalCountLabel() const
{
    QString countShowLabelText;
    switch (ui->screenComboBox->currentIndex()) {
    case 0:
        countShowLabelText = QString::number(m_questionObjectRef.getCurrentPtr()->getIncludedCount());
        break;
    case 1:
        countShowLabelText = QString::number(m_questionObjectRef.getCurrentPtr()->getBoyCount());
        break;
    case 2:
        countShowLabelText = QString::number(m_questionObjectRef.getCurrentPtr()->getGirlCount());
        break;
    case 3:
        countShowLabelText = QString::number(m_questionObjectRef.getCurrentPtr()->size());
        break;
    default:
        countShowLabelText = QString::number(m_questionObjectRef.getCurrentPtr()->getSpecificCount(ui->screenComboBox->currentText()));
        break;
    }
    ui->countShowLabel->setText(countShowLabelText);
}

void QuestionMainWindow::initSet()
{
    m_modelMainWMain->setHorizontalHeaderLabels({ tr("Name"), tr("ID") });
    ui->tableView_SelectedStudents->setModel(m_modelMainWMain);
    m_modelMainWTraversal->setHorizontalHeaderLabels({ tr("Name"), tr("ID") });
    ui->noQuestionedStudentsTableView->setModel(m_modelMainWTraversal);
    ui->tableView_SelectedStudents->setShowGrid(true);
    ui->tableView_SelectedStudents->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_SelectedStudents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_SelectedStudents->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_SelectedStudents->setAlternatingRowColors(true);
    ui->tableView_SelectedStudents->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_SelectedStudents->horizontalHeader()->setStretchLastSection(true);

    ui->screenComboBox->setCurrentIndex(0);
    ui->noQuestionedStudentsTableView->setShowGrid(true);
    ui->noQuestionedStudentsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->noQuestionedStudentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->noQuestionedStudentsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->noQuestionedStudentsTableView->setAlternatingRowColors(true);
    ui->noQuestionedStudentsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->noQuestionedStudentsTableView->horizontalHeader()->setStretchLastSection(true);
    ui->nameShower->setReadOnly(true);
    ui->idShower->setReadOnly(true);
    ui->customIdShow->setReadOnly(true);
    // traversal 窗口其他设置
    auto* validator {new QIntValidator(this)};
    ui->countEditor->setValidator(validator);
}

void QuestionMainWindow::onActionOpenDashboardTriggered()
{
    if (!m_questionObjectRef.getCurrentPtr()) {
        QMessageBox::warning(this, "提示", "请先选择班级");
        return;
    }
    // 打开看板窗口
    auto dashboard {new DashboardWindow(
        m_questionObjectRef.getCurrentPtr(),
        &m_historyRef, 
        this
    )};
    dashboard->setAttribute(Qt::WA_DeleteOnClose);
    dashboard->exec();


}
