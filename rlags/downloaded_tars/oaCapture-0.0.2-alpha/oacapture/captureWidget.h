/*****************************************************************************
 *
 * captureWidget.h -- class declaration
 *
 * Copyright 2013 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#pragma once

#include <QtGui>

#include "outputHandler.h"

#define	CAPTURE_AVI	1
#define	CAPTURE_SER	2
#define	CAPTURE_FITS	3

class CaptureWidget : public QGroupBox
{
  Q_OBJECT

  public:
    unsigned long	recordingStartTime;
    unsigned long	recordingEndTime;

    			CaptureWidget ( QWidget* parent = 0 );
    			~CaptureWidget();
    void		enableStartButton ( int );
    void		closeOutputHandler ( void );
    OutputHandler*	getOutputHandler ( void );
    void		enableSERCapture ( int );
    int			singleAutorunFinished ( void );
    void		startNewAutorun ( void );
    void		doStopRecording ( void );
    void		enableAutorun ( void );
    QString		getCurrentFilterName ( void );
    QString		getCurrentProfileName ( void );
    void		reloadFilters ( void );
    void		reloadProfiles ( void );
    void		updateFromConfig ( void );

  private:
    void		doStartRecording ( int );
    void		writeSettings ( OutputHandler* );
    QLabel*		secondsLabel;
    QLineEdit*		seconds;
    QPushButton*	startButton;
    QPushButton*	stopButton;
    QPushButton*	pauseButton;
    QPushButton*	autorunButton;
    QLabel*		autorunLabel;
    QVBoxLayout*	box;
    QHBoxLayout*	profile;
    QHBoxLayout*	file;
    QHBoxLayout*	type;
    QHBoxLayout*	controls;
    QLabel*		profileLabel;
    QComboBox*		profileMenu;
    QPushButton*	restoreButton;
    QLabel*		filterLabel;
    QComboBox*		filterMenu;
    QLabel*		fileLabel;
    QLineEdit*		fileName;
    QRegExpValidator*	fileNameValidator;
    QPushButton*	newFolderButton;
    QPushButton*	deleteButton;
    QPushButton*	openFolderButton;
    QLabel*		typeLabel;
    QComboBox*		typeMenu;
    QCheckBox*		limitCheckbox;
    QPushButton*	fileListButton;
    OutputHandler*	outputHandler;
    QIntValidator*	secondsValidator;

  public slots:
    void		showLimitInputBox ( int );
    void		changeLimitSeconds ( const QString& );
    void		fileTypeChanged ( int );
    void		startRecording ( void );
    void		stopRecording ( void );
    void		setNewCaptureDirectory ( void );
    void		openCaptureDirectory ( void );
    void		resetAutorun ( void );
    void		deleteLastRecordedFile ( void );
    void		updateFileNameTemplate ( void );
    void		filterTypeChanged ( int );
    void		profileTypeChanged ( int );
    void		updateSettingsFromProfile ( void );
};
