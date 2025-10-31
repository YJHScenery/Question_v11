// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"
#include "ClassDashboard.h"
#include <QRandomGenerator>
#include <algorithm>
#include <QMap>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

namespace QK {

    ClassDashboard::ClassDashboard(QObject* parent) : QObject(parent) {}

    void ClassDashboard::bindData(ClassGroup* classGroup, QuestionHistory* history) {
        m_classGroup = classGroup;
        m_history = history;
    }

    void ClassDashboard::refresh() {
        if (!m_classGroup) return;

        // 重新计算所有数据
        m_typeData = calculateTypeDistribution();
        m_genderData = calculateGenderRatio();
        m_questionData = calculateQuestionFrequency();
        m_propertyData = calculatePropertyDistribution();

        emit chartsUpdated(); // 通知UI更新图表
    }

    // 1. 学生类型分布（Excluded/Junior/Common/Senior）
    QMap<QString, int> ClassDashboard::calculateTypeDistribution() const
    {
        QMap<QString, int> data;
        if (!m_classGroup) return data;

        const auto& students {m_classGroup->getAllStudents()};
        for (const auto& stu : students) {
            const static QMap<StudentType, QString> typeMap{
                {StudentType::Excluded, tr("Excluded")},
                {StudentType::Junior, tr("Junior")},
                {StudentType::Common, tr("Common")},
                {StudentType::Senior, tr("Senior")}
            };
            data[typeMap.value(stu.getType())]++;
        }
        return data;
    }

    // 2. 性别比例（男/女/未知）
    QMap<QString, int> ClassDashboard::calculateGenderRatio() const
    {
        QMap<QString, int> data;
        if (!m_classGroup) return data;

        const auto& students = m_classGroup->getAllStudents();
        for (const auto& stu : students) {
            static const QMap<StudentGender, QString> genderMap{
                {StudentGender::Unknown, tr("Unknown")},
                {StudentGender::Boy, tr("Boy")},
                {StudentGender::Girl, tr("Girl")}
            };
            data[genderMap.value(stu.getGender())]++;
        }
        return data;
    }

    QChart* ClassDashboard::getTypeDistributionChart() {
        auto* chart{ new QChart() };
        chart->setTitle(tr("Type Distribution"));
        chart->setAnimationOptions(QChart::SeriesAnimations);

        // 饼状图
        auto* series{ new QPieSeries() };
        for (auto it = m_typeData.begin(); it != m_typeData.end(); ++it) {
            QPieSlice* slice{ series->append(it.key() + "(" + QString::number(it.value()) + ")", it.value()) };
            // 随机生成鲜明颜色
            const QColor color(QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256));
            slice->setColor(color);
            slice->setLabelVisible(true);
        }
        series->setName(tr("Type Distribution"));
        chart->addSeries(series);
        chart->legend()->setAlignment(Qt::AlignRight);
        return chart;
    }

    QChart* ClassDashboard::getGenderRatioChart() {
        auto* chart {new QChart()};
        chart->setTitle(tr("Gender Ratio"));
        chart->setAnimationOptions(QChart::SeriesAnimations);

        // 柱状图
        auto* series{ new QBarSeries()};
        auto* set {new QBarSet(tr("Number of Students"))};

        QStringList categories;
        for (auto it = m_genderData.begin(); it != m_genderData.end(); ++it) {
            categories << it.key();
            *set << it.value();
        }
        series->setName(tr("Gender Ratio"));
        series->append(set);
        chart->addSeries(series);

        // 创建X轴并关联
        auto* axisX{ new QBarCategoryAxis()};
        axisX->setCategories(categories);
        chart->addAxis(axisX, Qt::AlignBottom); // 添加轴到图表（指定位置）
        series->attachAxis(axisX); 

        // 创建Y轴并关联
        auto* axisY {new QValueAxis()};
        axisY->setTitleText(tr("Number of Students"));
        chart->addAxis(axisY, Qt::AlignLeft);  // 添加轴到图表（指定位置）
        series->attachAxis(axisY); 

        chart->legend()->setVisible(false);
        return chart;
    }

    // 3. 抽取频率统计（按历史记录）
    QChart* ClassDashboard::getQuestionFrequencyChart(int topN) {
        auto* chart {new QChart()};
        chart->setTitle(tr("Question Distribution TOP %1").arg(topN));
        chart->setAnimationOptions(QChart::SeriesAnimations);

        // 排序并取TOP N
        QList<QPair<QString, int>> sortedList;
        for (auto it = m_questionData.begin(); it != m_questionData.end(); ++it) {
            sortedList.append(qMakePair(it.key(), it.value()));
        }
        std::ranges::sort(sortedList,
                          [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                              return a.second > b.second;
                          });

        auto* series{ new QBarSeries()};
        auto* set {new QBarSet(tr("Question Times"))};
        QStringList categories;
        QString name{};
        QTextStream stream(&name);
        for (int count = 0; const auto& pair : sortedList) {
            if (count >= topN) break;
            if (pair.second == 0) continue;
            categories << pair.first;
            *set << pair.second;
            stream << pair.first << "(" << QString::number(pair.second) << ")\n";
            count++;
        }
        series->setName(name);
        series->append(set);
        chart->addSeries(series);

        // 创建X轴并关联
        auto* axisX {new QBarCategoryAxis()};
        axisX->setCategories(categories);
        axisX->setLabelsAngle(-45);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        // 创建Y轴并关联
        auto* axisY {new QValueAxis()};
        axisY->setTitleText(tr("Question Times"));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        chart->legend()->setVisible(false);
        return chart;
    }

    QChart* ClassDashboard::getPropertyTagChart()
    {
        auto* chart{ new QChart() };
        chart->setTitle(tr("Property Distribution"));
        chart->setAnimationOptions(QChart::SeriesAnimations);

        auto* series{ new QPieSeries() };
        // 过滤低频属性（出现次数<2的不显示）
        QString name{};
        QTextStream stream(&name);
        for (auto it{ m_propertyData.begin() }; it != m_propertyData.end(); ++it) {
            if (it.value() < 2) continue;
            const QString content{ it.key() + "(" + QString::number(it.value()) + ")" };
            series->append(content, it.value());
            stream << content << "\n";
        }
        // 处理无属性的情况
        if (series->slices().isEmpty()) {
            name = tr("No Property");
            series->append(name, 1);
        }
        series->setName(name);
        chart->addSeries(series);
        chart->legend()->setAlignment(Qt::AlignBottom);
        return chart;
    }

    // 4. 属性分布
    QMap<QString, int> ClassDashboard::calculatePropertyDistribution() const
    {
        QMap<QString, int> data;
        if (!m_classGroup) return data;

        const auto& students {m_classGroup->getAllStudents()};
        for (const auto& stu : students) {
            for (const auto& prop : stu.getProperties()) {
                if (!prop.isEmpty()) data[prop]++;
            }
        }
        return data;
    }


    QMap<QString, int> ClassDashboard::calculateQuestionFrequency() const
    {
        QMap<QString, int> data;
        if (!m_classGroup || !m_history) return data;

        // 初始化所有学生为0次
        const auto& students{ m_classGroup->getAllStudents()};
        for (const auto& stu : students) {
            data[stu.getName() + "(" + stu.getID() + ")"] = 0;
        }

        // 从历史记录累计抽取次数
        auto records {m_history->getRecordsByClass(m_classGroup->getName())};
        for (const auto& record : records) {
            for (const auto& stu : record.results) {
                QString key {stu.getName() + "(" + stu.getID() + ")"};
                data[key]++;
            }
        }
        return data;
    }

} // namespace QK