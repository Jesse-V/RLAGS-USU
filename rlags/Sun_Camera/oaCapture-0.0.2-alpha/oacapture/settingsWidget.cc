/*****************************************************************************
 *
 * settingsWidget.cc -- the main settings widget wrapper class
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

#include "version.h"
#include "config.h"
#include "state.h"
#include "mainWindow.h"
#include "settingsWidget.h"
#include "generalSettings.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"


SettingsWidget::SettingsWidget()
{
  setWindowTitle( APPLICATION_NAME " Settings" );
  setWindowIcon ( QIcon ( ":/icons/configure-3.png" ));

  general = new GeneralSettings ( this );
  capture = new CaptureSettings ( this );
  profiles = new ProfileSettings ( this );
  filters = new FilterSettings ( this );
  autorun = new AutorunSettings ( this );
  histogram = new HistogramSettings ( this );
  vbox = new QVBoxLayout ( this );
  tabSet = new QTabWidget ( this );
  buttonBox = new QHBoxLayout();
  cancelButton = new QPushButton ( tr ( "Cancel" ), this );
  saveButton = new QPushButton ( tr ( "Save" ), this );

  state.generalSettingsIndex = tabSet->addTab ( general, 
      QIcon ( ":/icons/cog.png" ), tr ( "General" ));
  // FIX ME -- temporarily disabled
  // state.captureSettingsIndex = tabSet->addTab ( capture, tr ( "Capture" ));
  state.profileSettingsIndex = tabSet->addTab ( profiles,
      QIcon ( ":/icons/jupiter.png" ), tr ( "Profiles" ));
  state.filterSettingsIndex = tabSet->addTab ( filters,
      QIcon ( ":/icons/filter-wheel.png" ), tr ( "Filters" ));
  state.autorunSettingsIndex = tabSet->addTab ( autorun,
      QIcon ( ":/icons/clicknrun.png" ), tr ( "Autorun" ));
  state.histogramSettingsIndex = tabSet->addTab ( histogram,
      QIcon ( ":/icons/barchart.png" ), tr ( "Histogram" ));

  tabSet->setUsesScrollButtons ( false );

  buttonBox->addStretch ( 1 );
  buttonBox->addWidget ( cancelButton );
  buttonBox->addWidget ( saveButton );

  vbox->addWidget ( tabSet );
  vbox->addLayout ( buttonBox );
  setLayout ( vbox );

  connect ( cancelButton, SIGNAL( clicked()), state.mainWindow,
      SLOT( closeSettingsWindow()));
  connect ( saveButton, SIGNAL( clicked()), this, SLOT( storeSettings()));
}


SettingsWidget::~SettingsWidget()
{
/*
  delete vbox;
  delete buttonBox;
  delete cancelButton;
  delete saveButton;
  delete tabSet;
  delete histogram;
  delete autorun;
  delete profiles;
  delete filters;
  delete capture;
  delete general;
*/
}


void
SettingsWidget::setActiveTab ( int index )
{
  tabSet->setCurrentIndex ( index );
}


void
SettingsWidget::storeSettings ( void )
{
  general->storeSettings();
  capture->storeSettings();
  profiles->storeSettings();
  filters->storeSettings();
  autorun->storeSettings();
  histogram->storeSettings();
  state.mainWindow->showStatusMessage ( tr ( "Changes saved" ));
}
