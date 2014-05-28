/*****************************************************************************
 *
 * autorunSettings.cc -- class for autorun settings in the settings widget
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

#include "autorunSettings.h"

#include "config.h"
#include "state.h"

AutorunSettings::AutorunSettings ( QWidget* parent ) : QWidget ( parent )
{
  numRunsLabel = new QLabel ( tr ( "Number of runs" ), this );
  numRuns = new QLineEdit ( this );
  numRunsValidator = new QIntValidator ( 1, 99999, this );
  numRuns->setValidator ( numRunsValidator );

  delayLabel = new QLabel ( tr ( "Delay between runs" ), this );
  delayLabel2 = new QLabel ( tr ( "(seconds)" ), this );
  delay = new QLineEdit ( this );
  delayValidator = new QIntValidator ( 0, 99999, this );
  delay->setValidator ( delayValidator );

  enableButton = new QPushButton ( tr ( "Enable Autorun" ));

  grid = new QGridLayout ( this );
  grid->addWidget ( numRunsLabel, 0, 0 );
  grid->addWidget ( numRuns, 0, 1 );
  grid->addWidget ( delayLabel, 1, 0 );
  grid->addWidget ( delay, 1, 1 );
  grid->addWidget ( delayLabel2, 1, 2 );
  grid->setRowStretch ( 2, 1 );
  grid->addWidget ( enableButton, 3, 1 );

  setLayout ( grid );

  connect ( enableButton, SIGNAL ( clicked()), this, SLOT ( enableAutorun()));
}


AutorunSettings::~AutorunSettings()
{
  delete grid;
  delete delay;
  delete delayValidator;
  delete delayLabel;
  delete numRuns;
  delete numRunsValidator;
  delete numRunsLabel;
}


void
AutorunSettings::storeSettings ( void )
{
  QString countStr = numRuns->text();
  QString delayStr = delay->text();
  if ( countStr != "" ) {
    config.autorunCount = countStr.toInt();
  }
  if ( delayStr != "" ) {
    config.autorunDelay = delayStr.toInt();
  }
}


void
AutorunSettings::enableAutorun ( void )
{
  storeSettings();
  state.captureWidget->resetAutorun();
}
