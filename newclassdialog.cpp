// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "newclassdialog.h"
#include "Student.h"
#include "ClassGroup.h"
#include "Question.h"
#include "sha512.h"
#include <algorithm>
#include <QMessageBox>
#include <QStandardItemModel>

NewClassDialog::NewClassDialog(QK::Question& _questionObj, QWidget *parent)
	: QDialog(parent), m_questionRef(_questionObj)
{
	ui.setupUi(this);
	this->initSet();
	this->connectSlots();
}

NewClassDialog::~NewClassDialog() = default;

void NewClassDialog::onAddStudentButtonClicked(const QString& passwdUsed, QStandardItemModel* model)
{
	const QString studentName{ ui.nameEditor->text() };
	const QString studentID{ ui.idEditor->text() };
	int typeIndex{ ui.typeComboBox->currentIndex() };
	const auto studentType{ static_cast<QK::StudentType>(typeIndex) };
	int genderIndex{ ui.genderComboBox->currentIndex() };
	const auto studentGender{ static_cast<QK::StudentGender>(genderIndex) };
	const QK::Student stu(studentName, studentID, studentType, studentGender);
	if (studentName.isEmpty()) {
		QMessageBox::critical(this, tr("Empty Name"), tr("Student Name can not be empty!"), QMessageBox::Ok );
		return;
	}
	m_cgCopy.unlock(passwdUsed);
	bool addSuccess{ m_cgCopy.addStudent(stu) };
	if (addSuccess) {
		auto itemName{ new QStandardItem(stu.getName()) };
		auto itemID{ new QStandardItem(stu.getID()) };

		static const QMap<QK::StudentType, QString> studentTypeMap{
			{QK::StudentType::Excluded, tr("Excluded")},
			{QK::StudentType::Junior, tr("Junior")},
			{QK::StudentType::Common, tr("Common")},
			{QK::StudentType::Senior, tr("Senior")}
		};
		auto itemType{ new QStandardItem(studentTypeMap.value(stu.getType())) };

		static const QMap<QK::StudentGender, QString> studentGenderMap{
			{QK::StudentGender::Unknown, tr("Unknown")},
			{QK::StudentGender::Boy, tr("Boy")},
			{QK::StudentGender::Girl, tr("Girl")}
		};
		auto itemGender{ new QStandardItem(studentGenderMap.value(stu.getGender())) };
		model->appendRow({ itemName, itemID, itemType, itemGender });
		ui.nameEditor->clear();
		ui.idEditor->clear();
	}
	else {
		QMessageBox::critical(
			this,
			tr("Cannot Add Student"),
			tr("Blank ID or the ID already exists in this class. \nIf you use the \"Random ID\" Button, please TRY PUSHING IT AGAIN."),
			QMessageBox::Ok
		);
		ui.idEditor->clear();
	}
	m_cgCopy.lock();
}

void NewClassDialog::onClassOkButtonClicked(QString& passwdUsed)
{
	const QString className{ ui.classNameEditor->text() };
	const QString passwd{ ui.passwordEditor->text() };
	const QString description{ ui.descriptionEditor->toPlainText() };
	if (className.isEmpty() || passwd.isEmpty()) {
		QMessageBox::critical(
			this,
			tr("Blank Content"),
			tr("Class Name or Password cannot be Black!"),
			QMessageBox::Ok
		);
		ui.passwordEditor->clear();
		ui.confirmPasswordEditor->clear();
	}
	else {

		if (passwd == ui.confirmPasswordEditor->text()) {
			if (passwd.length() < 8) {
				QMessageBox::warning(this, tr("Short Password"), tr("The password must be AT LEAST 8 characters long."), QMessageBox::Ok);
				ui.passwordEditor->clear();
				ui.confirmPasswordEditor->clear();
			}
			else {
				passwdUsed = passwd;
				QK::ClassGroup cg(className, description, passwd);
				QVector<QString> classNameList{ m_questionRef.getClassGroupNameList() };
				m_cgCopy = cg;
				bool success{ true };
				for (const QString& cgName : classNameList) {
					if (m_cgCopy.getName() == cgName) {
						success = false;
					}
				}

				if (!success) {
					ui.classNameEditor->clear();
					ui.passwordEditor->clear();
					ui.confirmPasswordEditor->clear();
					QMessageBox::critical(
						this,
						tr("Existed Class"),
						tr("This Class already exists. Please modify the class name"),
						QMessageBox::Ok
					);
					return;
				}
				ui.classNameShowLabel->setText(className);

				ui.classNameEditor->setEnabled(false);
				ui.passwordEditor->setEnabled(false);
				ui.confirmPasswordEditor->setEnabled(false);
				ui.descriptionEditor->setEnabled(false);
				ui.classOKButton->setEnabled(false);

				ui.nameEditor->setEnabled(true);
				ui.idEditor->setEnabled(true);
				ui.idRandomGenerateButton->setEnabled(true);
				ui.typeComboBox->setEnabled(true);
				ui.genderComboBox->setEnabled(true);
				ui.addStudentButton->setEnabled(true);
			}

		}
		else {
			ui.passwordEditor->clear();
			ui.confirmPasswordEditor->clear();
			QMessageBox::critical(
				this,
				tr("Password Error"),
				tr("The two passwords entered is inconsistent"),
				QMessageBox::Ok
			);
		}
	}
}

