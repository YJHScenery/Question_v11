// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "managerdialog.h"

#include <QFile>

#include "enterpassworddialog.h"
#include "createpassworddialog.h"
#include "propertieseditor.h"

#include <QMessageBox>
#include <QStandardItem>

ManagerDialog::ManagerDialog(QK::QuestionHistory& historyRef, QK::Question& _questionObj, QWidget *parent)
	: QDialog(parent), m_cgCopy(*(_questionObj.getCurrentPtr())), m_historyRef(historyRef), m_questionObj(_questionObj)
{
	ui.setupUi(this);
	m_selectModel = new QStandardItemModel(0, 4, this);
	ui.typeComboBox->addItems({tr("Excluded"), tr("Junior"), tr("Common"), tr("Senior")});
	ui.genderComboBox->addItems({tr("Unknown"), tr("Boy"), tr("Girl")});
	// static QK::ClassGroup cgCopy{*_questionObj.getCurrentPtr()};

	ui.descriptionEditor->setText(m_cgCopy.getDescription());
	ui.idEditor->setPlaceholderText(tr("ID Cannot be Modified Twice"));
	ui.searchEdit->setPlaceholderText(tr("Search by name or ID..."));
	m_selectModel->setHorizontalHeaderLabels({ tr("Name"), tr("ID"), tr("Type"), tr("Gender") });
	this->tableViewSet(ui.studentsTableView, m_selectModel);
	QVector<QK::Student> studentsVector{ m_cgCopy.getAllStudents() };
	this->loadStudentShowTableView(studentsVector, m_selectModel);
	this->connectSlots();
}

void ManagerDialog::connectSlots()
{
	connect(ui.studentsTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
	        [&](const QItemSelection& selected, const QItemSelection& deselected)
	        {
		        // ui.reviseModeRadioButton->setChecked(true);
		        this->onSelectChanged(selected, deselected);
		        ui.editPropertiesButton->setEnabled(true);
	        }
	);

	connect(ui.addStudentButton, &QPushButton::clicked, this, [=, this]() mutable
	{
		onAddOrReviseButtonClicked(m_selectModel);
	});

	connect(ui.searchEdit, &QLineEdit::textChanged, this, [=, this](const QString& text)
	{
		QVector<QK::Student> filtered;
		for (const auto& stu : m_cgCopy.getAllStudents())
		{
			if (stu.getName().contains(text, Qt::CaseInsensitive) ||
				stu.getID().contains(text, Qt::CaseInsensitive))
			{
				filtered.append(stu);
			}
		}
		loadStudentShowTableView(filtered, qobject_cast<QStandardItemModel*>(ui.studentsTableView->model()));
	});

	connect(ui.editPropertiesButton, &QPushButton::clicked, this, [=, this]()
	{
		const QString selectedID{ui.idEditor->text()};
		if (!selectedID.isEmpty())
		{
			PropertiesEditor* editor{new PropertiesEditor(m_cgCopy.getStudentRef(selectedID), this)};
			editor->exec();
		}
	});

	connect(ui.additionModeRadioButton, &QRadioButton::toggled, this, [=, this](bool checked)
	{
		this->onAdditionModeRadioButtonToggled(checked);
	});

	connect(ui.reviseModeRadioButton, &QRadioButton::toggled, this, [=, this](bool checked)
	{
		this->onReviseModeRadioButtonToggled(checked);
	});

	connect(ui.idRandomGenerateButton, &QPushButton::clicked, this, [=, this]()
	{
		// 点击此按钮则生成 8 位的随机数字 ID，有极小概率生成相同ID，此时学生不能正确添加
		QString resultRandomID{QK::QKRandom::generateDigitalString(8)};
		ui.idEditor->setText(resultRandomID);
	});

	ui.reviseModeRadioButton->setChecked(true);

	connect(ui.reviseButton, &QPushButton::clicked, this, [=, this]() mutable
	{
		if (ui.nameEditor->text().isEmpty())
		{
			return;
		}
		m_cgCopy.removeStudent(ui.idEditor->text());
		onAddOrReviseButtonClicked(m_selectModel);
		ui.searchEdit->clear();
	});

	connect(ui.removeButton, &QPushButton::clicked, this, [=, this]()
	{
		m_cgCopy.removeStudent(ui.idEditor->text());
		this->loadStudentShowTableView(m_cgCopy.getAllStudents(), m_selectModel);
	});

	connect(ui.clearButton, &QPushButton::clicked, this, [=, this]()
	{
		m_cgCopy.removeAllStudents();
		this->loadStudentShowTableView(m_cgCopy.getAllStudents(), m_selectModel);
	});

	connect(ui.deleteButton, &QPushButton::clicked, this, [=, this]()
	{
		this->onDeleteButtonClicked();
	});

	connect(ui.saveButton, &QPushButton::clicked, this, [=, this]()
	{
		this->onSaveButtonClicked();
	});

	connect(ui.cancelButton, &QPushButton::clicked, this, [=, this]()
	{
		this->close();
	});

	connect(ui.changePwdButton, &QPushButton::clicked, this, [=, this]()
	{
		this->onChangePwdButtonClicked();
	});

	connect(ui.clearHistoryButton, &QPushButton::clicked, this, [this]()
	{
		const QMessageBox::StandardButton reply{
			QMessageBox::question(
				this,
				tr("Verity Operation"),
				tr("Do you want to clear question history? "),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No
			)
		};
		if (reply == QMessageBox::Yes){
			bool removeState {m_historyRef.clearAllRecordsByClass(m_questionObj.getCurrentPtr()->getName())};
			if (removeState)
			{
				QMessageBox::information(this, tr("Success"),tr("Clear History Successfully!"));
			}
		}
	});
}

