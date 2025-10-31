// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "enterpassworddialog.h"
#include <QMessageBox>
#include "Question.h"

EnterPasswordDialog::EnterPasswordDialog(QK::Question& _questionObj, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    ui.passwordEditor->setObjectName("PasswordEditor");
    ui.passwordEditor->setEchoMode(QLineEdit::Password);
    ui.passwordEditor->setPlaceholderText(tr("Type your password here"));
	QIcon invisible_icon(":/images/resources/images/invisible.png");
	// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.visibleButton->setIcon(invisible_icon);
	ui.visibleButton->setIconSize(ui.visibleButton->size());
	connect(ui.visibleButton, &QToolButton::clicked, this, &EnterPasswordDialog::onVisibleButtonClicked);

    connect(ui.okButton, &QPushButton::clicked, this, [&_questionObj, this] {
        QK::ClassGroup* cgPointer{ _questionObj.getCurrentPtr() };
        QString passwdEntered{ui.passwordEditor->text()};
        bool isCorrectPasswd{ cgPointer->unlock(passwdEntered) };
        if (isCorrectPasswd) { // 自定义验证逻辑
            this->accept();    // 关闭并返回Accepted(1)
        }
        else {
            QMessageBox::warning(this, tr("Incorrect Password"), tr("This Password is Incorrect! Please Enter it Again."));
            ui.passwordEditor->clear();
        }
        });

    connect(ui.cancelButton, &QPushButton::clicked, this, [this] {
        this->reject(); // 关闭并返回Rejected(0)
        });
}

void EnterPasswordDialog::onVisibleButtonClicked()
{
	if (ui.passwordEditor->echoMode() == QLineEdit::Normal) {
		QIcon invisible_icon(":/images/resources/images/invisible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(invisible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Password);
	}
	else {
		QIcon visible_icon(":/images/resources/images/visible.png");
		// invisible_pixmap = invisible_pixmap.scaled(ui.visibleButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui.visibleButton->setIcon(visible_icon);
		ui.visibleButton->setIconSize(ui.visibleButton->size());
		ui.passwordEditor->setEchoMode(QLineEdit::Normal);
	}
}
EnterPasswordDialog::~EnterPasswordDialog()
{}


