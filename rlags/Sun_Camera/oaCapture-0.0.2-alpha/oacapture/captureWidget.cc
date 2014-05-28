/*****************************************************************************
 *
 * captureWidget.cc -- class for the capture widget in the UI
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

#include <unistd.h>

#include <ctime>
#include <iostream>
#include <fstream>
#include <QtGui>

#include "config.h"
#include "version.h"
#include "state.h"
#include "captureWidget.h"
#include "outputAVI.h"
#include "outputSER.h"
#include "outputFITS.h"

static QString	fileFormats[4] = { "", "AVI", "SER", "FITS" };

CaptureWidget::CaptureWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  box = new QVBoxLayout ( this );
  profile = new QHBoxLayout;
  file = new QHBoxLayout;
  type = new QHBoxLayout;
  controls = new QHBoxLayout;

  profileLabel = new QLabel ( tr ( "Profile:" ), this );
  profileMenu = new QComboBox ( this );
  QStringList profileNames;
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      profileNames << config.profiles[i].profileName;
    }
  }
  profileMenu->addItems ( profileNames );
  if ( config.numProfiles ) {
    profileMenu->setCurrentIndex ( config.profileOption );
  }
  connect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));

  restoreButton = new QPushButton (
      QIcon ( ":/icons/arrow-redo.png" ), "", this );
  restoreButton->setToolTip ( tr ( "Restore settings for current profile" ));
  connect ( restoreButton, SIGNAL( clicked()), this,
      SLOT( updateSettingsFromProfile()));
  if ( !config.numProfiles ) {
    restoreButton->setEnabled ( 0 );
  }

  filterLabel = new QLabel ( tr ( "Filter:" ), this );
  filterMenu = new QComboBox ( this );
  QStringList filterNames;
  if ( config.numFilters ) {
    for ( int i = 0; i < config.numFilters; i++ ) {
      filterNames << config.filters[i].filterName;
    }
  }
  filterMenu->addItems ( filterNames );
  if ( config.numFilters ) {
    filterMenu->setCurrentIndex ( config.filterOption );
  }
  connect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));

  profile->addWidget ( profileLabel );
  profile->addWidget ( profileMenu );
  profile->addWidget ( restoreButton );
  profile->addWidget ( filterLabel );
  profile->addWidget ( filterMenu );

  fileLabel = new QLabel ( tr ( "File: " ), this );
  fileName = new QLineEdit ( this );
  fileName->setText ( config.fileNameTemplate );
  QRegExp rx( "[^/; <>]+" );
  fileNameValidator = new QRegExpValidator ( rx, this );
  fileName->setValidator ( fileNameValidator );
  connect ( fileName, SIGNAL( editingFinished()), this,
      SLOT( updateFileNameTemplate()));

  newFolderButton = new QPushButton (
      QIcon ( ":/icons/folder-new-7.png" ), "", this );
  newFolderButton->setToolTip ( tr ( "Select a new capture directory" ));
  connect ( newFolderButton, SIGNAL( clicked()), this,
      SLOT( setNewCaptureDirectory()));

  deleteButton = new QPushButton (
      QIcon ( ":/icons/user-trash.png" ), "", this );
  deleteButton->setToolTip ( tr ( "Delete last captured file" ));
  connect ( deleteButton, SIGNAL( clicked()), this,
      SLOT( deleteLastRecordedFile()));

  openFolderButton = new QPushButton (
      QIcon ( ":/icons/folder-open-4.png" ), "", this );
  openFolderButton->setToolTip ( tr ( "View capture directory" ));
  connect ( openFolderButton, SIGNAL( clicked()), this,
      SLOT( openCaptureDirectory()));

  file->addWidget ( fileLabel );
  file->addWidget ( fileName );
  file->addWidget ( newFolderButton );
  file->addWidget ( deleteButton );
  file->addWidget ( openFolderButton );

  typeLabel = new QLabel ( tr ( "Type:" ), this );
  typeMenu = new QComboBox ( this );
  QStringList formats;
  formats << fileFormats[ CAPTURE_AVI ]
      << fileFormats[ CAPTURE_FITS ]
      << fileFormats[ CAPTURE_SER ];
  typeMenu->addItems ( formats );
  typeMenu->setCurrentIndex ( config.fileTypeOption - 1 );
  connect ( typeMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( fileTypeChanged ( int )));

  limitCheckbox = new QCheckBox ( tr ( "Limit:" ), this );
  limitCheckbox->setToolTip ( tr ( "Set a capture time limit" ));
  limitCheckbox->setChecked ( config.limitEnabled );
  connect ( limitCheckbox, SIGNAL( stateChanged ( int )), this,
      SLOT( showLimitInputBox ( int )));

  secondsLabel = new QLabel ( tr ( "Secs." ), this );
  seconds = new QLineEdit ( this );
  secondsValidator = new QIntValidator ( 0, 99999, this );
  seconds->setValidator ( secondsValidator );
  QString secStr;
  if ( config.limitValue > 0 ) {
    secStr = QString::number ( config.limitValue );
  } else {
    secStr = "0";
  }
  seconds->setText ( secStr );
  if ( config.limitEnabled ) {
    secondsLabel->show();
    seconds->show();
  } else {
    secondsLabel->hide();
    seconds->hide();
  }
  connect ( seconds, SIGNAL( textChanged ( const QString& )), this,
      SLOT( changeLimitSeconds ( const QString& )));

  type->addWidget ( typeLabel );
  type->addWidget ( typeMenu );
  type->addWidget ( limitCheckbox );
  type->addWidget ( seconds );
  type->addWidget ( secondsLabel );
  type->addStretch ( 1 );

  startButton = new QPushButton (
      QIcon ( ":/icons/media-playback-start-7.png" ), "", this );
  startButton->setToolTip ( tr ( "Start capturing data" ));
  connect ( startButton, SIGNAL( clicked()), this, SLOT( startRecording()));

#ifdef PAUSE_RECORD
  pauseButton = new QPushButton (
      QIcon ( ":/icons/media-playback-pause-7.png" ), "", this );
#endif

  stopButton = new QPushButton (
      QIcon ( ":/icons/media-playback-stop-7.png" ), "", this );
  stopButton->setToolTip ( tr ( "Stop capturing data" ));
  connect ( stopButton, SIGNAL( clicked()), this, SLOT( stopRecording()));

#ifdef SESSION_BROWSER
  fileListButton = new QPushButton (
      QIcon ( ":/icons/format-list-unordered.png" ), "", this );
#endif

  autorunButton = new QPushButton (
      QIcon ( ":/icons/clicknrun_grey.png" ), "", this );
  autorunButton->setToolTip ( tr ( "Reset autorun settings" ));
  autorunLabel = new QLabel ( "   " );

  startButton->setEnabled ( 0 );
#ifdef PAUSE_RECORD
  pauseButton->setEnabled ( 0 );
#endif
  stopButton->setEnabled ( 0 );
  if ( config.autorunCount && config.limitEnabled && config.limitValue ) {
    autorunButton->setEnabled ( 1 );
  } else {
    autorunButton->setEnabled ( 0 );
  }
  connect ( autorunButton, SIGNAL( clicked()), this, SLOT( resetAutorun()));

  controls->addWidget ( startButton );
#ifdef PAUSE_RECORD
  controls->addWidget ( pauseButton );
#endif
  controls->addWidget ( stopButton );
#ifdef SESSION_BROWSER
  controls->addWidget ( fileListButton );
#endif
  controls->addWidget ( autorunButton );
  controls->addWidget ( autorunLabel );

  box->addLayout ( profile );
  box->addLayout ( file );
  box->addLayout ( type );
  box->addLayout ( controls );

  setTitle ( tr ( "Capture" ));
  setLayout ( box );

  outputHandler = 0;
}


CaptureWidget::~CaptureWidget()
{
#ifdef SESSION_BROWSER
  delete fileListButton;
#endif
  delete stopButton;
  delete startButton;
  delete secondsValidator;
  delete seconds;
  delete limitCheckbox;
  delete typeMenu;
  delete typeLabel;
  delete openFolderButton;
  delete deleteButton;
  delete newFolderButton;
  delete fileName;
  delete fileLabel;
  delete filterMenu;
  delete filterLabel;
  delete profileMenu;
  delete profileLabel;
  delete controls;
  delete type;
  delete file;
  delete profile;
  delete box;
}


void
CaptureWidget::showLimitInputBox ( int state )
{
  if ( state == Qt::Unchecked ) {
    config.limitEnabled = 0;
    secondsLabel->hide();
    seconds->hide();
  } else {
    config.limitEnabled = 1;
    secondsLabel->show();
    seconds->show();
  }
}


void
CaptureWidget::fileTypeChanged ( int index )
{
  config.fileTypeOption = index + 1;
}


void
CaptureWidget::startRecording ( void )
{
  doStartRecording ( 0 );
  if ( state.autorunEnabled ) {
    autorunLabel->setText ( "1 of " + QString::number ( config.autorunCount ));
  }
}


void
CaptureWidget::doStartRecording ( int autorunFlag )
{
  OutputHandler *out = 0;

  switch ( config.fileTypeOption ) {
    case CAPTURE_AVI:
      out = new OutputAVI ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator(),
          state.camera->videoFramePixelFormat());
      break;
    case CAPTURE_SER:
      out = new OutputSER ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator());
      break;
    case CAPTURE_FITS:
      out = new OutputFITS ( config.imageSizeX, config.imageSizeY,
          state.controlWidget->getFPSNumerator(),
          state.controlWidget->getFPSDenominator());
      break;
  }

  if ( out->outputExists()) {
    if ( out->outputWritable()) {
      if ( QMessageBox::question ( this, tr ( "Start Recording" ),
          tr ( "Output file exists.  OK to overwrite?" ), QMessageBox::No |
          QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No ) {
        delete out;
        out = 0;
        return;
      }
    } else {
      QMessageBox::warning ( this, tr ( "Start Recording" ),
        tr ( "Output file exists and is not writable" ));
      delete out;
      out = 0;
      return;
    }
  }

  if ( !out || out->openOutput()) {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to create file for output" ));
    if ( state.autorunEnabled ) {
      state.autorunRemaining = 1;
      singleAutorunFinished();
      autorunFlag = 0;
    }
    startButton->setEnabled ( 1 );
#ifdef PAUSE_RECORD
    pauseButton->setEnabled ( 0 );
#endif
    state.imageWidget->enableAllControls ( 1 );
    state.controlWidget->enableFPSControl ( 1 );
    return;
  }

  writeSettings ( out );

  outputHandler = out;
  state.mainWindow->showStatusMessage ( tr ( "Recording started" ));
  state.lastRecordedFile = out->getRecordingFilename();

  if ( config.limitEnabled && config.limitValue ) {
    struct timeval t;
    ( void ) gettimeofday ( &t, 0 );
    unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
        ( unsigned long ) t.tv_usec / 1000;
    recordingStartTime = now;
    recordingEndTime = now + config.limitValue * 1000;
  }

  if ( !autorunFlag && state.autorunEnabled ) {
    state.autorunRemaining = config.autorunCount;
  }

  startButton->setEnabled ( 0 );
  stopButton->setEnabled ( 1 );
#ifdef PAUSE_RECORD
  pauseButton->setEnabled ( 1 );
#endif
  autorunButton->setEnabled ( 0 );
  state.imageWidget->enableAllControls ( 0 );
  state.controlWidget->enableFPSControl ( 0 );
}


void
CaptureWidget::stopRecording ( void )
{
  if ( state.autorunEnabled ) {
    state.autorunEnabled = 0;
    state.autorunStartNext = 0;
    state.autorunRemaining = 0;
  }
  doStopRecording();
  if ( config.autorunCount && config.limitEnabled && config.limitValue ) {
    autorunButton->setEnabled ( 1 );
  }
  stopButton->setEnabled ( 0 );
}


void
CaptureWidget::doStopRecording ( void )
{
  closeOutputHandler();
  startButton->setEnabled ( 1 );
#ifdef PAUSE_RECORD
  pauseButton->setEnabled ( 0 );
#endif
  state.imageWidget->enableAllControls ( 1 );
  state.controlWidget->enableFPSControl ( 1 );
}


void
CaptureWidget::enableStartButton ( int state )
{
  startButton->setEnabled ( state );
}


void
CaptureWidget::closeOutputHandler ( void )
{
  if ( outputHandler ) {
    outputHandler->closeOutput();
    delete outputHandler;
    outputHandler = 0;
    state.mainWindow->showStatusMessage ( tr ( "Recording stopped" ));
  }
}


OutputHandler*
CaptureWidget::getOutputHandler ( void )
{
  return outputHandler;
}


void
CaptureWidget::enableSERCapture ( int state )
{
  int haveSER = ( 3 == typeMenu->count()) ? 1 : 0;
  if ( haveSER && !state ) {
    typeMenu->removeItem ( 2 );
  }
  if ( !haveSER && state ) {
    typeMenu->addItem ( fileFormats[ CAPTURE_SER ] );
  }
}


void
CaptureWidget::setNewCaptureDirectory ( void )
{
  QFileDialog dialog( this );

  dialog.setFileMode ( QFileDialog::Directory );
  dialog.setOption ( QFileDialog::ShowDirsOnly );
  if ( config.captureDirectory != "" ) {
    dialog.setDirectory ( config.captureDirectory );
  } else {
    dialog.setDirectory ( "." );
  }
  dialog.setWindowTitle ( tr ( "Select capture directory" ));
  int done = 0;
  while ( !done ) {
    if ( dialog.exec()) {
      QStringList names = dialog.selectedFiles();
      if ( !access ( names[0].toStdString().c_str(), W_OK | R_OK )) {
        config.captureDirectory = names[0];
        done = 1;
      } else {
        QMessageBox err;
        err.setText ( tr (
            "The selected directory is not writable/accessible" ));
        err.setInformativeText ( tr ( "Select another?" ));
        err.setStandardButtons ( QMessageBox::No | QMessageBox::Yes );
        err.setDefaultButton ( QMessageBox::Yes );
        int ret = err.exec();
        done = ( ret == QMessageBox::Yes ) ? 0 : 1;
      }
    } else {
      done = 1;
    }
  }
}


void
CaptureWidget::changeLimitSeconds ( const QString& secStr )
{
  config.limitValue = secStr.toInt();
}


void
CaptureWidget::enableAutorun ( void )
{
  autorunButton->setEnabled ( 1 );
  autorunButton->setIcon ( QIcon ( ":/icons/clicknrun.png" ) );
  autorunLabel->setText ( "0 of " + QString::number ( config.autorunCount ));
  // set this to 0 to stop autorun being started automagicallly until
  // the first one is kicked off with the start button
  state.autorunStartNext = 0;
}


int
CaptureWidget::singleAutorunFinished ( void )
{
  if ( !--state.autorunRemaining ) {
    autorunLabel->setText ( "     " );
    state.autorunEnabled = 0;
    state.autorunStartNext = 0;
    stopButton->setEnabled ( 0 );
  }
  return state.autorunRemaining;
}


void
CaptureWidget::startNewAutorun ( void )
{
  doStartRecording ( 1 );
  autorunLabel->setText ( QString::number ( config.autorunCount -
      state.autorunRemaining + 1 ) + " of " +
      QString::number ( config.autorunCount ));
  state.autorunStartNext = 0;
}


void
CaptureWidget::resetAutorun ( void )
{
  if ( config.autorunCount ) {
    enableAutorun();
    state.autorunEnabled = 1;
    state.mainWindow->showStatusMessage ( tr ( "Autorun Enabled" ));
  }
}


void
CaptureWidget::deleteLastRecordedFile ( void )
{
  if ( "" == state.lastRecordedFile ) {
    QMessageBox::warning ( this, tr ( "Delete File" ),
        tr ( "No last file to delete" ));
    return;
  }

  QString name = state.lastRecordedFile.section ( '/', -1 );
  if ( QMessageBox::question ( this, tr ( "Delete File" ),
      tr ( "Delete file " ) + name, QMessageBox::Cancel | QMessageBox::Yes,
      QMessageBox::Cancel ) == QMessageBox::Yes ) {
    if ( unlink ( state.lastRecordedFile.toStdString().c_str())) {
      QMessageBox::warning ( this, tr ( "Delete File" ),
      tr ( "Delete failed for" ) + name );
    }
    state.lastRecordedFile = "";
  }
}


void
CaptureWidget::openCaptureDirectory ( void )
{
  QFileDialog dialog( this );

  dialog.setFileMode ( QFileDialog::AnyFile );
  if ( config.captureDirectory != "" ) {
    dialog.setDirectory ( config.captureDirectory );
  } else {
    dialog.setDirectory ( "." );
  }
  dialog.setWindowTitle ( tr ( "Capture directory" ));
  dialog.exec();
}


void
CaptureWidget::updateFileNameTemplate ( void )
{
  config.fileNameTemplate = fileName->text();
}


QString
CaptureWidget::getCurrentFilterName ( void )
{
  if ( config.numFilters > 0 && config.filterOption < config.numFilters ) {
    return ( config.filters[ config.filterOption ].filterName );
  }
  return "";
}


void
CaptureWidget::filterTypeChanged ( int index )
{
  config.filterOption = index;
}


QString
CaptureWidget::getCurrentProfileName ( void )
{
  if ( config.numProfiles > 0 && config.profileOption < config.numProfiles ) {
    return config.profiles[ config.profileOption ].profileName;
  }
  return "";
}


void
CaptureWidget::profileTypeChanged ( int index )
{
  config.profileOption = index;
  updateSettingsFromProfile();
}


void
CaptureWidget::reloadFilters ( void )
{
  disconnect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));
  int oldCount = filterMenu->count();
  if ( config.numFilters ) {
    for ( int i = 0; i < config.numFilters; i++ ) {
      if ( i < oldCount ) {
        filterMenu->setItemText ( i, config.filters[i].filterName );
      } else {
        filterMenu->addItem ( config.filters[i].filterName );
      }
    }
  }
  if ( config.numFilters < oldCount ) {
    for ( int i = config.numFilters; i < oldCount; i++ ) {
      filterMenu->removeItem ( config.numFilters );
    }
  }
  connect ( filterMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( filterTypeChanged ( int )));
}


void
CaptureWidget::reloadProfiles ( void )
{
  disconnect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));
  int oldCount = profileMenu->count();
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      if ( i < oldCount ) {
        profileMenu->setItemText ( i, config.profiles[i].profileName );
      } else {
        profileMenu->addItem ( config.profiles[i].profileName );
      }
    }
  }
  if ( config.numProfiles < oldCount ) {
    for ( int i = config.numProfiles; i < oldCount; i++ ) {
      profileMenu->removeItem ( config.numProfiles );
    }
  }
  connect ( profileMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( profileTypeChanged ( int )));
  restoreButton->setEnabled ( config.numProfiles ? 1 : 0 );
}


void
CaptureWidget::updateSettingsFromProfile ( void )
{
  config.sixteenBit = config.profiles[ config.profileOption ].sixteenBit;
  config.binning2x2 = config.profiles[ config.profileOption ].binning2x2;
  config.useROI = config.profiles[ config.profileOption ].useROI;
  config.imageSizeX = config.profiles[ config.profileOption ].imageSizeX;
  config.imageSizeY = config.profiles[ config.profileOption ].imageSizeY;
  config.gainValue = config.profiles[ config.profileOption ].gainValue;
  config.exposureValue = config.profiles[ config.profileOption ].exposureValue;
  config.exposureAbsoluteValue =
      config.profiles[ config.profileOption ].exposureAbsoluteValue;
  config.gammaValue = config.profiles[ config.profileOption ].gammaValue;
  config.brightnessValue =
      config.profiles[ config.profileOption ].brightnessValue;
  config.frameRateNumerator =
      config.profiles[ config.profileOption ].frameRateNumerator;
  config.frameRateDenominator =
      config.profiles[ config.profileOption ].frameRateDenominator;
  config.filterOption =
      config.profiles[ config.profileOption ].filterOption;
  config.fileTypeOption =
      config.profiles[ config.profileOption ].fileTypeOption;
  config.fileNameTemplate =
      config.profiles[ config.profileOption ].fileNameTemplate;
  config.limitEnabled =
      config.profiles[ config.profileOption ].limitEnabled;
  config.limitValue =
      config.profiles[ config.profileOption ].limitValue;

  state.cameraWidget->updateFromConfig();
  state.imageWidget->updateFromConfig();
  state.controlWidget->updateFromConfig();
  updateFromConfig();
}


void
CaptureWidget::updateFromConfig ( void )
{
  if ( config.numFilters > config.filterOption ) {
    filterMenu->setCurrentIndex ( config.filterOption );
  }
  fileName->setText ( config.fileNameTemplate );
  if ( typeMenu->count() >= config.fileTypeOption ) {
    typeMenu->setCurrentIndex ( config.fileTypeOption - 1 );
  }
  limitCheckbox->setChecked ( config.limitEnabled );
  if ( config.limitValue > 0 ) {
    seconds->setText ( QString::number ( config.limitValue ));
  }
}


void
CaptureWidget::writeSettings ( OutputHandler* out )
{
  QString settingsFile = out->getRecordingBasename();
  settingsFile += ".txt";

  std::ofstream settings ( settingsFile.toStdString().c_str());
  if ( settings.is_open()) {
    time_t now = time(0);
    struct tm* tmp = localtime ( &now );
    settings << APPLICATION_NAME << " " << VERSION_STR << std::endl;
    settings << std::endl;
    settings << "camera: " << state.camera->name() << std::endl;
    settings << "time: " << tmp->tm_year + 1900 << "-" << tmp->tm_mon + 1 <<
        "-" << tmp->tm_mday << " " << tmp->tm_hour << ":" << tmp->tm_min <<
        ":" << tmp->tm_sec << std::endl;
    settings << "gain: " << config.gainValue << std::endl;
    settings << "gamma: " << config.gammaValue << std::endl;
    settings << "brightness: " << config.brightnessValue << std::endl;
    if ( state.camera->hasControl ( OA_CTRL_EXPOSURE_ABSOLUTE )) {
      settings << "exposure (ms): " << config.exposureAbsoluteValue <<
      std::endl;
    } else {
      settings << "exposure: " << config.exposureValue << std::endl;
    }
    if ( state.camera->hasFrameRateSupport()) {
      settings << "frame rate/sec: " << config.frameRateNumerator;
      if ( config.frameRateDenominator != 1 ) {
        settings << "/" << config.frameRateDenominator;
      }
      settings << std::endl;
    }
    settings << "image size: " << config.imageSizeX << "x" <<
        config.imageSizeY << std::endl;
    if ( config.numFilters > 0 && config.filterOption < config.numFilters ) {
      settings << "filter: " <<
          config.filters[ config.filterOption ].filterName.toStdString() <<
          std::endl;
    }
    settings.close();
  } else {
    QMessageBox::warning ( this, APPLICATION_NAME,
        tr ( "Unable to create settings output file" ));
  }
}
