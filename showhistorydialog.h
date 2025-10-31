// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#pragma once
#include "global.h"
#include <QDialog>
#include "ui_showhistorydialog.h"
#include "QuestionHistory.h"
#include "Question.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ShowHistoryDialogClass; };
QT_END_NAMESPACE

class ShowHistoryDialog : public QDialog
{
	Q_OBJECT

public:
	ShowHistoryDialog(QK::Question& questionRef, QK::QuestionHistory& historyRef, QWidget *parent = nullptr);
	~ShowHistoryDialog();

	void loadHistory(const QVector<QK::QuestionRecord>& records) const;
private slots:
	void searchTextChanged(const QString& text) const;
	void exportButtonClicked();
private:
	Ui::ShowHistoryDialogClass *ui;
	QK::QuestionHistory& m_historyRef;
	QK::Question& m_questionRef;
	QVector<QString> m_classNames;
	inline static const QString allClassHolder{"..."};
};

