// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only

#include "global.h"
#include "propertieseditor.h"
#include <QStandardItemModel>
#include <QTableView>

PropertiesEditor::PropertiesEditor(QK::Student& stuRef, QWidget *parent)
	: QDialog(parent), m_stuRef(stuRef)
	, ui(new Ui::PropertiesEditorClass())
{
	ui->setupUi(this);
	QStandardItemModel* model = new QStandardItemModel(0, 1, this);
	model->setHorizontalHeaderLabels({tr("%1's properties").arg(stuRef.getName())});
	ui->propertyTableView->setModel(model);
	ui->propertyTableView->setShowGrid(true);
	ui->propertyTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->propertyTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui->propertyTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->propertyTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->propertyTableView->setAlternatingRowColors(true);
	ui->propertyTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->propertyTableView->horizontalHeader()->setStretchLastSection(true);

	ui->studentNameLabel->setText(tr("Properties of Student %1(ID: %2)").arg(m_stuRef.getName(), m_stuRef.getID()));
	ui->propertyLineEdit->setPlaceholderText(tr("Search or type a property here..."));

	this->loadShowTableView(m_stuRef.getProperties(), model);

	QItemSelectionModel* selectionModel = ui->propertyTableView->selectionModel();
	connect(selectionModel, &QItemSelectionModel::selectionChanged, 
		[&](const QItemSelection& selected, const QItemSelection& deselected) {
			this->onSelectChanged(selected, deselected);
		});

	connect(ui->propertyLineEdit, &QLineEdit::textChanged, this,
		[=, this](const QString& text) {
			QVector<QString> searchResults;
			for (const auto& property : m_stuRef.getProperties()) {
				if (property.contains(text)) {
					searchResults.append(property);
				}
			}
			this->loadShowTableView(searchResults, model);
		});

	connect(ui->addButton, &QPushButton::clicked, this, [=, this]() {
		const QString temp{ ui->propertyLineEdit->text() };
		if (temp.isEmpty()) return;
		m_stuRef.addProperty(temp);
		this->loadShowTableView(m_stuRef.getProperties(), model);
		ui->propertyLineEdit->clear();
		});

	connect(ui->removeButton, &QPushButton::clicked, this, [=, this]() {
		m_stuRef.deleteProperty(ui->propertyLineEdit->text());
		this->loadShowTableView(m_stuRef.getProperties(), model);
		ui->propertyLineEdit->clear();
		});
}

void PropertiesEditor::onSelectChanged(const QItemSelection& selected, const QItemSelection& deselected) const
{
	QModelIndexList indexes = selected.indexes();
	if (indexes.isEmpty()) return;
	const int row = indexes.first().row();
	try {
		const QString property = m_stuRef.getProperties().at(row);
		ui->propertyLineEdit->setText(property);
	}
	catch (std::out_of_range) {
		ui->propertyLineEdit->setText("");
	}
}

PropertiesEditor::~PropertiesEditor()
{
	delete ui;
}


void PropertiesEditor::loadShowTableView(const QVector<QString>& properties, QStandardItemModel* model)
{
	const int rows = model->rowCount();
	if (rows > 0) {
		model->removeRows(0, rows);
	}
	for (const QString& property : properties) {
		QStandardItem* propertyItem{ new QStandardItem(property) };
		model->appendRow(propertyItem);
	}
}

