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

	//related to octreegen calls
	QProcess* proc = nullptr;
	QString procOutput;
	QString procError;
	QProgressDialog* progress;
	std::vector<HDF5DatasetSelect*> selectsToExecute;

	QString fileName;

	HDF5DatasetSelect gazSelect;
	HDF5DatasetSelect starsSelect;
	HDF5DatasetSelect darkmatterSelect;

	void dialogWithConsoleOutput(QString const& title, QString const& text, QString const& launchedCmdLine);
};

#endif // MAINWINDOW_H
