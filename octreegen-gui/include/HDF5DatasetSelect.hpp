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
	HDF5DatasetSelect(QWidget* parent, QString const& particlesLabel, QString const& preExtension);
	bool datasetPathIsValid();
	QString getDatasetPath() const { return lineEditDataset.text(); };
	QString getOutputPath() const { return lineEditSaveAs.text(); }

  public slots:
	void load(QString const& path);
	void lineEdited(QString const& path);
	void fixLineEdit();
	void selectSaveAs();

  signals:
	void selectedObjChanged();

  private:
	QTreeWidget tree;
	QLineEdit lineEditDataset;
	QLabel infoLabel;
	QLineEdit lineEditSaveAs;
	QString preExtension;

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
