/*****************************************************************************
 *
 * histogramSettings.cc -- class for the histogram settings in the settings UI
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

#include "histogramSettings.h"

#include "config.h"
#include "state.h"

HistogramSettings::HistogramSettings ( QWidget* parent ) : QWidget ( parent )
{
  splitBox = new QCheckBox ( tr ( "Split RGB histogram" ), this );
  splitBox->setChecked ( config.splitHistogram );
  box = new QVBoxLayout ( this );
  box->addWidget ( splitBox );
  box->addStretch ( 1 );
  setLayout ( box );
}


HistogramSettings::~HistogramSettings()
{
  delete box;
  delete splitBox;
}


void
HistogramSettings::storeSettings ( void )
{
  config.splitHistogram = splitBox->isChecked() ? 1 : 0;
  if ( state.histogramWidget ) {
    state.histogramWidget->updateLayout();
  }
}