ManagerDialog::~ManagerDialog() = default;

void ManagerDialog::onAddOrReviseButtonClicked(QStandardItemModel *model) {
	QString studentName{ ui.nameEditor->text() };
	QString studentID{ ui.idEditor->text() };

	int typeIndex{ ui.typeComboBox->currentIndex() };
	QK::StudentType studentType{ static_cast<QK::StudentType>(typeIndex) };

	int genderIndex{ ui.genderComboBox->currentIndex() };
	QK::StudentGender studentGender{ static_cast<QK::StudentGender>(genderIndex) };

	QK::Student stu(studentName, studentID, studentType, studentGender);
	if (studentName.isEmpty()) {
		QMessageBox::critical(
			this,
			tr("Empty Name"),
			tr("Student Name can not be empty!"),
			QMessageBox::Ok
		);
		return;
	}

	bool addSuccess{ m_cgCopy.addStudent(stu) };

	if (addSuccess) {
		this->loadStudentShowTableView(m_cgCopy.getAllStudents(), model);
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
}

void ManagerDialog::loadStudentShowTableView(const QVector<QK::Student>& students, QStandardItemModel* model)
{
	// m_selectModel->clear();
	int rows = model->rowCount();
	if (rows > 0) {
		model->removeRows(0, rows); // 从第0行开始删除所有行
	}
	for (const QK::Student& stu : students) {
		QStandardItem* itemName{ new QStandardItem(stu.getName()) };
		QStandardItem* itemID{ new QStandardItem(stu.getID()) };
		
		static const QMap<QK::StudentType, QString> studentTypeMap{
			{QK::StudentType::Excluded, tr("Excluded")},
			{QK::StudentType::Junior, tr("Junior")},
			{QK::StudentType::Common, tr("Common")},
			{QK::StudentType::Senior, tr("Senior")}
		};
		QStandardItem* itemType{ new QStandardItem(studentTypeMap.value(stu.getType())) };

		static const QMap<QK::StudentGender, QString> studentGenderMap{
			{QK::StudentGender::Unknown, tr("Unknown")},
			{QK::StudentGender::Boy, tr("Boy")},
			{QK::StudentGender::Girl, tr("Girl")}
		};
		QStandardItem* itemGender{new QStandardItem(studentGenderMap.value(stu.getGender()))};
		// QStandardItem* itemType{ new QStandardItem(stu.getTypeString()) };
		// QStandardItem* itemGender{ new QStandardItem(stu.getGenderString()) };
		// size_t index{ cgPointer->size() };
		model->appendRow({ itemName, itemID, itemType, itemGender });
	}
}

void ManagerDialog::onAdditionModeRadioButtonToggled(bool checked)
{
	if (checked) {
		ui.addStudentButton->setEnabled(true);
		ui.idEditor->setEnabled(true);
		ui.reviseButton->setEnabled(false);
		ui.deleteButton->setEnabled(false);
		ui.clearButton->setEnabled(false);
		ui.removeButton->setEnabled(false);
		ui.idRandomGenerateButton->setEnabled(true);
		ui.descriptionEditor->setEnabled(false);
		ui.studentsTableView->setEnabled(false);
		ui.searchEdit->clear();
		ui.searchEdit->setEnabled(false);
		ui.nameEditor->clear();
		ui.idEditor->clear();
		ui.typeComboBox->setCurrentIndex(static_cast<int>(QK::StudentType::Common));
		ui.genderComboBox->setCurrentIndex(static_cast<int>(QK::StudentGender::Unknown));
		ui.editPropertiesButton->setEnabled(false);
	}
}

void ManagerDialog::onReviseModeRadioButtonToggled(bool checked)
{
	if (checked) {
		ui.addStudentButton->setEnabled(false);
		ui.idEditor->setEnabled(false);
		ui.reviseButton->setEnabled(true);
		ui.deleteButton->setEnabled(true);
		ui.clearButton->setEnabled(true);
		ui.removeButton->setEnabled(true);
		ui.idRandomGenerateButton->setEnabled(false);
		ui.descriptionEditor->setEnabled(true);
		ui.searchEdit->clear();
		ui.searchEdit->setEnabled(true);
		ui.studentsTableView->setEnabled(true);
		ui.editPropertiesButton->setEnabled(false);
	}
}

void ManagerDialog::onDeleteButtonClicked()
{
	QMessageBox::StandardButton ret{ QMessageBox::question(
			this,
			tr("Delete Class-Irreversible operation"),
			tr("Do you want to delete this class? This operation is irreversible"),
			QMessageBox::Ok | QMessageBox::Cancel
			)
	};
	bool removeSuccess{ false };
	switch (ret) {
	case QMessageBox::Ok:
		removeSuccess = m_questionObj.removeCurrent();
		break;
	case QMessageBox::Cancel:
		break;
	default:
		break;
	}
	if (removeSuccess) {
		QMessageBox::information(
			this,
			tr("Delete Success"),
			tr("Delete Class Successfully"));
		this->close();
	}
	if (m_questionObj.getSize() == 0) {
		m_questionObj.append(QK::ClassGroup("PLACEHOLDER CLASS", "", ""));
	}
}

void ManagerDialog::onSaveButtonClicked()
{
	QMessageBox::StandardButton ret{ QMessageBox::question(
			this,
			tr("Save All Operations"),
			tr("Do you keep all operations?"),
			QMessageBox::Save | QMessageBox::Cancel,
			QMessageBox::Save
		) };
	bool saveSuccess{ false };
	switch (ret) {
	case QMessageBox::Save:
		m_cgCopy.reviseDescription(ui.descriptionEditor->toPlainText());
		*(m_questionObj.getCurrentPtr()) = m_cgCopy;
		saveSuccess = m_questionObj.saveCurrentToFile();
		if (saveSuccess) {
			QMessageBox::information(
				this,
				tr("Save Success"),
				tr("All operations have been completed!")
			);
		}
		break;
	case QMessageBox::Cancel:
		break;
	default:
		break;
	}
	this->close();
}

void ManagerDialog::onChangePwdButtonClicked()
{
	EnterPasswordDialog *enterPwdDialog{new EnterPasswordDialog(m_questionObj, this) };
	if (enterPwdDialog->exec() == QDialog::Accepted) {
		QString pwdOld{ enterPwdDialog->findChild<QLineEdit*>("PasswordEditor")->text() };
		
		CreatePasswordDialog* pwdDialog{ new CreatePasswordDialog(this) };
		int state{ pwdDialog->exec() };
		if (state == QDialog::Accepted) {
			QString passwd{ pwdDialog->getPassword() };
			bool success{ m_cgCopy.changePassword(pwdOld, passwd) };
			if (success) {
				QMessageBox::information(this, tr("Change Password"), tr("Change Password Successfully!"));
			}
			else {
				QMessageBox::warning(
					this, tr("Some Problem"),
					tr("Some issues have prevented the password from being correctly modified.")
				);
			}
		}
		delete enterPwdDialog;
		delete pwdDialog;
		*(m_questionObj.getCurrentPtr()) = m_cgCopy;
	}
	else {
		QMessageBox::critical(this, tr("Incorrect Password"), tr("Please input correct password of this class!"));
	}
}

void ManagerDialog::tableViewSet(QTableView* tableView, QStandardItemModel* model)
{
	tableView->setModel(model);
	tableView->setShowGrid(true);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); // 关键：锁定列宽模式
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	tableView->setAlternatingRowColors(true);
	tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	tableView->horizontalHeader()->setStretchLastSection(true);
}

void ManagerDialog::onSelectChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QModelIndexList indexes = selected.indexes();
	if (indexes.isEmpty()) return;  // 无选中时跳过

	// 获取选中行的行号（所有选中单元格的行号相同，取第一个即可）
	int row = indexes.first().row();

	// 获取第二列（列索引为1）的索引（通过模型获取指定行和列的索引）
	QModelIndex secondColIndex = indexes.first().model()->index(row, 1);

	// 提取第二列的内容（转换为QString）
	QString secondColData = secondColIndex.data().toString();

	// 调用getStudentRef(QString)重载版本
	QK::Student& stuRef{ m_cgCopy.getStudentRef(secondColData) };

	// 后续赋值逻辑不变
	ui.nameEditor->setText(stuRef.getName());
	ui.idEditor->setText(stuRef.getID());
	ui.typeComboBox->setCurrentIndex(static_cast<int>(stuRef.getType()));
	ui.genderComboBox->setCurrentIndex(static_cast<int>(stuRef.getGender()));
}

