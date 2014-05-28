/*****************************************************************************
 *
 * filterSettings.cc -- class for the filter settings tab in the settings UI
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

#include "filterSettings.h"


FilterSettings::FilterSettings ( QWidget* parent ) : QWidget ( parent )
{
  list = new QListWidget ( this );
  if ( config.numFilters ) {
    for ( int i = 0; i < config.numFilters; i++ ) {
      list->addItem ( config.filters[i].filterName );
      QListWidgetItem* entry = list->item ( i );
      entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
    }
  }
  addButton = new QPushButton ( QIcon ( ":/icons/list-add-4.png" ),
      tr ( "Add Item" ));
  addButton->setStyleSheet("Text-align:left");
  connect ( addButton, SIGNAL ( clicked()), this, SLOT ( addEntry()));
  removeButton = new QPushButton ( QIcon ( ":/icons/list-remove-4.png" ),
      tr ( "Remove Item" ));
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
}


FilterSettings::~FilterSettings()
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
FilterSettings::storeSettings ( void )
{
  if ( listAltered ) {
    if ( config.numFilters ) {
      config.filters.clear();
    }
    config.numFilters = list->count();
    if ( config.numFilters ) {
      for ( int i = 0; i < config.numFilters; i++ ) {
        QListWidgetItem* entry = list->item ( i );
        FILTER f;
        f.filterName = entry->text();
        config.filters.append ( f );
      }
    }
    state.captureWidget->reloadFilters();
  }
}


void
FilterSettings::addEntry ( void )
{
  QListWidgetItem* entry = new QListWidgetItem ( "" );
  entry->setFlags ( entry->flags() | Qt :: ItemIsEditable );
  list->addItem ( entry );
  list->editItem ( entry );
  listAltered = 1;
}


void
FilterSettings::removeEntry ( void )
{
  QListWidgetItem* entry = list->takeItem ( list->currentRow());
  if ( entry ) {
    delete entry;
    listAltered = 1;
  }
}


void
FilterSettings::focusChanged ( QListWidgetItem* curr, QListWidgetItem* old )
{
  Q_UNUSED ( curr )
  if ( old && old->text() == "" ) {
    ( void ) list->takeItem ( list->row ( old ));
    delete old;
    listAltered = 1;
  }
}


void
FilterSettings::itemChanged ( QListWidgetItem* item )
{
  Q_UNUSED ( item )
  listAltered = 1;
}
