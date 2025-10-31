// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QDialog>
#include "ui_createpassworddialog.h"

class CreatePasswordDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CreatePasswordDialog(QWidget *parent = nullptr);
	QString getPassword();
	~CreatePasswordDialog() override;

protected slots:
	void onVisibleButtonClicked();

	void onOKButtonClicked();

private:
	Ui::CreatePasswordDialogClass ui;
};


