// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QDialog>
#include <QStandardItemModel>
#include "ui_newclassdialog.h"
#include "Question.h"

class NewClassDialog : public QDialog
{
	Q_OBJECT

public:
	explicit NewClassDialog(QK::Question& _questionObj, QWidget* parent = nullptr);

	~NewClassDialog() override;

protected:
	void initSet() const;

protected slots:
	void onClassOkButtonClicked(QString& passwdUsed);

	void onAddStudentButtonClicked(const QString& passwdUsed, QStandardItemModel* model);

	void onSaveButtonClicked();

	void onVisibleButtonClicked() const;

	void connectSlots();
private:

	Ui::NewClassDialogClass ui{};

	QK::ClassGroup m_cgCopy;

	QK::Question& m_questionRef;


};


