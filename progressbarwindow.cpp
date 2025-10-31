// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "progressbarwindow.h"
#include <QProgressBar>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout> 
#include <cmath>

ProgressBarWindow::ProgressBarWindow(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ProgressBarWindowClass())
{
	ui->setupUi(this);
    // 创建进度条并初始化
    processBar = new QProgressBar(this);
    processBar->setRange(0, 100);  // 范围0~100
    processBar->setValue(0);       // 初始值0
    processBar->setTextVisible(true);  // 显示百分比文字

    // 布局管理（将进度条居中显示在窗口中）
    auto* layout{ new QVBoxLayout(this) };
    layout->addWidget(processBar);
    setLayout(layout);

    // 初始化定时器
    progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &ProgressBarWindow::updateProgress);
}

ProgressBarWindow::~ProgressBarWindow()
{
    if (progressTimer->isActive()) {
        progressTimer->stop();
    }
	delete ui;
}

void ProgressBarWindow::startProgress() {
    processBar->setValue(0);
    elapsedTimer.start();  // 启动计时（记录当前时间点）
    progressTimer->start(50);
}

void ProgressBarWindow::updateProgress() {
    constexpr double totalTime{ 6000 };  // 6秒
    // 从 start() 到现在的毫秒数
    const qint64 elapsed{ elapsedTimer.elapsed() }; 

    if (elapsed >= totalTime) {
        processBar->setValue(100);
        progressTimer->stop();
        this->close();
        return;
    }

    const double ratio{ elapsed / totalTime };
    constexpr int n{ 4 };
    const double progress{ 100 * (1 - pow(1 - ratio, n)) };
    processBar->setValue(static_cast<int>(progress));
}