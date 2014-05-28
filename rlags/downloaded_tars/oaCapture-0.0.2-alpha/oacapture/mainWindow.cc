/*****************************************************************************
 *
 * mainWindow.cc -- the main controlling window class
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

#include <QtGui>

#include <unistd.h>

extern "C" {
#include "oacam.h"
}

#include "version.h"
#include "config.h"
#include "displayWindow.h"
#include "mainWindow.h"
#include "state.h"

CONFIG		config;
STATE		state;


MainWindow::MainWindow()
{
  signalMapper = 0;
  rescan = disconnect = 0;
  connectedCameras = cameraMenuCreated = 0;
  doingQuit = 0;

  readConfig();
  createStatusBar();
  createMenus();
  setWindowTitle( APPLICATION_NAME " " VERSION_STR );

  state.mainWindow = this;
  state.controlWidget = 0;
  state.libavStarted = 0;
  state.camera = new Camera;
  state.histogramOn = 0;
  oldHistogramState = -1;
  state.histogramWidget = 0;
  state.lastRecordedFile = "";

  displayWindow = new DisplayWindow ( this );
  setCentralWidget ( displayWindow );

  char d[ PATH_MAX ];
  state.currentDirectory = QString::fromAscii ( getcwd ( d, PATH_MAX ));
}


MainWindow::~MainWindow()
{
  // FIX ME -- delete cameras[]

  state.histogramOn = 0;

  delete about;
  delete histogram;
  delete autorun;
  delete debayer;
  delete filters;
  delete profiles;
  delete capture;
  delete general;
  delete debayerOpt;
  delete flipY;
  delete flipX;
  delete darkframe;
  delete focusaid;
  delete cutout;
  delete reticle;
  delete autoguide;
  delete alignbox;
  delete autoalign;
  delete histogramOpt;
  if ( signalMapper ) {
    delete signalMapper;
  }
  delete exit;
  if ( rescan ) {
    delete rescan;
  }
  if ( disconnect ) {
    delete disconnect;
  }
  // delete saveConfigAs;
  delete saveConfig;
  delete loadConfig;
  delete capturedValue;
  delete fpsActualValue;
  delete fpsMaxValue;
  delete progressBar;
  delete capturedLabel;
  delete fpsActualLabel;
  delete fpsMaxLabel;
  delete displayWindow;
  if ( state.camera ) {
    delete state.camera;
  }
  if ( state.histogramWidget ) {
    delete state.histogramWidget;
  }
}


void
MainWindow::readConfig ( void )
{
  QSettings settings ( ORGANISATION_NAME_SETTINGS, APPLICATION_NAME );

  // -1 means we don't have a config file.  We change it to 1 later in the
  // function
  config.saveSettings = settings.value ( "saveSettings", -1 ).toInt();

  if ( !config.saveSettings ) {
    config.showHistogram = 0;
    config.autoAlign = 0;
    config.showReticle = 0;
    config.cutout = 0;
    config.focusAid = 0;
    config.darkFrame = 0;
    config.flipX = 0;
    config.flipY = 0;
    config.debayer = 0;

    config.sixteenBit = 0;
    config.binning2x2 = 0;

    config.useROI = 0;
    config.imageSizeX = 0;
    config.imageSizeY = 0;

    config.zoomButton1Option = 1;
    config.zoomButton2Option = 3;
    config.zoomButton3Option = 5;
    config.zoomValue = 100;

    config.gainValue = 50;
    config.exposureValue = 10;
    config.exposureAbsoluteValue = 10;
    config.gammaValue = -1;
    config.brightnessValue = -1;
    config.exposureMenuOption = 3;
    config.frameRateNumerator = 0;
    config.frameRateDenominator = 1;

    config.profileOption = 0;
    config.filterOption = 0;
    config.fileTypeOption = 1;
    config.limitEnabled = 0;
    config.limitValue = -1;
    config.fileNameTemplate = QString ( "oaCapture-%DATE-%TIME" );
    config.captureDirectory = QString ( "" );

    config.autorunCount = 0;
    config.autorunDelay = 0;

    config.preview = 1;
    config.nightMode = 0;
    config.displayFPS = 0;

    config.splitHistogram = 0;

    config.numProfiles = 0;
    config.numFilters = 0;

  } else {

    int version = settings.value ( "configVersion", 1 ).toInt();
    Q_UNUSED ( version )

    restoreGeometry ( settings.value ( "geometry").toByteArray());

    // FIX ME -- how to handle this?
    // config.cameraDevice = settings.value ( "device/camera", -1 ).toInt();

    config.showHistogram = settings.value ( "options/showHistogram",
        0 ).toInt();
    config.autoAlign = settings.value ( "options/autoAlign", 0 ).toInt();
    config.showReticle = settings.value ( "options/showReticle", 0 ).toInt();
    config.cutout = settings.value ( "options/cutout", 0 ).toInt();
    config.focusAid = settings.value ( "options/focusAid", 0 ).toInt();
    config.darkFrame = settings.value ( "options/darkFrame", 0 ).toInt();
    config.flipX = settings.value ( "options/flipX", 0 ).toInt();
    config.flipY = settings.value ( "options/flipY", 0 ).toInt();
    config.debayer = settings.value ( "options/debayer", 0 ).toInt();

    config.sixteenBit = settings.value ( "camera/sixteenBit", 0 ).toInt();
    config.binning2x2 = settings.value ( "camera/binning2x2", 0 ).toInt();

    config.useROI = settings.value ( "image/useROI", 0 ).toInt();
    config.imageSizeX = settings.value ( "image/imageSizeX", 0 ).toInt();
    config.imageSizeY = settings.value ( "image/imageSizeY", 0 ).toInt();

    config.zoomButton1Option = settings.value ( "image/zoomButton1Option",
        1 ).toInt();
    config.zoomButton2Option = settings.value ( "image/zoomButton2Option",
        3 ).toInt();
    config.zoomButton3Option = settings.value ( "image/zoomButton3Option",
        5 ).toInt();
    config.zoomValue = settings.value ( "image/zoomValue", 100 ).toInt();

    config.gainValue = settings.value ( "control/gainValue", 50 ).toInt();
    config.exposureValue = settings.value ( "control/exposureValue",
        10 ).toInt();
    config.exposureAbsoluteValue = settings.value (
        "control/exposureAbsoluteValue", 10 ).toInt();
    config.gammaValue = settings.value ( "control/gammaValue", -1 ).toInt();
    config.brightnessValue = settings.value ( "control/brightnessValue",
        -1 ).toInt();
    config.exposureMenuOption = settings.value ( "control/exposureMenuOption",
        3 ).toInt();
    config.frameRateNumerator = settings.value ( "control/frameRateNumerator",
        0 ).toInt();
    config.frameRateDenominator = settings.value (
        "control/frameRateDenominator", 1 ).toInt();

    config.profileOption = settings.value ( "control/profileOption",
        0 ).toInt();
    config.filterOption = settings.value ( "control/filterOption", 0 ).toInt();
    config.fileTypeOption = settings.value ( "control/fileTypeOption",
        1 ).toInt();
    config.limitEnabled = settings.value ( "control/limitEnabled", 0 ).toInt();
    config.limitValue = settings.value ( "control/limitValue", -1 ).toInt();
    config.fileNameTemplate = settings.value ( "control/fileNameTemplate",
        "oaCapture-%DATE-%TIME" ).toString();
    config.captureDirectory = settings.value ( "control/captureDirectory",
        "" ).toString();

    config.autorunCount = settings.value ( "autorun/count", 0 ).toInt();
    config.autorunDelay = settings.value ( "autorun/delay", 0 ).toInt();

    config.preview = settings.value ( "display/preview", 1 ).toInt();
    config.nightMode = settings.value ( "display/nightMode", 0 ).toInt();
    config.displayFPS = settings.value ( "display/displayFPS", 0 ).toInt();

    config.splitHistogram = settings.value ( "histogram/split", 0 ).toInt();

    config.numProfiles = settings.beginReadArray ( "profiles" );
    if ( config.numProfiles ) {
      for ( int i = 0; i < config.numProfiles; i++ ) {
        settings.setArrayIndex ( i );
        PROFILE p;
        p.profileName = settings.value ( "name", "" ).toString();
        p.sixteenBit = settings.value ( "sixteenBit", 0 ).toInt();
        p.binning2x2 = settings.value ( "binning2x2", 0 ).toInt();
        p.useROI = settings.value ( "useROI", 0 ).toInt();
        p.imageSizeX = settings.value ( "imageSizeX", 0 ).toInt();
        p.imageSizeY = settings.value ( "imageSizeY", 0 ).toInt();
        p.gainValue = settings.value ( "gainValue", 50 ).toInt();
        p.exposureValue = settings.value ( "exposureValue", 10 ).toInt();
        p.exposureAbsoluteValue = settings.value ( "exposureAbsoluteValue",
            10 ).toInt();
        p.gammaValue = settings.value ( "gammaValue", -1 ).toInt();
        p.brightnessValue = settings.value ( "brightnessValue", -1 ).toInt();
        p.frameRateNumerator = settings.value ( "frameRateNumerator",
            0 ).toInt();
        p.frameRateDenominator = settings.value ( "frameRateDenominator",
            1 ).toInt();
        p.filterOption = settings.value ( "filterOption", 0 ).toInt();
        p.fileTypeOption = settings.value ( "fileTypeOption", 1 ).toInt();
        p.fileNameTemplate = settings.value ( "fileNameTemplate",
            "oaCapture-%DATE-%TIME" ).toString();
        p.limitEnabled = settings.value ( "limitEnabled", 0 ).toInt();
        p.limitValue = settings.value ( "limitValue", -1 ).toInt();
        config.profiles.append ( p );
      }
    }
    settings.endArray();

    config.numFilters = settings.beginReadArray ( "filters" );
    if ( config.numFilters ) {
      for ( int i = 0; i < config.numFilters; i++ ) {
        settings.setArrayIndex ( i );
        FILTER f;
        f.filterName = settings.value ( "name", "" ).toString();
        config.filters.append ( f );
      }
    }
    settings.endArray();
  }

  if ( !config.saveSettings || config.saveSettings == -1 ) {
    config.saveSettings = -config.saveSettings;
    // FIX ME -- these should probably be configured elsewhere
    QList<QString> defaults;
    defaults << "L" << "R" << "G" << "B" << "IR" << "UV" << "Ha" << "Hb" <<
        "S2" << "O3" << "CH4";
    config.numFilters = defaults.count();
    for ( int i = 0; i < config.numFilters; i++ ) {
      FILTER f;
      f.filterName = defaults[i];
      config.filters.append ( f );
    }
  }
}


void
MainWindow::writeConfig ( void )
{
  if ( !config.saveSettings ) {
    return;
  }

  QSettings settings ( ORGANISATION_NAME_SETTINGS, APPLICATION_NAME );

  settings.setValue ( "saveSettings", config.saveSettings );

  settings.setValue ( "configVersion", 1 );
  settings.setValue ( "geometry", geometry());

  // FIX ME -- how to handle this?
  // settings.setValue ( "device/camera", -1 ).toInt();

  settings.setValue ( "options/showHistogram", config.showHistogram );
  settings.setValue ( "options/autoAlign", config.autoAlign );
  settings.setValue ( "options/showReticle", config.showReticle );
  settings.setValue ( "options/cutout", config.cutout );
  settings.setValue ( "options/focusAid", config.focusAid );
  settings.setValue ( "options/darkFrame", config.darkFrame );
  settings.setValue ( "options/flipX", config.flipX );
  settings.setValue ( "options/flipY", config.flipY );
  settings.setValue ( "options/debayer", config.debayer );

  settings.setValue ( "camera/sixteenBit", config.sixteenBit );
  settings.setValue ( "camera/binning2x2", config.binning2x2 );

  settings.setValue ( "image/useROI", config.useROI );
  settings.setValue ( "image/imageSizeX", config.imageSizeX );
  settings.setValue ( "image/imageSizeY", config.imageSizeY );

  settings.setValue ( "image/zoomButton1Option", config.zoomButton1Option );
  settings.setValue ( "image/zoomButton2Option", config.zoomButton2Option );
  settings.setValue ( "image/zoomButton3Option", config.zoomButton3Option );
  settings.setValue ( "image/zoomValue", config.zoomValue );

  settings.setValue ( "control/gainValue", config.gainValue );
  settings.setValue ( "control/exposureValue", config.exposureValue );
  settings.setValue ( "control/exposureAbsoluteValue",
      config.exposureAbsoluteValue );
  settings.setValue ( "control/gammaValue", config.gammaValue );
  settings.setValue ( "control/brightnessValue", config.brightnessValue );
  settings.setValue ( "control/exposureMenuOption", config.exposureMenuOption );
  settings.setValue ( "control/frameRateNumerator", config.frameRateNumerator );
  settings.setValue ( "control/frameRateDenominator",
      config.frameRateDenominator );

  settings.setValue ( "control/profileOption", config.profileOption );
  settings.setValue ( "control/filterOption", config.filterOption );
  settings.setValue ( "control/fileTypeOption", config.fileTypeOption );
  settings.setValue ( "control/limitEnabled", config.limitEnabled );
  settings.setValue ( "control/limitValue", config.limitValue );
  settings.setValue ( "control/fileNameTemplate", config.fileNameTemplate );
  settings.setValue ( "control/captureDirectory", config.captureDirectory );

  settings.setValue ( "autorun/count", config.autorunCount );
  settings.setValue ( "autorun/delay", config.autorunDelay );

  settings.setValue ( "display/preview", config.preview );
  settings.setValue ( "display/nightMode", config.nightMode );
  settings.setValue ( "display/displayFPS", config.displayFPS );

  settings.setValue ( "histogram/split", config.splitHistogram );

  settings.beginWriteArray ( "profiles" );
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "name", config.profiles[i].profileName );
      settings.setValue ( "sixteenBit", config.profiles[i].sixteenBit );
      settings.setValue ( "binning2x2", config.profiles[i].binning2x2 );
      settings.setValue ( "useROI", config.profiles[i].useROI );
      settings.setValue ( "imageSizeX", config.profiles[i].imageSizeX );
      settings.setValue ( "imageSizeY", config.profiles[i].imageSizeY );
      settings.setValue ( "gainValue", config.profiles[i].gainValue );
      settings.setValue ( "exposureValue", config.profiles[i].exposureValue );
      settings.setValue ( "exposureAbsoluteValue",
          config.profiles[i].exposureAbsoluteValue );
      settings.setValue ( "gammaValue", config.profiles[i].gammaValue );
      settings.setValue ( "brightnessValue",
          config.profiles[i].brightnessValue );
      settings.setValue ( "frameRateNumerator",
          config.profiles[i].frameRateNumerator );
      settings.setValue ( "frameRateDenominator",
          config.profiles[i].frameRateDenominator );
      settings.setValue ( "filterOption", config.profiles[i].filterOption );
      settings.setValue ( "fileTypeOption", config.profiles[i].fileTypeOption );
      settings.setValue ( "fileNameTemplate",
          config.profiles[i].fileNameTemplate );
      settings.setValue ( "limitEnabled", config.profiles[i].limitEnabled );
      settings.setValue ( "limitValue", config.profiles[i].limitValue );
    }
  }
  settings.endArray();

  settings.beginWriteArray ( "filters" );
  if ( config.numFilters ) {
    for ( int i = 0; i < config.numFilters; i++ ) {
      settings.setArrayIndex ( i );
      settings.setValue ( "name", config.filters[i].filterName );
    }
  }
  settings.endArray();

}


void
MainWindow::createStatusBar ( void )
{
  statusLine = statusBar();
  setStatusBar ( statusLine );

  tempLabel = new QLabel ( tr ( "Temp (C)" ));
  tempLabel->setFixedWidth ( 60 );
  fpsMaxLabel = new QLabel ( tr ( "FPS (max)" ));
  fpsMaxLabel->setFixedWidth ( 80 );
  fpsActualLabel = new QLabel ( tr ( "FPS (actual)" ));
  fpsActualLabel->setFixedWidth ( 80 );
  capturedLabel = new QLabel ( tr ( "Captured" ));
  capturedLabel->setFixedWidth ( 80 );
  progressBar = new QProgressBar;
  progressBar->setFixedWidth ( 200 );
  progressBar->setRange ( 0, 100 );
  progressBar->setTextVisible ( true );

  tempValue = new QLabel ( "" );
  tempValue->setFixedWidth ( 30 );
  fpsMaxValue = new QLabel ( "0" );
  fpsMaxValue->setFixedWidth ( 30 );
  fpsActualValue = new QLabel ( "0" );
  fpsActualValue->setFixedWidth ( 30 );
  capturedValue = new QLabel ( "0" );
  capturedValue->setFixedWidth ( 40 );

  statusLine->addPermanentWidget ( tempLabel );
  statusLine->addPermanentWidget ( tempValue );
  statusLine->addPermanentWidget ( fpsMaxLabel );
  statusLine->addPermanentWidget ( fpsMaxValue );
  statusLine->addPermanentWidget ( fpsActualLabel );
  statusLine->addPermanentWidget ( fpsActualValue );
  statusLine->addPermanentWidget ( capturedLabel );
  statusLine->addPermanentWidget ( capturedValue );
  statusLine->addPermanentWidget ( progressBar );
  
  statusLine->showMessage ( tr ( "started" ));
}


void
MainWindow::createMenus ( void )
{
  // FIX ME -- add "restore program defaults" option

  // File menu
  loadConfig = new QAction ( tr ( "Re&load Config" ), this );
  loadConfig->setStatusTip ( tr ( "Load default configuration" ));
  // FIX ME - set up slots

  saveConfig = new QAction ( tr ( "&Save Config" ), this );
  saveConfig->setShortcut ( QKeySequence::Save );
  saveConfig->setStatusTip ( tr ( "Save default configuration" ));
  // FIX ME - set up slots

  exit = new QAction ( tr ( "&Quit" ), this );
  exit->setShortcut ( QKeySequence::Quit );
  connect ( exit, SIGNAL( triggered()), this, SLOT( quit()));

  fileMenu = menuBar()->addMenu( tr ( "&File" ));
  fileMenu->addAction ( loadConfig );
  fileMenu->addAction ( saveConfig );
  fileMenu->addSeparator();
  fileMenu->addAction ( exit );

  // Camera device menu

  cameraMenu = menuBar()->addMenu ( tr ( "&Device" ));
  doCameraMenu();

  // Options menu

  histogramOpt = new QAction ( QIcon ( ":/icons/barchart.png" ),
      tr ( "Histogram" ), this );
  histogramOpt->setStatusTip ( tr ( "Open window for image histogram" ));
  histogramOpt->setCheckable ( true );
  connect ( histogramOpt, SIGNAL( changed()), this, SLOT( enableHistogram()));
  histogramOpt->setChecked ( config.showHistogram );

  autoalign = new QAction ( tr ( "Auto Align" ), this );
  autoalign->setCheckable ( true );
  // FIX ME - set up slots

  alignbox = new QAction ( tr ( "Align Box" ), this );
  alignbox->setCheckable ( true );
  // FIX ME - set up slots

  autoguide = new QAction ( tr ( "Auto Guide" ), this );
  autoguide->setCheckable ( true );
  // FIX ME - set up slots

  reticle = new QAction ( QIcon ( ":/icons/reticle.png" ),
      tr ( "Reticle" ), this );
  reticle->setStatusTip ( tr ( "Overlay a reticle on the preview image" ));
  reticle->setCheckable ( true );
  connect ( reticle, SIGNAL( changed()), this, SLOT( enableReticle()));
  reticle->setChecked ( config.showReticle );

  cutout = new QAction ( tr ( "Cut Out" ), this );
  cutout->setCheckable ( true );
  // FIX ME - set up slots

  focusaid = new QAction ( tr ( "Focus Aid" ), this );
  focusaid->setCheckable ( true );
  // FIX ME - set up slots

  darkframe = new QAction ( tr ( "Dark Frame" ), this );
  darkframe->setCheckable ( true );
  // FIX ME - set up slots

  flipX = new QAction ( QIcon ( ":/icons/object-flip-horizontal.png" ), 
      tr ( "Flip X" ), this );
  flipX->setStatusTip ( tr ( "Flip image left<->right" ));
  flipX->setCheckable ( true );
  flipX->setChecked ( config.flipX );
  connect ( flipX, SIGNAL( changed()), this, SLOT( enableFlipX()));

  flipY = new QAction ( QIcon ( ":/icons/object-flip-vertical.png" ),
      tr ( "Flip Y" ), this );
  flipY->setStatusTip ( tr ( "Flip image top<->bottom" ));
  flipY->setCheckable ( true );
  flipY->setChecked ( config.flipY );
  connect ( flipY, SIGNAL( changed()), this, SLOT( enableFlipY()));

  debayerOpt = new QAction ( tr ( "Debayer" ), this );
  debayerOpt->setCheckable ( true );
  // FIX ME - set up slots

  optionsMenu = menuBar()->addMenu ( tr ( "&Options" ));
  optionsMenu->addAction ( histogramOpt );
#ifdef ENABLE_AUTOALIGN
  optionsMenu->addAction ( autoalign );
#endif
#ifdef ENABLE_ALIGNBOX
  optionsMenu->addAction ( alignbox );
#endif
#ifdef ENABLE_AUTOGUIDE
  optionsMenu->addAction ( autoguide );
#endif
  optionsMenu->addAction ( reticle );
#ifdef ENABLE_CUTOUT
  optionsMenu->addAction ( cutout );
#endif
#ifdef ENABLE_FOCUSAID
  optionsMenu->addAction ( focusaid );
#endif
#ifdef ENABLE_DARKFRAME
  optionsMenu->addAction ( darkframe );
#endif
  optionsMenu->addAction ( flipX );
  optionsMenu->addAction ( flipY );
#ifdef ENABLE_DEBAYER
  optionsMenu->addAction ( debayerOpt );
#endif

  // settings menu

  general = new QAction ( QIcon ( ":/icons/cog.png" ),
      tr ( "General" ), this );
  general->setStatusTip ( tr ( "General configuration" ));
  connect ( general, SIGNAL( triggered()), this, SLOT( doGeneralSettings()));

  capture = new QAction ( tr ( "Capture" ), this );
  connect ( capture, SIGNAL( triggered()), this, SLOT( doCaptureSettings()));

  profiles = new QAction ( QIcon ( ":/icons/jupiter.png" ),
      tr ( "Profiles" ), this );
  profiles->setStatusTip ( tr ( "Edit saved profiles" ));
  connect ( profiles, SIGNAL( triggered()), this, SLOT( doProfileSettings()));

  filters = new QAction ( QIcon ( ":/icons/filter-wheel.png" ),
      tr ( "Filters" ), this );
  filters->setStatusTip ( tr ( "Configuration for filters" ));
  connect ( filters, SIGNAL( triggered()), this, SLOT( doFilterSettings()));

  debayer = new QAction ( tr ( "Debayer" ), this );
  // FIX ME - set up slots

  autorun = new QAction ( QIcon ( ":/icons/clicknrun.png" ),
      tr ( "Autorun" ), this );
  autorun->setStatusTip ( tr ( "Configuration for repeat captures" ));
  connect ( autorun, SIGNAL( triggered()), this, SLOT( doAutorunSettings()));

  histogram = new QAction ( QIcon ( ":/icons/barchart.png" ),
      tr ( "Histogram" ), this );
  histogram->setStatusTip ( tr ( "Configuration for histogram" ));
  connect ( histogram, SIGNAL( triggered()), this,
      SLOT( doHistogramSettings()));

  settingsMenu = menuBar()->addMenu ( tr ( "&Settings" ));
  settingsMenu->addAction ( general );
  // FIX ME -- temporarily disabled
  // settingsMenu->addAction ( capture );
  settingsMenu->addAction ( profiles );
  settingsMenu->addAction ( filters );
#ifdef ENABLE_DEBAYER
  settingsMenu->addAction ( debayer );
#endif
  settingsMenu->addAction ( autorun );
  settingsMenu->addAction ( histogram );

  // help menu

  about = new QAction ( tr ( "About" ), this );
  connect ( about, SIGNAL( triggered()), this, SLOT( aboutDialog()));

  helpMenu = menuBar()->addMenu ( tr ( "&Help" ));
  helpMenu->addAction ( about );
}


void
MainWindow::connectCamera ( int deviceIndex )
{
  if ( -1 == oldHistogramState ) {
    oldHistogramState = state.histogramOn;
  }
  state.histogramOn = 0;
  doDisconnect();
  if ( !state.camera->initialise ( devs[ deviceIndex ] )) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to connect camera" ));
    state.histogramOn = oldHistogramState;
    return;
  }

  disconnect->setEnabled( 1 );
  rescan->setEnabled( 0 );
  displayWindow->configure();
  statusLine->showMessage ( state.camera->name() + tr ( " connected" ), 5000 );
  state.captureWidget->enableStartButton ( 1 );
  // FIX ME -- should these happen in the "configure" functions for each
  // widget?
  state.previewWidget->setVideoFramePixelFormat (
      state.camera->videoFramePixelFormat());
  state.cameraWidget->enableBinningControl ( !state.camera->isColour());
  state.previewWidget->enableTempDisplay ( state.camera->hasTemperature());
  state.captureWidget->enableSERCapture ( !state.camera->isColour());

  enableFlipX();
  enableFlipY();

  // start regardless of whether we're displaying or capturing the
  // data
  state.camera->start();
  state.histogramOn = oldHistogramState;
  oldHistogramState = -1;
}


void
MainWindow::disconnectCamera ( void )
{
  oldHistogramState = state.histogramOn;
  state.histogramOn = 0;
  state.captureWidget->enableStartButton ( 0 );
  doDisconnect();
  statusLine->showMessage ( tr ( "Camera disconnected" ));
}


void
MainWindow::doDisconnect ( void )
{
  if ( state.camera && state.camera->isInitialised()) {
    if ( state.captureWidget ) {
      state.captureWidget->closeOutputHandler();
    }
    state.camera->stop();
    state.camera->disconnect();
    disconnect->setEnabled( 0 );
    rescan->setEnabled( 1 );
  }
}


void
MainWindow::rescanCameras ( void )
{
  doCameraMenu();
}


void
MainWindow::setCapturedFrames ( unsigned int newVal )
{
  QString stringVal;

  stringVal.setNum ( newVal );
  capturedValue->setText ( stringVal );
}


void
MainWindow::setActualFrameRate ( unsigned int count )
{
  QString stringVal;

  stringVal.setNum ( count );
  fpsActualValue->setText ( stringVal );
}


void
MainWindow::setTemperature ( float temp )
{
  QString stringVal;

  stringVal.setNum ( temp, 'g', 3 );
  tempValue->setText ( stringVal );
}


void
MainWindow::clearTemperature ( void )
{
  tempValue->setText ( "" );
}


void
MainWindow::quit ( void )
{
  doingQuit = 1;
  doDisconnect();
  writeConfig();
  qApp->quit();
}


void
MainWindow::showFPSMaxValue ( int value )
{
  fpsMaxValue->setText ( QString::number ( value ));
}


void
MainWindow::clearFPSMaxValue ( void )
{
  fpsMaxValue->setText ( "" );
}


void
MainWindow::showStatusMessage ( QString message )
{
  statusLine->showMessage ( message );
}


void
MainWindow::setNightMode ( int mode )
{
  // FIX ME -- need to set flag so subwindows can be started with the
  // correct stylesheet
  if ( mode ) {
    setNightStyleSheet ( this );
    if ( state.histogramWidget ) {
      setNightStyleSheet ( state.histogramWidget );
    }
    config.nightMode = 1;
  } else {
    clearNightStyleSheet ( this );
    if ( state.histogramWidget ) {
      clearNightStyleSheet ( state.histogramWidget );
    }
    config.nightMode = 0;
  }
  update();
}


void
MainWindow::enableHistogram ( void )
{
  if ( histogramOpt->isChecked()) {
    if ( !state.histogramWidget ) {
      state.histogramWidget = new HistogramWidget();
      // need to do this to be able to uncheck the menu item on closing
      state.histogramWidget->setAttribute ( Qt::WA_DeleteOnClose );
      state.histogramWidget->setWindowFlags ( Qt::WindowStaysOnTopHint );
      connect ( state.histogramWidget, SIGNAL( destroyed ( QObject* )), this,
          SLOT ( histogramClosed()));
    }
    state.histogramWidget->show();
    state.histogramOn = 1;
    config.showHistogram = 1;
  } else {
    if ( state.histogramWidget ) {
      state.histogramWidget->hide();
    }
    state.histogramOn = 0;
    config.showHistogram = 0;
  }
}


void
MainWindow::histogramClosed ( void )
{
  state.histogramWidget = 0;
  state.histogramOn = 0;
  histogramOpt->setChecked ( 0 );
  // We don't want to change this if the histogram window is closing because
  // we exited
  if ( !doingQuit ) {
    config.showHistogram = 0;
  }
}


void
MainWindow::setNightStyleSheet ( QWidget* window )
{
  window->setStyleSheet (
      "background: rgb( 92, 0, 0 );"
      "border-color: rgb( 192, 0, 0 );"
  );
}


void
MainWindow::clearNightStyleSheet ( QWidget* window )
{
  window->setStyleSheet ( "" );
}


void
MainWindow::enableReticle ( void )
{
  config.showReticle = reticle->isChecked() ? 1 : 0;
}


void
MainWindow::enableFlipX ( void )
{
  int flipState = flipX->isChecked() ? 1 : 0;

  config.flipX = flipState;
  if ( state.camera && state.camera->hasControl ( OA_CTRL_HFLIP )) {
    state.camera->setControl ( OA_CTRL_HFLIP, flipState );
    return;
  }
  state.previewWidget->enableFlipX ( flipState );
}


void
MainWindow::enableFlipY ( void )
{
  int flipState = flipY->isChecked() ? 1 : 0;

  config.flipY = flipState;
  if ( state.camera && state.camera->hasControl ( OA_CTRL_VFLIP )) {
    state.camera->setControl ( OA_CTRL_VFLIP, flipState );
    return;
  }
  state.previewWidget->enableFlipY ( flipState );
}


void
MainWindow::aboutDialog ( void )
{
  QMessageBox::about ( this, tr ( "About " APPLICATION_NAME ),
      tr ( "<h2>" APPLICATION_NAME " " VERSION_STR "</h2>"
      "<p>Copyright &copy; " COPYRIGHT_YEARS " " AUTHOR_NAME "</p>"
      "<p>" APPLICATION_NAME " is an open source video capture application "
      "intended primarily for planetary imaging."
      "<p>Thanks are due to numerous forum members for testing and "
      "encouragement and to those camera manufacturers who have provided "
      "documentation, Linux SDKs and other help without which this "
      "application would have taken much longer to create." ));
}


void
MainWindow::setProgress ( int progress )
{
  progressBar->setValue ( progress );
}


void
MainWindow::doGeneralSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.generalSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doCaptureSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.captureSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doProfileSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.profileSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doFilterSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.filterSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doAutorunSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.autorunSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::doHistogramSettings ( void )
{
  createSettingsWidget();
  state.settingsWidget->setActiveTab ( state.histogramSettingsIndex );
  state.settingsWidget->show();
}


void
MainWindow::createSettingsWidget ( void )
{
  if ( !state.settingsWidget ) {
    state.settingsWidget = new SettingsWidget();
    state.settingsWidget->setWindowFlags ( Qt::WindowStaysOnTopHint );
    state.settingsWidget->setAttribute ( Qt::WA_DeleteOnClose );
    connect ( state.settingsWidget, SIGNAL( destroyed ( QObject* )), this,
        SLOT ( settingsClosed()));
  }
}


void
MainWindow::settingsClosed ( void )
{
  state.settingsWidget = 0;
}


void
MainWindow::doCameraMenu ( void )
{
  int numDevs;
  int i;

  // FIX ME -- move the device type stuff into the library

  devTypes[0] = "unknown";
  devTypes[1] = "V4L2";
  devTypes[2] = "PWC";
  devTypes[3] = "ZWASI";
  devTypes[4] = "QHY";
  devTypes[5] = "PGR";

  if ( rescan ) {
    rescan->setEnabled( 0 );
  }
  if ( connectedCameras ) {
    for ( i = 0; i < connectedCameras; i++ ) {
      cameraMenu->removeAction ( cameras[i] );
      delete cameras[i];
    }
    delete signalMapper;
  }

  bzero ( devs, sizeof ( devs ));
  numDevs = state.camera->listConnected ( devs );

  if ( numDevs > 0 ) {
    signalMapper = new QSignalMapper ( this );
    for ( i = 0; i < numDevs && devs[i]; i++ ) {
      QString iface ( devTypes[ devs[i]->interface ]);
      QString name ( devs[i]->deviceName );
      menuEntry[i] = "(" + iface + ") " + name;
      cameras[i] = new QAction ( menuEntry[i], this );
      if ( cameraMenuCreated ) {
        cameraMenu->insertAction ( cameraMenuSeparator, cameras[i] );
      } else {
        cameraMenu->addAction ( cameras[i] );
      }
      signalMapper->setMapping ( cameras[i], i );
      connect ( cameras[i], SIGNAL( triggered()), signalMapper, SLOT( map()));
    }
    connect ( signalMapper, SIGNAL( mapped ( int )), this,
        SLOT( connectCamera ( int )));
  }

  if ( !cameraMenuCreated ) {
    cameraMenuSeparator = cameraMenu->addSeparator();
    rescan = new QAction ( tr ( "Rescan" ), this );
    rescan->setStatusTip ( tr ( "Scan for newly connected devices" ));
    connect ( rescan, SIGNAL( triggered()), this, SLOT( rescanCameras() ));
    disconnect = new QAction ( tr ( "Disconnect" ), this );
    connect ( disconnect, SIGNAL( triggered()), this,
        SLOT( disconnectCamera()));
    disconnect->setEnabled( 0 );
    cameraMenu->addAction ( rescan );
    cameraMenu->addAction ( disconnect );
  }

  cameraMenuCreated = 1;
  connectedCameras = numDevs;
  if ( rescan ) {
    rescan->setEnabled( 1 );
  }
}


void
MainWindow::closeSettingsWindow ( void )
{
  if ( state.settingsWidget ) {
    state.settingsWidget->close();
    state.settingsWidget = 0;
  }
}
