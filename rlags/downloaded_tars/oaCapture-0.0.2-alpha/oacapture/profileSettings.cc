/*****************************************************************************
 *
 * profileSettings.cc -- class for profile settings tab in the settings UI
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

#include "config.h"
#include "state.h"

#include "profileSettings.h"


ProfileSettings::ProfileSettings ( QWidget* parent ) : QWidget ( parent )
{
  list = new QListWidget ( this );
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      list->addItem ( config.profiles[i].profileName );
      QListWidgetItem* entry = list->item ( i );
      entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
    }
  }
  addButton = new QPushButton ( QIcon ( ":/icons/list-add-4.png" ),
      tr ( "Add Profile" ));
  addButton->setToolTip (
      tr ( "Create a new profile with the current settings" ));
  addButton->setStyleSheet("Text-align:left");
  connect ( addButton, SIGNAL ( clicked()), this, SLOT ( addEntry()));
  removeButton = new QPushButton ( QIcon ( ":/icons/list-remove-4.png" ),
      tr ( "Remove Profile" ));
  removeButton->setStyleSheet("Text-align:left");
  connect ( removeButton, SIGNAL ( clicked()), this, SLOT ( removeEntry()));
  connect ( list, SIGNAL ( currentItemChanged ( QListWidgetItem*,
      QListWidgetItem* )), this, SLOT ( focusChanged ( QListWidgetItem*,
      QListWidgetItem* )));
  connect ( list, SIGNAL ( itemChanged ( QListWidgetItem* )), this,
      SLOT ( itemChanged ( QListWidgetItem* )));

  vbox = new QVBoxLayout();
  vbox->addStretch ( 3 );
  vbox->addWidget ( addButton );
  vbox->addWidget ( removeButton );
  vbox->addStretch ( 3 );

  hbox = new QHBoxLayout ( this );
  hbox->addWidget ( list );
  hbox->addLayout ( vbox );
  hbox->addStretch ( 1 );
  setLayout ( hbox );
  listAltered = 0;

  // we work on a copy of the profile list to avoid messing up the live
  // one
  if ( config.numProfiles ) {
    for ( int i = 0; i < config.numProfiles; i++ ) {
      changedProfiles.append ( config.profiles.at ( i ));
    }
  }
}


ProfileSettings::~ProfileSettings()
{
/*
  delete hbox;
  delete vbox;
  delete removeButton;
  delete addButton;
  delete list;
*/
}


void
ProfileSettings::storeSettings ( void )
{
  if ( listAltered ) {
    if ( config.numProfiles ) {
      config.profiles.clear();
    }
    config.numProfiles = list->count();
    config.profiles = changedProfiles;
    state.captureWidget->reloadProfiles();
  }
}


void
ProfileSettings::addEntry ( void )
{
  PROFILE p;
  p.profileName = "";
  p.sixteenBit = config.sixteenBit;
  p.binning2x2 = config.binning2x2;
  p.useROI = config.useROI;
  p.imageSizeX = config.imageSizeX;
  p.imageSizeY = config.imageSizeY;
  p.gainValue = config.gainValue;
  p.exposureValue = config.exposureValue;
  p.exposureAbsoluteValue = config.exposureAbsoluteValue;
  p.gammaValue = config.gammaValue;
  p.brightnessValue = config.brightnessValue;
  p.frameRateNumerator = config.frameRateNumerator;
  p.frameRateDenominator = config.frameRateDenominator;
  p.filterOption = config.filterOption;
  p.fileTypeOption = config.fileTypeOption;
  p.fileNameTemplate = config.fileNameTemplate;
  p.limitEnabled = config.limitEnabled;
  p.limitValue = config.limitValue;
  changedProfiles.append ( p );
  QListWidgetItem* entry = new QListWidgetItem ( "" );
  entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
  list->addItem ( entry );
  list->editItem ( entry );
  listAltered = 1;
}


void
ProfileSettings::removeEntry ( void )
{
  int position = list->currentRow();
  QListWidgetItem* entry = list->takeItem ( position );
  if ( entry ) {
    delete entry;
    changedProfiles.removeAt ( position );
    listAltered = 1;
  }
}


void
ProfileSettings::focusChanged ( QListWidgetItem* curr, QListWidgetItem* old )
{
  Q_UNUSED ( curr )
  if ( old && old->text() == "" ) {
    int position = list->row ( old );
    ( void ) list->takeItem ( position );
    delete old;
    changedProfiles.removeAt ( position );
    listAltered = 1;
  }
}


void
ProfileSettings::itemChanged ( QListWidgetItem* item )
{
  int position = list->row ( item );
  if ( item->text() != changedProfiles[ position ].profileName ) {
    changedProfiles[ position ].profileName = item->text();
  }
  listAltered = 1;
}
