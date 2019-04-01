/*
    Copyright (C) 2018 Florian Cabot <florian.cabot@epfl.ch>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef HDF5DATASETSELECT_H
#define HDF5DATASETSELECT_H

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "utils.hpp"

class HDF5DatasetSelect : public QWidget
{
	Q_OBJECT
  public:
	HDF5DatasetSelect(QWidget* parent, QString const& particlesLabel);
	bool datasetPathIsValid();
	QString getDatasetPath() const { return lineEditDataset.text(); };
  public slots:
	void load(QString const& path);
	void lineEdited(QString const& path);
	void fixLineEdit();

  signals:
	void selectedObjChanged();

  private:
	QTreeWidget tree;
	QLineEdit lineEditDataset;
	QLabel infoLabel;

	HDF5Object hdf5_obj;

	QTreeWidgetItem* constructItems(HDF5Object const& obj,
	                                QTreeWidgetItem* parent = nullptr);
	void updateInfoLabel();

	static QString pathFromItem(QTreeWidgetItem* item);
	static QTreeWidgetItem* itemFromPath(QStringList& path,
	                                     QTreeWidgetItem* rootItem);
	static HDF5Object* objFromPath(QStringList& path, HDF5Object* rootObj);
};

#endif // HDF5DATASETSELECT_H
