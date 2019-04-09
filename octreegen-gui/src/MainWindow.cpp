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
    , coordinatesSelect(this, tr("Select particles coordinates :"))
    , radiusSelect(this, tr("Select particles radius (optional) :"))
    , luminositySelect(this, tr("Select particles luminosity (optional) :"))
{
	QMenu* menu = new QMenu("File", &menubar);
	connect(menu->addAction(tr("&Open HDF5 File...")), SIGNAL(triggered()),
	        this, SLOT(openHDF5()));
	connect(menu->addAction(tr("&Quit")), SIGNAL(triggered()), this,
	        SLOT(close()));
	menubar.addMenu(menu);

	menu = new QMenu("About", &menubar);
	connect(menu->addAction(tr("&About octreegen-gui")), SIGNAL(triggered()),
	        this, SLOT(aboutOctreegenGUI()));
	connect(menu->addAction(tr("A&bout Qt")), SIGNAL(triggered()), this,
	        SLOT(aboutQt()));
	menubar.addMenu(menu);

	setMenuBar(&menubar);

	QVBoxLayout* mainLayout = new QVBoxLayout(&centralWidget);
	QWidget* w0             = new QWidget(this);

	QHBoxLayout* layout = new QHBoxLayout(w0);

	layout->addWidget(&coordinatesSelect);
	layout->addWidget(&radiusSelect);
	layout->addWidget(&luminositySelect);

	mainLayout->addWidget(w0);

	QWidget* bottom      = new QWidget(this);
	QHBoxLayout* bottomL = new QHBoxLayout(bottom);

	connect(&coordinatesSelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));
	connect(&radiusSelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));
	connect(&luminositySelect, SIGNAL(selectedObjChanged()), this,
	        SLOT(updateGenerateButton()));

	QWidget* w2           = new QWidget(this);
	QHBoxLayout* layoutH2 = new QHBoxLayout(w2);
	layoutH2->addWidget(new QLabel(tr("Save as :"), this));
	layoutH2->addWidget(&lineEditSaveAs);
	QPushButton* b = new QPushButton("...", this);
	connect(b, SIGNAL(clicked(bool)), this, SLOT(selectSaveAs()));
	layoutH2->addWidget(b);
	mainLayout->addWidget(w2);

	connect(&generateButton, SIGNAL(clicked(bool)), this, SLOT(generate()));
	generateButton.setEnabled(false);
	bottomL->addWidget(&generateButton);

	mainLayout->addWidget(bottom);

	setCentralWidget(&centralWidget);
	centralWidget.setEnabled(false);
}

void MainWindow::openHDF5()
{
	fileNames = QFileDialog::getOpenFileNames(
	    this, tr("Open HDF5 File(s)"), QString(""), tr("HDF5 Files (*.hdf5)"));
	if(fileNames.empty())
		return;
	coordinatesSelect.load(fileNames);
	radiusSelect.load(fileNames);
	luminositySelect.load(fileNames);
	centralWidget.setEnabled(true);

	QFileInfo fi(fileNames[0]);
	lineEditSaveAs.setText(fi.dir().absolutePath() + QDir::separator()
	                       + fi.baseName() + ".octree");
}

void MainWindow::updateGenerateButton()
{
	if(coordinatesSelect.datasetPathIsValid())
	{
		generateButton.setEnabled(true);

		QString datasetPath(coordinatesSelect.getDatasetPath());
		QStringList splittedPath = datasetPath.split('/');
		splittedPath.pop_back();

		QFileInfo fi(fileNames[0]);
		lineEditSaveAs.setText(fi.dir().absolutePath() + QDir::separator()
		                       + fi.baseName() + "."
		                       + splittedPath.last() + ".octree");
	}
	else
		generateButton.setEnabled(false);
}

