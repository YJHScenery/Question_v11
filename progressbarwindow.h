// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#pragma once
#include "global.h"
#include <QDialog>
#include "ui_progressbarwindow.h"
#include <QProgressBar>
#include <QElapsedTimer> 
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class ProgressBarWindowClass; };
QT_END_NAMESPACE

class ProgressBarWindow : public QDialog
{
	Q_OBJECT

public:
	ProgressBarWindow(QWidget *parent = nullptr);
	~ProgressBarWindow();
    void startProgress();

private slots:
    // 定时更新进度
    void updateProgress();

private:
    QProgressBar* processBar;  // 进度条
    QTimer* progressTimer;     // 定时器
    QElapsedTimer elapsedTimer; // 计时器
	Ui::ProgressBarWindowClass *ui;
};


