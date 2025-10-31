// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"

#include "questionmainwindow.h"
#include "ui_questionmainwindow.h"
#include <array>

// 试图支持多种硬件平台，暂时只支持 Arduino
bool QuestionMainWindow::createHardware(QK::HardwareType type)
{
    if (type == QK::HardwareType::Arduino) {
        bool initStatus{ false };
        bool bindStatus{ false };
        if (m_hardware == nullptr) {
            m_hardware = new QK::QuestionArduino(MY_ARDUINO_REMOTE_VID_01, MY_ARDUINO_REMOTE_PID_01, this);
        }
        initStatus = m_hardware->initSerialPort(QSerialPort::Baud9600, QSerialPort::Data8, QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);
        bindStatus = m_hardware->bindSerialPort();
        return initStatus && bindStatus && m_hardware != nullptr;
    }
    return false;

}

void QuestionMainWindow::setHardware(QK::QuestionHardware_base* hardwarePtr)
{
    if (m_hardware) {
        // 断开与旧硬件接口的连接
        disconnect(m_hardware, nullptr, this, nullptr);
    }
    m_hardware = hardwarePtr;
    this->connectHardwareSignals();
}


void QuestionMainWindow::connectHardwareSignals()
{
    if (!m_hardware) {
        qDebug() << "Hardware not initialized.";
        return;
    }
    m_statusLabel->setText(tr("No hardware connected"));
    // this->statusBar()->showMessage(m_hardware->getStatusMessage(), 3000);
    // qDebug() << "Hardware initialized.";
    // 连接硬件接口的信号到主窗口的槽
    connect(m_hardware, &QK::QuestionHardware_base::updateNumber,
        this, [=, this](const int number) {
            QString tempText{ ui->countEditor->text() };
            if (tempText.startsWith("0") || tempText.size() >= 4) {
                tempText.clear();
            }
            ui->countEditor->setText(tempText + QString::number(number));
        });

    connect(m_hardware, &QK::QuestionHardware_base::startSignal,
        this, [=, this]() {
            if (ui->tabWidget->currentIndex() == 0) {
                emit ui->startButton->clicked();
            }
            else if (ui->tabWidget->currentIndex() == 1) {
                emit ui->startTraversalButton->clicked();
            }
        });

    connect(m_hardware, &QK::QuestionHardware_base::infoOccurred,
        this, &QuestionMainWindow::onInfoOccurred_H);

    connect(m_hardware, &QK::QuestionHardware_base::resetSignal,
        this, &QuestionMainWindow::onButtonResetClicked_H);

    connect(m_hardware, &QK::QuestionHardware_base::minusSignal,
        this, [=, this]() {
            ui->chooseCountSlider->triggerAction(QAbstractSlider::SliderSingleStepSub);
        });

    connect(m_hardware, &QK::QuestionHardware_base::plusSignal,
        this, [=, this]() {
            ui->chooseCountSlider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        });

    connect(m_hardware, &QK::QuestionHardware_base::tab0Signal,
        this, [=, this]() {
            ui->tabWidget->setCurrentIndex(0);
        });

    connect(m_hardware, &QK::QuestionHardware_base::tab1Signal,
        this, [=, this]() {
            ui->tabWidget->setCurrentIndex(1);
        });

    connect(m_hardware, &QK::QuestionHardware_base::weightSignal,
        this, [=, this]() {
            static constexpr std::array<std::array<bool, 2>, 4> checkedStatus{ {{true, false}, {false, true}, {true, true}, {false, false}} };
            const std::array<bool, 2> checked{ui->weightModeCheckBox->isChecked(), ui->runAnimationButton->isChecked()};
            const auto it{std::ranges::find(checkedStatus, checked)};
            size_t index{ 0 };
            if (it != checkedStatus.end()) {
                index = std::distance(checkedStatus.begin(), it);
                if (index == 3) {
                    index = 0;
                }
                else {
                    ++index;
                }
            }
            ui->weightModeCheckBox->setChecked(checkedStatus[index][0]);
            ui->runAnimationButton->setChecked(checkedStatus[index][1]);
        });

    connect(m_hardware, &QK::QuestionHardware_base::screenPlusSignal,
        this, [=, this]() {
            const int index {ui->screenComboBox->currentIndex()};
            const int count {ui->screenComboBox->count()};
            if (index < count - 1) {
                ui->screenComboBox->setCurrentIndex(ui->screenComboBox->currentIndex() + 1);
            }
            else {
                ui->screenComboBox->setCurrentIndex(0);
            }
        });

    connect(m_hardware, &QK::QuestionHardware_base::screenMinusSignal,
        this, [=, this]() {
            const size_t index {static_cast<size_t>(ui->screenComboBox->currentIndex())};
            if (index > 0) {
                ui->screenComboBox->setCurrentIndex(ui->screenComboBox->currentIndex() - 1);
            }
            else {
                ui->screenComboBox->setCurrentIndex(ui->screenComboBox->count() - 1);
            }
		});

    if (m_hardware->getHardwareType() == QK::HardwareType::Arduino) {
        auto arduino {qobject_cast<QK::QuestionArduino*>(m_hardware)};
        if (arduino) {
            connect(arduino, &QK::QuestionArduino::connectionStatusChanged,
                this, [this](bool connected) {
                    if (connected) {
                        m_statusLabel->setText(tr("Connected to Arduino"));
                    }
                    else {
                        m_statusLabel->setText(tr("Disconnected from Arduino"));
                    }
                });
        }
    }

}


void QuestionMainWindow::onInfoOccurred_H(const QString& infoMsg) const
{
    m_statusLabel->setText(infoMsg);
}


void QuestionMainWindow::onButtonResetClicked_H() const
{
    ui->countEditor->clear();
}



