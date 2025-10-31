// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QDialog>
#include "ui_enterpassworddialog.h"
#include "Question.h"

class EnterPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	EnterPasswordDialog(QK::Question& _questionObj, QWidget *parent = nullptr);
	~EnterPasswordDialog();

protected slots:
	void onVisibleButtonClicked();

private:
	Ui::EnterPasswordDialogClass ui;
};


