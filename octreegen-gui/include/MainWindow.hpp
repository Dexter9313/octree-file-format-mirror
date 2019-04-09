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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <streambuf>
#include <sstream>

#include <QCoreApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QVBoxLayout>

#include <iostream>

#include "HDF5DatasetSelect.hpp"

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	MainWindow();

  public slots:
	void openHDF5();
	void updateGenerateButton();
	void generate();
	void aboutOctreegenGUI();
	void aboutQt();
	void selectSaveAs();

  private:
	QMenuBar menubar;
	QWidget centralWidget;
	QPushButton generateButton;
	QLineEdit lineEditSaveAs;

	// related to octreegen calls
	QProcess* proc = nullptr;

	QStringList fileNames;

	HDF5DatasetSelect coordinatesSelect;
	HDF5DatasetSelect radiusSelect;
	HDF5DatasetSelect luminositySelect;

	static void processCarriageReturns(QString& str);
};

#endif // MAINWINDOW_H
