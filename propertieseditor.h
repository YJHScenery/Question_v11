#pragma once
// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include <QDialog>
#include <QStandardItemModel>
#include <QVector>
#include <QString>
#include <QItemSelection>
#include "ui_propertieseditor.h"
#include "Student.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PropertiesEditorClass; };
QT_END_NAMESPACE

class PropertiesEditor : public QDialog
{
	Q_OBJECT

public:
	PropertiesEditor(QK::Student& stuRef, QWidget* parent = nullptr);
	~PropertiesEditor();

	void loadShowTableView(const QVector<QString>& properties, QStandardItemModel* model);

private slots:
	void onSelectChanged(const QItemSelection& selected, const QItemSelection& deselected) const;

private:
	QK::Student& m_stuRef;
	Ui::PropertiesEditorClass *ui;
};


