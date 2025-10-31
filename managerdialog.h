// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QDialog>
#include "ui_managerdialog.h"
#include "Question.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include "QuestionHistory.h"

class ManagerDialog : public QDialog
{
	Q_OBJECT

public:
	ManagerDialog(QK::QuestionHistory& historyRef, QK::Question& _questionObj, QWidget *parent = nullptr);

	static void loadStudentShowTableView(const QVector<QK::Student>& students, QStandardItemModel* model);

	void connectSlots();

	~ManagerDialog() override;

protected slots:
	void onSelectChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void onAdditionModeRadioButtonToggled(bool checked);

	void onReviseModeRadioButtonToggled(bool checked);

	void onDeleteButtonClicked();

	void onSaveButtonClicked();

	void onChangePwdButtonClicked();

	void onAddOrReviseButtonClicked(QStandardItemModel* model);

private:
	void tableViewSet(QTableView* tableView, QStandardItemModel* model);
	
	Ui::ManagerDialogClass ui;
	QK::ClassGroup m_cgCopy;
	QK::QuestionHistory& m_historyRef;
	QK::Question& m_questionObj;
	QStandardItemModel* m_selectModel;
};

