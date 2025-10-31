// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once
#include "global.h"

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QString>

namespace QK 
{
    enum class HardwareType : quint32
    {
        None,
        Arduino,
        RaspberryPi,
        STM32,
		ESP32,
        Other
	};

    class QuestionHardware_base : public QObject
    {
        Q_OBJECT
    public:
        QuestionHardware_base(unsigned vid, unsigned pid, QObject* parent = nullptr): 
			m_vid(vid),
            m_pid(pid),
            QObject(parent),
			m_serialPort(new QSerialPort(this))
        { };

        ~QuestionHardware_base() override = default;

        virtual bool bindSerialPort() = 0;

        virtual bool initSerialPort(QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, 
            QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl) = 0;

        virtual bool sendData(const QByteArray& data) = 0;

        [[nodiscard]] virtual QString getPortName() const = 0;

        [[nodiscard]] virtual QString getStatusMessage() const = 0;

		[[nodiscard]] virtual bool isConnected() const = 0;

        [[nodiscard]] virtual HardwareType getHardwareType() const { return HardwareType::None; };

        [[nodiscard]] unsigned getVid() const { return m_vid; };

        [[nodiscard]] unsigned getPid() const { return m_pid; };

    signals:
        void updateNumber(int number);

        void startSignal();

        void resetSignal();

        void weightSignal();

        void tab0Signal();

		void tab1Signal();

        void screenPlusSignal();

        void screenMinusSignal();

        void infoOccurred(const QString& errorMsg);

        void minusSignal();

        void plusSignal();

    protected:
        unsigned m_vid;

        unsigned m_pid;

        QString m_portName;

        QString m_statusMessage;
        
        QSerialPort* m_serialPort;

    };

}
