/*****************************************************************************
 *
 * mainWindow.h -- class declaration
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

#include "config.h"
#include "displayWindow.h"
#include "histogramWidget.h"

extern "C" {
#include "oacam.h"
} 


class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
			MainWindow();
			~MainWindow();
    void		setCapturedFrames ( unsigned int );
    void		setActualFrameRate ( unsigned int );
    void		setTemperature ( float );
    void		clearTemperature ( void );
    void		showFPSMaxValue ( int );
    void		clearFPSMaxValue ( void );
    void		showStatusMessage ( QString );
    void		setNightStyleSheet ( QWidget* );
    void		clearNightStyleSheet ( QWidget* );
    void		setProgress ( int );

  private:
    QMenu*		fileMenu;
    QMenu*		cameraMenu;
    QMenu*		optionsMenu;
    QMenu*		settingsMenu;
    QMenu*		helpMenu;
    DisplayWindow*	displayWindow;
    QAction*		disconnect;
    QAction*		rescan;
    QAction*		cameraMenuSeparator;
    QProgressBar*	progressBar;
    QLabel*		capturedValue;
    QLabel*		fpsMaxValue;
    QLabel*		fpsActualValue;
    QStatusBar*		statusLine;
    QLabel*		tempValue;

    void		readConfig ( void );
    void		writeConfig ( void );
    void		createStatusBar ( void );
    void		createMenus ( void );
    void		doDisconnect ( void );

    const char*  	devTypes[8];
    QAction*		cameras[ OA_MAX_DEVICES ];
    QString		menuEntry[ OA_MAX_DEVICES ];
    oaDevice*		devs[ OA_MAX_DEVICES ];

    QLabel*		tempLabel;
    QLabel*		fpsMaxLabel;
    QLabel*		fpsActualLabel;
    QLabel*		capturedLabel;
    QAction*		loadConfig;
    QAction*		saveConfig;
    QAction*		saveConfigAs;
    QAction*		exit;
    QSignalMapper*	signalMapper;
    QAction*		histogramOpt;
    QAction*		ephems;
    QAction*		autoalign;
    QAction*		alignbox;
    QAction*		autoguide;
    QAction*		reticle;
    QAction*		cutout;
    QAction*		focusaid;
    QAction*		darkframe;
    QAction*		derotate;
    QAction*		flipX;
    QAction*		flipY;
    QAction*		debayerOpt;
    QAction*		general;
    QAction*		capture;
    QAction*		profiles;
    QAction*		filters;
    QAction*		debayer;
    QAction*		autorun;
    QAction*		histogram;
    QAction*		about;
    int			oldHistogramState;
    int			connectedCameras;
    int			cameraMenuCreated;
    int			doingQuit;
    void		createSettingsWidget ( void );

  public slots:
    void		connectCamera( int );
    void		disconnectCamera ( void );
    void		rescanCameras ( void );
    void		quit ( void );
    void		setNightMode ( int );
    void		enableHistogram ( void );
    void		histogramClosed ( void );
    void		enableReticle ( void );
    void		enableFlipX ( void );
    void		enableFlipY ( void );
    void		aboutDialog ( void );
    void		doGeneralSettings ( void );
    void		doCaptureSettings ( void );
    void		doProfileSettings ( void );
    void		doFilterSettings ( void );
    void		doAutorunSettings ( void );
    void		doHistogramSettings ( void );
    void		doCameraMenu ( void );
    void		closeSettingsWindow ( void );
    void		settingsClosed ( void );
};
