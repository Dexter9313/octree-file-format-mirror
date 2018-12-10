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

#include "MainWindow.hpp"

MainWindow::MainWindow()
    : QMainWindow()
    , menubar(this)
    , centralWidget(this)
    , generateButton(tr("Generate octrees"), this)
    , gazSelect(this, tr("Select gaz particles positions :"), ".gaz")
    , starsSelect(this, tr("Select stars particles positions :"), ".stars")
    , darkmatterSelect(this, tr("Select dark matter particles positions :"),
                       ".dm")
{
	QMenu* menu = new QMenu("File", &menubar);
	connect(menu->addAction(tr("&Open HDF5 File...")), SIGNAL(triggered()),
	        this, SLOT(openHDF5()));
	connect(menu->addAction(tr("&Quit")), SIGNAL(triggered()), this,
	        SLOT(close()));
	menubar.addMenu(menu);

	menu = new QMenu("About", &menubar);
	menubar.addMenu(menu);

	setMenuBar(&menubar);

	QVBoxLayout* mainLayout = new QVBoxLayout(&centralWidget);
	QWidget* w0             = new QWidget(this);

	QHBoxLayout* layout = new QHBoxLayout(w0);

	layout->addWidget(&gazSelect);
	layout->addWidget(&starsSelect);
	layout->addWidget(&darkmatterSelect);

	mainLayout->addWidget(w0);

	QWidget* bottom      = new QWidget(this);
	QHBoxLayout* bottomL = new QHBoxLayout(bottom);

	connect(&gazSelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));
	connect(&starsSelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));
	connect(&darkmatterSelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));

	connect(&generateButton, SIGNAL(clicked(bool)), this, SLOT(generate()));
	generateButton.setEnabled(false);
	bottomL->addWidget(&generateButton);

	mainLayout->addWidget(bottom);

	setCentralWidget(&centralWidget);
	centralWidget.setEnabled(false);
}

void MainWindow::openHDF5()
{
	fileName = QFileDialog::getOpenFileName(
	    this, tr("Open HDF5 File"), QString(""), tr("HDF5 Files (*.hdf5)"));
	std::cout << fileName.toStdString() << std::endl;
	gazSelect.load(fileName);
	starsSelect.load(fileName);
	darkmatterSelect.load(fileName);
	centralWidget.setEnabled(true);
}

void MainWindow::updateGenerateButton()
{
	if(gazSelect.datasetPathIsValid() && starsSelect.datasetPathIsValid()
	   && darkmatterSelect.datasetPathIsValid())
		generateButton.setEnabled(true);
	else
		generateButton.setEnabled(false);
}

void MainWindow::generate(int previousExitCode)
{
	QString baseCmdLine(QString("octreegen ") + fileName + ":");

	if(proc == nullptr)
	{
		selectsToExecute = {&gazSelect, &starsSelect, &darkmatterSelect};

		proc = new QProcess(this);
		connect(proc, SIGNAL(readyReadStandardOutput()), this,
		        SLOT(processOutput()));
		connect(proc, SIGNAL(readyReadStandardError()), this,
		        SLOT(processOutput()));
		connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this,
		        SLOT(generate(int)));

		progress = new QProgressDialog(tr("Generating trees..."), QString(), 0,
		                               selectsToExecute.size(), this);
		progress->setMinimumDuration(0);
		progress->setValue(0);

		procOutput = "";
		procError  = "";
		QString cmdLine(baseCmdLine + selectsToExecute[0]->getDatasetPath()
		                + " " + selectsToExecute[0]->getOutputPath());
		proc->start(cmdLine);
		if(!proc->waitForStarted())
		{
			QMessageBox::critical(this, tr("Critical Error"),
			                      tr("Could not start octreegen. Please make "
			                         "sure it is installed."));
			proc->waitForFinished();
			delete proc;
			delete progress;
			proc = nullptr;
			return;
		}
	}
	else
	{
		std::cout << procOutput.toStdString() << std::endl;
		std::cout << procError.toStdString() << std::endl;
		if(previousExitCode != 0)
		{
			delete proc;
			delete progress;
			proc = nullptr;
			dialogWithConsoleOutput(
			    tr("Warning"),
			    tr("Something went wrong during the octree generation..."),
			    baseCmdLine + selectsToExecute[0]->getDatasetPath() + " "
			        + selectsToExecute[0]->getOutputPath());
			return;
		}
		progress->setValue(progress->value() + 1);

		selectsToExecute.erase(selectsToExecute.begin());
		if(selectsToExecute.size() == 0)
		{
			delete proc;
			delete progress;
			proc = nullptr;
			QMessageBox::information(this, tr("Information"), tr("Success !"));
			return;
		}
		procOutput = "";
		procError  = "";
		QString cmdLine(baseCmdLine + selectsToExecute[0]->getDatasetPath()
		                + " " + selectsToExecute[0]->getOutputPath());
		proc->start(cmdLine);
		if(!proc->waitForStarted())
		{
			delete proc;
			delete progress;
			proc = nullptr;
			QMessageBox::critical(this, tr("Critical Error"),
			                      tr("Could not start octreegen. Please make "
			                         "sure it is installed."));
			return;
		}
	}
}

void MainWindow::processOutput()
{
	procOutput += proc->readAllStandardOutput();
	procError += proc->readAllStandardError();
}

void MainWindow::dialogWithConsoleOutput(QString const& title,
                                         QString const& text,
                                         QString const& launchedCmdLine)
{
	QMessageBox box(this);
	box.setModal(true);
	box.setWindowTitle(title);
	box.setIcon(QMessageBox::Warning);
	box.setText(text);
	box.setDetailedText("Command :\n" + launchedCmdLine + "\n\nOutput :\n"
	                    + procError);
	box.setStyleSheet("QTextEdit {background-color:black;color:white;} ");
	box.exec();
}
