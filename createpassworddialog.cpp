// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "createpassworddialog.h"
#include <QMessageBox>

CreatePasswordDialog::CreatePasswordDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.passwordEditor->setEchoMode(QLineEdit::Password);
	ui.passwordEditor->setPlaceholderText(tr("Type your password here"));
	ui.passwordConfirmEditor->setEchoMode(QLineEdit::Password);
	ui.passwordConfirmEditor->setPlaceholderText(tr("Confirm your password here"));

	QIcon invisible_icon(":/images/resources/images/invisible.png");
	// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.visibleButton->setIcon(invisible_icon);
	ui.visibleButton->setIconSize(ui.visibleButton->size());

	connect(ui.okButton, &QPushButton::clicked, this, &CreatePasswordDialog::onOKButtonClicked);

	connect(ui.visibleButton, &QToolButton::clicked, this, &CreatePasswordDialog::onVisibleButtonClicked);
}

void CreatePasswordDialog::onVisibleButtonClicked()
{
	if (ui.passwordEditor->echoMode() == QLineEdit::Normal) {
		QIcon invisible_icon(":/images/resources/images/invisible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(invisible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Password);
		ui.passwordConfirmEditor->setEchoMode(QLineEdit::Password);
	}
	else {
		QIcon visible_icon(":/images/resources/images/visible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(visible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Normal);
		ui.passwordConfirmEditor->setEchoMode(QLineEdit::Normal);
	}
}
QString CreatePasswordDialog::getPassword()
{
	return ui.passwordEditor->text();
}

void CreatePasswordDialog::onOKButtonClicked()
{
	QString pwd{ ui.passwordEditor->text() };
	QString pwdC{ ui.passwordConfirmEditor->text() };
	if (pwd == pwdC) {
		if (pwd.length() < 8) {
			QMessageBox::warning(this, tr("Short Password"), tr("The password must be AT LEAST 8 characters long."), QMessageBox::Ok);
			ui.passwordEditor->clear();
			ui.passwordConfirmEditor->clear();
		}
		else {
			this->accept();
		}
	}
	else {
		QMessageBox::warning(this, tr("Password Error"),                      // 对话框标题
			tr("The two passwords entered is inconsistent"),   // 错误内容
			QMessageBox::Ok);
		ui.passwordEditor->clear();
		ui.passwordConfirmEditor->clear();
	}
}

CreatePasswordDialog::~CreatePasswordDialog()
{

}
