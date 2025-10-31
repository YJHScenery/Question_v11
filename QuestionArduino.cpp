// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#include "global.h"

#include "QuestionArduino.h"
#include <QMessageBox>

namespace QK 
{
	QuestionArduino::QuestionArduino(unsigned vid, unsigned pid, QObject* parent) :
		QuestionHardware_base(vid, pid, nullptr),
		m_portCheckTimer(new QTimer(this)),
		m_wasConnected(false)
	{
		m_portName = m_serialPort->portName();
		connect(m_serialPort, &QSerialPort::readyRead, this, &QuestionArduino::onDataReceived);
		connect(m_serialPort, &QSerialPort::errorOccurred, this, &QuestionArduino::onSerialErrorOccurred);

		m_portCheckTimer->setInterval(1000); // 1秒检查一次
		connect(m_portCheckTimer, &QTimer::timeout, this, &QuestionArduino::checkPortAvailability);
		m_portCheckTimer->start();

	}

	QuestionArduino::~QuestionArduino()
	{
		if (m_serialPort->isOpen()) {
			m_serialPort->close();
		}
	}

	bool QuestionArduino::bindSerialPort()
	{
		// m_statusMessage = "No port found";
		for (auto& info : QSerialPortInfo::availablePorts()) {
			if (info.vendorIdentifier() == m_vid && info.productIdentifier() == m_pid) {
				m_serialPort->setPort(info);
				m_portName = info.portName();
				m_statusMessage = m_portName;
				qDebug() << "Found port:" << m_statusMessage;
				break;
			}
			else {
				// m_statusMessage = "Port not found";
				qDebug() << " Port:" << info.portName() << " not matched.";
			}
		}
		if (m_portName.isEmpty()) {
			m_portName = tr("No port found");
			emit infoOccurred(tr("Can't find the serial port."));
            return false;
		}
		if (m_serialPort->open(QIODevice::ReadWrite)) {
			qDebug() << "Arduino connected on port:" << m_portName;
			return true;
		}
		else {
			emit infoOccurred(tr("Failed to open port %1: %2")
				.arg(m_portName)
				.arg(m_serialPort->errorString()));
			return false;
		}
	}

	bool QuestionArduino::initSerialPort(
		QSerialPort::BaudRate baudRate, 
		QSerialPort::DataBits dataBits, 
		QSerialPort::Parity parity, 
		QSerialPort::StopBits stopBits, 
		QSerialPort::FlowControl flowControl
	)
	{
		if (m_serialPort != nullptr) {
			m_serialPort->setBaudRate(baudRate);
			m_serialPort->setDataBits(dataBits);
			m_serialPort->setParity(parity);
			m_serialPort->setStopBits(stopBits);
			m_serialPort->setFlowControl(flowControl);
			return true;
		}
		else {
			QMessageBox::critical(nullptr, "Error", "Serial port not initialized.");
			return false;
		}
	}

	bool QuestionArduino::sendData(const QByteArray& data)
	{
		if (!m_serialPort->isOpen()) {
			emit infoOccurred(tr("Serial port not open"));
			return false;
		}
		qint64 bytesWritten{ m_serialPort->write(data) };
		return bytesWritten == data.size();
	}

	bool QuestionArduino::isConnected() const
	{
		return m_serialPort->isOpen();
	}

	QString QuestionArduino::getPortName() const
	{
		return m_portName;
	}

	QString QuestionArduino::getStatusMessage() const
	{
		return m_statusMessage;
	}
	
	void QuestionArduino::onDataReceived()
	{
		if (m_serialPort->bytesAvailable() <= 0)
			return;

		QByteArray newData = m_serialPort->readAll();

		m_receiveBuffer.append(newData);

		qDebug() << "Buffer now:" << m_receiveBuffer;

		int endIndex;
		while ((endIndex = m_receiveBuffer.indexOf('\n')) != -1) {
			QByteArray frame = m_receiveBuffer.left(endIndex).trimmed();
			m_receiveBuffer.remove(0, endIndex + 1);

			if (!frame.isEmpty()) {
				parseData(frame);
			}
		}
	}

	void QuestionArduino::parseData(const QByteArray& data)
	{

		const QMap<int, std::function<void()>> commandSignalsMap{
			{ static_cast<int>('s'), [this]() { emit startSignal(); } },
			{ static_cast<int>('r'), [this]() { emit resetSignal(); } },
			{ static_cast<int>('-'), [this]() { emit minusSignal(); } },
			{ static_cast<int>('+'), [this]() { emit plusSignal(); } },
			{ static_cast<int>('<'), [this]() { emit tab0Signal(); } },
			{ static_cast<int>('>'), [this]() { emit tab1Signal(); } },
			{ static_cast<int>('w'), [this]() { emit weightSignal(); } },
			{ static_cast<int>('p'), [this]() { emit screenPlusSignal(); } },
			{ static_cast<int>('m'), [this]() { emit screenMinusSignal(); } }
		};
		bool isNumber;
		qDebug() << "Received data:" << data;
		int number = data.toInt(&isNumber, 16);
		if (!isNumber) {
			emit infoOccurred(tr("Invalid data: %1").arg(data));
		}
		else {
			if (number >= 0 && number <= 9) {
				emit updateNumber(number);
			}
			else {
				auto it = commandSignalsMap.find(number);
				if (it != commandSignalsMap.end()) {
					it.value()();
				}
				else {
					emit infoOccurred(tr("Unknown command: %1").arg(number));
				}
			}
		}
		
	}

	void QuestionArduino::onSerialErrorOccurred(QSerialPort::SerialPortError error)
	{
		if (error == QSerialPort::NoError) return;

		// 常见的拔出相关错误
		if (error == QSerialPort::DeviceNotFoundError ||
			error == QSerialPort::ResourceError ||
			error == QSerialPort::PermissionError) {
			if (m_serialPort->isOpen()) {
				m_serialPort->close();
			}
			emit connectionStatusChanged(false);
			emit infoOccurred(tr("Device disconnected: %1").arg(m_serialPort->errorString()));
		}
		else {
			emit infoOccurred(tr("Serial error: %1").arg(m_serialPort->errorString()));
		}
	}

	void QuestionArduino::checkPortAvailability()
	{
		bool portExists = false;

		// 枚举当前可用串口，判断目标串口是否存在
		for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
			if (info.vendorIdentifier() == m_vid && info.productIdentifier() == m_pid) {
				portExists = true;
				break;
			}
		}

		if (!m_wasConnected && portExists) {
			emit infoOccurred(tr("Device detected, attempting to reconnect..."));
			attemptReconnect();
		}

		if (m_wasConnected && !portExists && m_serialPort->isOpen()) {
			m_serialPort->close();
			emit connectionStatusChanged(false);
			emit infoOccurred(tr("Device disconnected"));
			m_statusMessage = "Device disconnected";
		}

		m_wasConnected = isConnected();
	}

	// 尝试重连
	void QuestionArduino::attemptReconnect()
	{
		if (m_serialPort->isOpen()) {
			m_serialPort->close();
		}

		if (this->bindSerialPort() && 
			this->initSerialPort(QSerialPort::Baud9600, QSerialPort::Data8, QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl)) 
		{
			emit connectionStatusChanged(true);
			emit infoOccurred(tr("Connected successfully: %1").arg(m_portName));
		}
	}


}
