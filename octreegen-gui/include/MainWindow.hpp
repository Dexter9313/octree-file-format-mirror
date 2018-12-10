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

#include <QFileDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
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
	void generate(int previousExitCode = 0);
	void processOutput();

  private:
	QMenuBar menubar;
	QWidget centralWidget;
	QPushButton generateButton;

	// related to octreegen calls
	QProcess* proc = nullptr;
	QString procOutput;
	QString procError;
	QProgressDialog* progress;
	std::vector<HDF5DatasetSelect*> selectsToExecute;

	QString fileName;

	HDF5DatasetSelect gazSelect;
	HDF5DatasetSelect starsSelect;
	HDF5DatasetSelect darkmatterSelect;

	void dialogWithConsoleOutput(QString const& title, QString const& text,
	                             QString const& launchedCmdLine);
};

#endif // MAINWINDOW_H
