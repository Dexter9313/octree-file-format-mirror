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

#include "HDF5DatasetSelect.hpp"
#include <iostream>

HDF5DatasetSelect::HDF5DatasetSelect(QWidget* parent,
                                     QString const& particlesLabel)
    : QWidget(parent)
    , tree(this)
    , lineEditDataset(this)
    , infoLabel(this)
{
	QVBoxLayout* layoutV = new QVBoxLayout(this);
	layoutV->addWidget(new QLabel(particlesLabel, this));

	tree.setColumnCount(1);
	tree.setHeaderLabel(QString());
	connect(&tree, SIGNAL(itemSelectionChanged()), this, SLOT(fixLineEdit()));
	layoutV->addWidget(&tree);

	QWidget* w           = new QWidget(this);
	QHBoxLayout* layoutH = new QHBoxLayout(w);
	layoutH->addWidget(new QLabel(tr("Dataset path :"), this));
	connect(&lineEditDataset, SIGNAL(textEdited(QString const&)), this,
	        SLOT(lineEdited(QString const&)));
	connect(&lineEditDataset, SIGNAL(editingFinished()), this,
	        SLOT(fixLineEdit()));
	layoutH->addWidget(&lineEditDataset);
	layoutV->addWidget(w);

	updateInfoLabel();
	layoutV->addWidget(&infoLabel);

}

bool HDF5DatasetSelect::datasetPathIsValid()
{
	QString path(lineEditDataset.text());
	if(path == "")
		return false;
	QStringList p(path.split('/'));
	HDF5Object* result;
	if(p.size() <= 1)
		result = nullptr;
	else if(p.size() > 1 && p[0] == "")
	{
		p.pop_front();
		result = &hdf5_obj;
		if(p[0] != "")
			result = objFromPath(p, result);
	}
	if(result != nullptr && result->type == H5O_TYPE_DATASET)
		return true;
	return false;
}

void HDF5DatasetSelect::load(QString const& path)
{
	tree.clear();
	hdf5_obj = readHDF5RootObject(path.toStdString());
	constructItems(hdf5_obj);
	tree.setRootIsDecorated(true);
}

void HDF5DatasetSelect::lineEdited(QString const& path)
{
	if(path == "")
	{
		tree.clearSelection();
		return;
	}
	QStringList p(path.split('/'));
	QTreeWidgetItem* result;
	if(p.size() <= 1)
		result = nullptr;
	else if(p.size() > 1 && p[0] == "")
	{
		p.pop_front();
		result = tree.topLevelItem(0);
		if(p[0] != "")
			result = itemFromPath(p, result);
	}
	if(result != nullptr)
		tree.setCurrentItem(result);
}

void HDF5DatasetSelect::fixLineEdit()
{
	if(tree.selectedItems().size() == 0)
		lineEditDataset.setText("");
	else
		lineEditDataset.setText(pathFromItem(tree.selectedItems()[0]));
	updateInfoLabel();

	emit selectedObjChanged();
}

QTreeWidgetItem* HDF5DatasetSelect::constructItems(HDF5Object const& obj,
                                                   QTreeWidgetItem* parent)
{
	QTreeWidgetItem* item;
	if(parent == nullptr)
		item = new QTreeWidgetItem(&tree);
	else
		item = new QTreeWidgetItem(parent);

	item->setText(0, QString::fromStdString(obj.name));

	for(HDF5Object o : obj.links)
		item->addChild(constructItems(o, item));

	return item;
}

void HDF5DatasetSelect::updateInfoLabel()
{
	if(datasetPathIsValid())
	{
		infoLabel.setStyleSheet("QLabel { color : green; }");
		infoLabel.setText(tr("Dataset selected"));
	}
	else
	{
		infoLabel.setStyleSheet("QLabel { color : red; }");
		if(tree.selectedItems().size() == 0)
			infoLabel.setText(tr("No object selected"));
		else
			infoLabel.setText(tr("Selected object is not a dataset"));
	}
}

QString HDF5DatasetSelect::pathFromItem(QTreeWidgetItem* item)
{
	if(item->parent())
	{
		QString parentPath(pathFromItem(item->parent()));
		if(parentPath == "/")
			return pathFromItem(item->parent()) + item->text(0);
		else
			return pathFromItem(item->parent()) + '/' + item->text(0);
	}
	else
		return item->text(0);
}

QTreeWidgetItem* HDF5DatasetSelect::itemFromPath(QStringList& path,
                                                 QTreeWidgetItem* rootItem)
{
	if(path.size() == 0)
		return rootItem;
	for(int i(0); i < rootItem->childCount(); ++i)
	{
		QTreeWidgetItem* child = rootItem->child(i);
		if(!child)
			continue;
		if(child->text(0) == path[0])
		{
			if(path.size() == 1)
				return child;
			else
			{
				path.pop_front();
				return itemFromPath(path, child);
			}
		}
	}

	return nullptr;
}

HDF5Object* HDF5DatasetSelect::objFromPath(QStringList& path,
                                           HDF5Object* rootObj)
{
	if(path.size() == 0)
		return rootObj;
	for(unsigned int i(0); i < rootObj->links.size(); ++i)
	{
		HDF5Object* obj = &(rootObj->links[i]);
		if(obj->name == path[0].toStdString())
		{
			if(path.size() == 1)
				return obj;
			else
			{
				path.pop_front();
				return objFromPath(path, obj);
			}
		}
	}

	return nullptr;
}
