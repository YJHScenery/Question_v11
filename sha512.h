// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include "global.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QByteArray>
#include <QString>
#include <QStringList>

namespace QK {
    inline QString sha512WithSalt(const QString &password, QByteArray &salt) {
        // 生成 16个字节 随机盐（若未提供）
        if (salt.isEmpty()) {
            salt.resize(16);
            QRandomGenerator::global()->fillRange(reinterpret_cast<quint32 *>(salt.data()),
                                                  salt.size() / sizeof(quint32));
        }

        QByteArray saltedPassword = salt + password.toUtf8();

        QByteArray hash = QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha512);

        return salt.toHex() + "$" + hash.toHex();
    }

    inline bool verifyPassword(const QString &password, const QString &hashedData) {
        QStringList parts = hashedData.split("$");
        if (parts.size() != 2) {
            return false;
        }
        QByteArray salt = QByteArray::fromHex(parts[0].toLatin1());

        QString newHash = sha512WithSalt(password, salt);
        return (newHash == hashedData);
    }
}