void NewClassDialog::onSaveButtonClicked()
{
	if (ui.classNameEditor->text().isEmpty() || ui.passwordEditor->text().isEmpty()) {
		QMessageBox::critical(
			this,
			tr("Blank Content"),
			tr("Class Name or Password cannot be Black!"),
			QMessageBox::Ok
		);
	}
	else {
		m_questionRef.append(m_cgCopy);
		if (m_questionRef.saveCurrentToFile()){
			m_questionRef.removePlaceHolder();
		}
		this->close();
	}
}

void NewClassDialog::onVisibleButtonClicked() const
{
	if (ui.passwordEditor->echoMode() == QLineEdit::Normal) {
		const QIcon invisible_icon(":/images/resources/images/invisible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(invisible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Password);
		ui.confirmPasswordEditor->setEchoMode(QLineEdit::Password);
	}
	else {
		const QIcon visible_icon(":/images/resources/images/visible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(visible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Normal);
		ui.confirmPasswordEditor->setEchoMode(QLineEdit::Normal);
	}
}

void NewClassDialog::connectSlots()
{
	static QString passwdUsed{};
	auto* model {new QStandardItemModel(0, 4, this)};
	model->setHorizontalHeaderLabels({tr("Name"), tr("ID"), tr("Type"), tr("Gender")});
	ui.showStudentsTableView->setModel(model);
	connect(ui.idRandomGenerateButton, &QPushButton::clicked, this, [=, this]()
	{
		// 点击此按钮则生成 8 位的随机数字 ID，有极小概率生成相同ID，此时学生不能正确添加
		QString resultRandomID{QK::QKRandom::generateDigitalString(8)};
		ui.idEditor->setText(resultRandomID);
	});

	connect(ui.classOKButton, &QPushButton::clicked, this, [&]()
	{
		this->onClassOkButtonClicked(passwdUsed);
	});

	connect(ui.addStudentButton, &QPushButton::clicked, this, [=, this]()
	{
		this->onAddStudentButtonClicked(passwdUsed, model);
	});

	connect(ui.saveButton, &QPushButton::clicked, this, &NewClassDialog::onSaveButtonClicked);

	connect(ui.visibleButton, &QToolButton::clicked, this, &NewClassDialog::onVisibleButtonClicked);
}

void NewClassDialog::initSet() const
{
	ui.typeComboBox->addItems({ tr("Excluded"), tr("Junior"), tr("Common"), tr("Senior") });
	ui.genderComboBox->addItems({ tr("Unknown"), tr("Boy"), tr("Girl") });
	ui.typeComboBox->setCurrentIndex(2);
	ui.genderComboBox->setCurrentIndex(0);
	ui.showStudentsTableView->setShowGrid(true);
	ui.showStudentsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.showStudentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.showStudentsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.showStudentsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.showStudentsTableView->horizontalHeader()->setStretchLastSection(true);
	ui.idEditor->setPlaceholderText(tr("ID Cannot be Modified Twice"));
	ui.nameEditor->setEnabled(false);
	ui.idEditor->setEnabled(false);
	ui.idRandomGenerateButton->setEnabled(false);
	ui.typeComboBox->setEnabled(false);
	ui.genderComboBox->setEnabled(false);
	ui.addStudentButton->setEnabled(false);
	ui.passwordEditor->setEchoMode(QLineEdit::Password);
	ui.confirmPasswordEditor->setEchoMode(QLineEdit::Password);
	ui.passwordEditor->setPlaceholderText(tr("Type your password here"));
	ui.confirmPasswordEditor->setPlaceholderText(tr("Confirm your password here"));
	ui.showStudentsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); // 关键：锁定列宽模式
	ui.showStudentsTableView->setAlternatingRowColors(true);
	ui.showStudentsTableView->setColumnWidth(0, 120);
	ui.showStudentsTableView->setColumnWidth(1, 70);
	ui.showStudentsTableView->setColumnWidth(2, 70);
	ui.showStudentsTableView->setColumnWidth(3, 60);
	const QIcon invisible_icon(":/images/resources/images/invisible.png");
	// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.visibleButton->setIcon(invisible_icon);
	ui.visibleButton->setIconSize(ui.visibleButton->size());
}
