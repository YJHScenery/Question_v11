// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "DashboardWindow.h"
#include "ui_DashboardWindow.h"

DashboardWindow::DashboardWindow(QK::ClassGroup* classGroup, QK::QuestionHistory* history, QWidget* parent)
    : QDialog(parent), ui(new Ui::DashboardWindowClass) {
    ui->setupUi(this);

    // 初始化数据看板
    m_dashboard = new QK::ClassDashboard(this);
    m_dashboard->bindData(classGroup, history);
    connect(m_dashboard, &QK::ClassDashboard::chartsUpdated,
            this, &DashboardWindow::onChartsUpdated);

    // 初始化UI控件
    ui->topNSpinBox->setRange(3, 10);
    ui->topNSpinBox->setValue(5);

    // 布局设置（使用网格布局排列图表）
    QGridLayout* gridLayout = new QGridLayout(ui->chartContainer);
    gridLayout->setSpacing(20);
    ui->chartContainer->setLayout(gridLayout);


    connect(ui->topNSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DashboardWindow::onTopNValueChanged);

    // 初始加载数据
    m_dashboard->refresh();
}

DashboardWindow::~DashboardWindow() {
    // 释放图表视图
    for (auto view : m_chartViews) {
        delete view;
    }
    delete ui;
}


void DashboardWindow::onChartsUpdated() {
    // 清除现有图表
    for (QChartView* view : m_chartViews) {
        ui->chartContainer->layout()->removeWidget(view);
        delete view;
    }
    m_chartViews.clear();

    // 创建新图表视图
    QChartView* typeView{ new QChartView(m_dashboard->getTypeDistributionChart()) };
    QChartView* genderView{ new QChartView(m_dashboard->getGenderRatioChart()) };
    QChartView* questionView{ new QChartView(m_dashboard->getQuestionFrequencyChart(ui->topNSpinBox->value())) };
    QChartView* propertyView{ new QChartView(m_dashboard->getPropertyTagChart()) };

    // 设置图表视图属性
    std::function<void(QChartView*)> setupChartView{ 
        [](QChartView* view) {
        view->setRenderHint(QPainter::Antialiasing); // 抗锯齿
        view->setMinimumHeight(300);
        QString tooltip;
        //QTextStream stream(&tooltip);
        //stream << "Names: ";
        for (const auto& series : view->chart()->series()) {
            tooltip.append(series->name());
            tooltip.append("\n");
        }
        view->setToolTip(tooltip);
        } 
    };
    setupChartView(typeView);
    setupChartView(genderView);
    setupChartView(questionView);
    setupChartView(propertyView);

    // 添加到布局（2x2网格）
    auto* grid{ qobject_cast<QGridLayout*>(ui->chartContainer->layout()) };
    grid->addWidget(typeView, 0, 0);    // 第一行第一列
    grid->addWidget(genderView, 0, 1);  // 第一行第二列
    grid->addWidget(questionView, 1, 0);    // 第二行第一列
    grid->addWidget(propertyView, 1, 1); // 第二行第二列

    m_chartViews = {typeView, genderView, questionView, propertyView};
}

void DashboardWindow::onTopNValueChanged(int val) {
    // 更新抽取频率图表的TOP N数量
    if (!m_chartViews.isEmpty() && m_chartViews[2]) {
        ui->chartContainer->layout()->removeWidget(m_chartViews[2]);
        delete m_chartViews[2];
        QChart* chart{ m_dashboard->getQuestionFrequencyChart(val) };
        auto* questionView{ new QChartView(chart) };
        questionView->setRenderHint(QPainter::Antialiasing);
        QString tooltip;
        QTextStream stream(&tooltip);
        for (const auto& series : chart->series()) {
            stream << series->name() << "\n";
        }
        questionView->setToolTip(tooltip);
        // questionView->setMinimumHeight(300);
        auto* grid{ qobject_cast<QGridLayout*>(ui->chartContainer->layout()) };
        grid->addWidget(questionView, 1, 0);
        m_chartViews[2] = questionView;
    }

}