void MainWindow::generate()
{
	this->setEnabled(false);
	QString cmdLine("octreegen ");
	cmdLine += '"';
	for(auto fileName : fileNames)
	{
		cmdLine += fileName + ' ';
	}
	cmdLine.remove(cmdLine.size() - 1, 1);
	cmdLine +=  "\":" + coordinatesSelect.getDatasetPath();

	if(radiusSelect.datasetPathIsValid()
	   || luminositySelect.datasetPathIsValid())
		cmdLine += ":";
	if(radiusSelect.datasetPathIsValid())
		cmdLine += radiusSelect.getDatasetPath();
	if(luminositySelect.datasetPathIsValid())
		cmdLine += ":" + luminositySelect.getDatasetPath();
	cmdLine += " " + lineEditSaveAs.text();

	if(proc == nullptr)
	{
		proc = new QProcess(this);
		proc->start(cmdLine);
		if(!proc->waitForStarted())
		{
			QMessageBox::critical(this, tr("Critical Error"),
			                      tr("Could not start octreegen. Please make "
			                         "sure it is installed."));
			proc->waitForFinished();
			delete proc;
			proc = nullptr;
			return;
		}

		QString detailed("Command :\n" + cmdLine + "\n\nOutput :\n");

		QTextEdit te(nullptr);
		te.setReadOnly(true);
		te.setFont(QFont("Monospace", 10));
		te.setFixedSize(QSize(800, 400));
		te.setStyleSheet("QTextEdit {background-color:black;color:white;} ");
		te.ensureCursorVisible();
		te.show();
		while(!proc->waitForFinished(100))
		{
			QCoreApplication::processEvents();
			detailed += proc->readAll().toStdString().c_str();
			processCarriageReturns(detailed);
			te.setText(detailed);
			te.verticalScrollBar()->setValue(te.verticalScrollBar()->maximum());
		}
		detailed += proc->readAll().toStdString().c_str();
		processCarriageReturns(detailed);
		te.setText(detailed);
		te.verticalScrollBar()->setValue(te.verticalScrollBar()->maximum());
		if(proc->exitCode() != 0)
		{
			delete proc;
			proc = nullptr;
			QMessageBox::warning(
			    this, tr("Warning"),
			    tr("Something went wrong during the octree generation..."));
			return;
		}

		delete proc;
		proc = nullptr;
		QMessageBox::information(this, tr("Information"), tr("Success !"));
	}
	this->setEnabled(true);
}

void MainWindow::aboutOctreegenGUI()
{
	QMessageBox::about(
	    this, tr("About octreegen-gui"),
	    tr("Copyright (C) 2018 Florian Cabot <florian.cabot@epfl.ch>\n"
	       "\n"
	       "This program is free software; you can redistribute it and/or"
	       "modify\n"
	       "it under the terms of the GNU General Public License as published "
	       "by\n"
	       "the Free Software Foundation; either version 3 of the License, or"
	       "(at your option) any later version.\n"
	       "\n"
	       "This program is distributed in the hope that it will be useful,\n"
	       "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
	       "GNU General Public License for more details.\n"
	       "\n"
	       "You should have received a copy of the GNU General Public License "
	       "along\n"
	       "with this program; if not, write to the Free Software Foundation, "
	       "Inc.,\n"
	       "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA."));
}

void MainWindow::aboutQt()
{
	QMessageBox::aboutQt(this);
}

void MainWindow::selectSaveAs()
{
	QString result = QFileDialog::getSaveFileName(
	    this, tr("Save as"), lineEditSaveAs.text(),
	    tr("Octree files (*.octree)"));
	if(result != "")
		lineEditSaveAs.setText(result);
}

void MainWindow::processCarriageReturns(QString& str)
{
	int cr;
	while((cr = str.indexOf('\r')) >= 0)
	{
		int lineBreak = str.lastIndexOf('\n', cr);
		// + 3 := remove "\r\033[K" after '\r'
		str.remove(lineBreak + 1, cr - lineBreak + 3);
	}
}
