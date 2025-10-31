#pragma once
// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include <QObject>
#include <QtCharts>
#include <QMap>
#include <QChartGlobal>


#include "ClassGroup.h"
#include "QuestionHistory.h"

#ifdef QT_CHARTS_USE_NAMESPACE
QT_CHARTS_USE_NAMESPACE
#endif
namespace QK {
    class ClassDashboard : public QObject {
        Q_OBJECT

    public:
        explicit ClassDashboard(QObject* parent = nullptr);

        // 绑定班级和历史记录（必须调用）
        void bindData(ClassGroup* classGroup, QuestionHistory* history);

        // 生成各类图表
        QChart* getTypeDistributionChart();      // 学生类型分布饼图
        QChart* getGenderRatioChart();           // 性别比例柱状图
        QChart* getQuestionFrequencyChart(int topN); // 抽取频率TOP N条形图
        QChart* getPropertyTagChart();           // 属性标签云图（简化版）

        // 刷新所有数据
        void refresh();

    signals:
        void chartsUpdated(); // 图表数据更新信号

    private:
        // 数据计算
        QMap<QString, int> calculateTypeDistribution() const;   // 计算类型分布
        QMap<QString, int> calculateGenderRatio() const;        // 计算性别比例
        QMap<QString, int> calculateQuestionFrequency() const;      // 计算抽取频率
        QMap<QString, int> calculatePropertyDistribution() const;// 计算属性分布

        ClassGroup* m_classGroup = nullptr;       // 关联的班级
        QuestionHistory* m_history = nullptr;     // 关联的历史记录

        // 缓存计算结果（避免重复计算）
        QMap<QString, int> m_typeData;
        QMap<QString, int> m_genderData;
        QMap<QString, int> m_questionData;
        QMap<QString, int> m_propertyData;
    };

} // namespace QK
