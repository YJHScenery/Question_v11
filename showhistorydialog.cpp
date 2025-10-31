// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "showhistorydialog.h"
#include "global.h"
#include <QComboBox>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>

ShowHistoryDialog::ShowHistoryDialog(QK::Question& questionRef, QK::QuestionHistory& historyRef, QWidget *parent)
	: QDialog(parent), ui(new Ui::ShowHistoryDialogClass()), m_historyRef(historyRef)
	, m_questionRef(questionRef)
{
	ui->setupUi(this);
    m_classNames = questionRef.getClassGroupNameList();
    ui->chooseClassComboBox->addItem(allClassHolder);
    ui->chooseClassComboBox->addItems(m_classNames);
    ui->chooseClassComboBox->setCurrentText(m_questionRef.getCurrentPtr()->getName());

    ui->historyTableWidget->setColumnCount(4);
    ui->historyTableWidget->setHorizontalHeaderLabels({tr("Time"), tr("Class"), tr("Mode"), tr("Results")});
    ui->historyTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->searchTextChanged(ui->chooseClassComboBox->currentText());

    connect(ui->OkButton, &QPushButton::clicked, this, [=, this]() {this->close(); });

    connect(ui->exportButton, &QPushButton::clicked, this, &ShowHistoryDialog::exportButtonClicked);

    connect(ui->chooseClassComboBox, &QComboBox::currentTextChanged, this, [=, this](const QString& text)
    {
        this->searchTextChanged(text);
    });


}

ShowHistoryDialog::~ShowHistoryDialog()
{
	delete ui;
}

void ShowHistoryDialog::loadHistory(const QVector<QK::QuestionRecord>& records) const
{

    ui->historyTableWidget->setRowCount(static_cast<int>(records.size()));
    for (int i = 0; i < records.size(); i++) {
        const auto& record = records[i];
        ui->historyTableWidget->setItem(i, 0, new QTableWidgetItem(record.timestamp.toString()));
        ui->historyTableWidget->setItem(i, 1, new QTableWidgetItem(record.className));
        ui->historyTableWidget->setItem(i, 2, new QTableWidgetItem(record.mode));

        QString resultText;
        const qsizetype allSize{ record.results.size()};
        if (allSize > 16) {
            for (qsizetype j{ 0 }; j < 16; j++) {
                resultText += record.results[j].getName() + "(" + record.results[j].getID() + "),";
            }
            resultText += "...";
        }
        else {
            for (const auto& stu : record.results) {
                resultText += stu.getName() + "(" + stu.getID() + "),";
            }
        }
        ui->historyTableWidget->setItem(i, 3, new QTableWidgetItem(resultText));
    }
}

void ShowHistoryDialog::searchTextChanged(const QString& text) const
{
    QVector<QK::QuestionRecord> results;
    if (text == allClassHolder){
        results = m_historyRef.getAllRecords();
    }else{
        for (const auto& record : m_historyRef.getAllRecords()){
            if (record.className == text){
                results.append(record);
            }
        }
    }
    this->loadHistory(results);
}

void ShowHistoryDialog::exportButtonClicked()
{
    const QString className{ui->chooseClassComboBox->currentText()};
    auto state{QK::XlsxIEState::UnknownError};
    if (className==allClassHolder){
        const QString filePath = QFileDialog::getSaveFileName(this, tr("Export History"), QString("%1/result.xlsx").arg(QDir::homePath()), "XLSX文件 (*.xlsx)");
        if (filePath.isEmpty()) return;
        state = m_historyRef.exportToXlsx(filePath);
    }
    else{
        const QString filePath = QFileDialog::getSaveFileName(this, tr("Export History"), QString("%1/%2.xlsx").arg(QDir::homePath(), className), "XLSX文件 (*.xlsx)");
        if (filePath.isEmpty()) return;
        state = m_historyRef.exportToXlsx(className, filePath);
    }
    if (state ==  QK::XlsxIEState::ExportSuccess) {
        QMessageBox::information(this, tr("Success"), tr("Export Successfully! "));
    }
    else {
        QMessageBox::warning(this, tr("Failed"), tr("Failed to export history"));
    }
}


