// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once
#include "global.h"
#include <functional>

#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "QuestionHardware_base.h"
namespace QK {

class QuestionArduino final : public QuestionHardware_base
{
	Q_OBJECT
public:
	QuestionArduino(unsigned vid, unsigned pid, QObject* parent = nullptr);
	~QuestionArduino() override;

	bool bindSerialPort() override;

	bool initSerialPort(QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl) override;

	bool sendData(const QByteArray& data) override;

	bool isConnected() const override;

	QString getPortName() const override;

	QString getStatusMessage() const override;

	HardwareType getHardwareType() const override { return HardwareType::Arduino; }

signals:
	void connectionStatusChanged(bool connected);

private slots:

    // 处理串口数据
	void onDataReceived();

	// 处理串口错误
	void onSerialErrorOccurred(QSerialPort::SerialPortError error);

	void checkPortAvailability();

private:
	QByteArray m_receiveBuffer;

	void parseData(const QByteArray& data);

	void attemptReconnect();

	QTimer* m_portCheckTimer;

	bool m_wasConnected;


};

}