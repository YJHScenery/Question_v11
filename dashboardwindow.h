#pragma once
// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"
#include <QWidget>
#include <QDialog>
#include <QtCharts>
#include "ClassDashboard.h"
#include "ui_dashboardwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DashboardWindowClass; };
QT_END_NAMESPACE

class DashboardWindow : public QDialog
{
	Q_OBJECT

public:
    DashboardWindow(QK::ClassGroup* classGroup, QK::QuestionHistory* history, QWidget* parent = nullptr);
    ~DashboardWindow();

private slots:
    // void onRefreshClicked();         // 手动刷新数据
    void onChartsUpdated();          // 图表数据更新
    void onTopNValueChanged(int val);// 调整TOP N数量

private:
	Ui::DashboardWindowClass *ui;
	QK::ClassDashboard* m_dashboard; // 数据看板管理器
	QVector<QChartView*> m_chartViews; // 图表视图列表
};


