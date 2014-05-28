/*****************************************************************************
 *
 * generalSettings.cc -- class for general settings in the settings UI
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

#include "generalSettings.h"

#include "config.h"
#include "state.h"


GeneralSettings::GeneralSettings ( QWidget* parent ) : QWidget ( parent )
{
  saveBox = new QCheckBox ( "Load and save settings automatically", this );
  saveBox->setChecked ( config.saveSettings );
  box = new QVBoxLayout ( this );
  box->addWidget ( saveBox );
  box->addStretch ( 1 );
  setLayout ( box );
}


GeneralSettings::~GeneralSettings()
{
  delete box;
  delete saveBox;
}


void
GeneralSettings::storeSettings ( void )
{
  config.saveSettings = saveBox->isChecked() ? 1 : 0;
}
