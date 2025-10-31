// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QMainWindow>
#include "Question.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QLabel>
#include "QuestionHardware_base.h"
#include "QuestionArduino.h"
#include "QuestionHistory.h"
#include "progressbarwindow.h"

namespace Ui {
class QuestionMainWindow;
}

class QuestionMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QuestionMainWindow() = delete;

    explicit QuestionMainWindow(QK::Question& _questionObj, QK::QuestionHistory& _questionHistoryObj, QWidget* parent = nullptr);

    void flushWidgetAndQuestionObj();

    void setPlaceHolderIfEmpty() const;

    void initToolBar();

    void initLogo() const;

    void initWindowConfig() const;

    void initSliderAndSpinBox();

    void initTotalCountLabel() const;

    bool createHardware(QK::HardwareType type);

    void setHardware(QK::QuestionHardware_base* hardwarePtr);

    void connectWindowSignals();

    void connectHardwareSignals();

    void loadPropertiesToComboBox() const;

    void initSet();

    static void loadNotQuestionedTableView(const QVector<QK::Student>& students, QStandardItemModel* model);

    ~QuestionMainWindow() override;

protected slots:

    void onActionNewClassTriggered();

    void onActionClassManagerTriggered();

    void onActionShowHistoryTriggered();

    void onActionExportTriggered();

    void onActionImportTriggered();

    void onActionHelpTriggered();

    void onActionAboutQuestionTriggered();

    void onActionAboutUsTriggered();

    void onButtonStartQuestionClicked();

    void onButtonStartTraversalClicked() const;

    void onButtonResetClicked();

    void onButtonExportResultClicked();

    void onInfoOccurred_H(const QString& infoMsg) const;

    void onButtonResetClicked_H() const;

    void onActionOpenDashboardTriggered();

private:
    Ui::QuestionMainWindow* ui;

    // 绑定 Question 对象，避免使用全局变量
    QK::Question& m_questionObjectRef;

    QK::QuestionHistory& m_historyRef;

    QVector<QK::Student> m_questionedStudentsList{};

    QStandardItemModel* m_modelMainWMain{nullptr};

    QStandardItemModel* m_modelMainWTraversal{nullptr};

    QK::QuestionHardware_base* m_hardware{nullptr};

    QLabel* m_statusLabel;
    
};
