// Copyright (C) 2025 叶景皓 Ye Jinghao
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

/*
* 此文件基于 Qt 的 XML 模块，用于读写配置文件。
* 在 Question 11.0.x 版本中，仅存在两个属性：关闭软件时选中的班级和指定的人数（通过滑块设置的人数而不是手动输入的人数）。
* 示例 xml 内容如下：
<?xml version="1.0" encoding="UTF-8"?>
<Config>
 <Class>示例班级</Class>
 <Number>1</Number>
</Config>
*/

#pragma once
#include "global.h"
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QIODevice>

struct WindowConfig 
{
    QString m_class;
    int m_number;
    bool m_weightChecked;
    bool m_animateChecked;
};


inline void saveWindowConfig(const QString& filePath, const WindowConfig& config) {
    QDomDocument doc;

    // 添加XML声明
    const QDomProcessingInstruction header = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8")");
    doc.appendChild(header);

    // 创建根节点
    QDomElement root {doc.createElement("config")};
    doc.appendChild(root);

    // 添加 class 节点
    QDomElement classElem {doc.createElement("class")};
    classElem.appendChild(doc.createTextNode(config.m_class));
    root.appendChild(classElem);

    // 添加 number 节点
    QDomElement numElem {doc.createElement("number")};
    numElem.appendChild(doc.createTextNode(QString::number(config.m_number)));
    root.appendChild(numElem);
    
    // 添加 weightChecked 节点
    QDomElement weightCheckedElem {doc.createElement("weightChecked")};
    weightCheckedElem.appendChild(doc.createTextNode(QString::number(static_cast<int>(config.m_weightChecked))));
    root.appendChild(weightCheckedElem);

    // 添加 animateChecked 节点
    QDomElement animateCheckedElem {doc.createElement("animateChecked")};
    animateCheckedElem.appendChild(doc.createTextNode(QString::number(static_cast<int>(config.m_animateChecked))));
    root.appendChild(animateCheckedElem);


    // 保存到文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << doc.toString();
        file.close();
    }
}

inline bool loadWindowConfig(const QString& filePath, WindowConfig& config) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement root {doc.documentElement()};
    if (root.isNull())
        return false;

    // 解析 class 节点
    const QDomElement classElem {root.firstChildElement("class")};
    if (!classElem.isNull())
        config.m_class = classElem.text();

    // 解析 number 节点（需转换类型）
    const QDomElement numElem {root.firstChildElement("number")};
    if (!numElem.isNull())
        config.m_number = numElem.text().toInt();

    // 解析 weightChecked 节点（需转换类型）
    const QDomElement weightCheckedElem {root.firstChildElement("weightChecked")};
    if (!weightCheckedElem.isNull())
        config.m_weightChecked = static_cast<bool>(weightCheckedElem.text().toInt());

    // 解析 animateChecked 节点（需转换类型）
    const QDomElement animateCheckedElem {root.firstChildElement("animateChecked")};
    if (!animateCheckedElem.isNull())
        config.m_animateChecked = static_cast<bool>(animateCheckedElem.text().toInt());

    return true;
}