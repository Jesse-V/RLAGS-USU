/*****************************************************************************
 *
 * settingsWidget.h -- class declaration
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

#include "generalSettings.h"
#include "captureSettings.h"
#include "profileSettings.h"
#include "filterSettings.h"
#include "autorunSettings.h"
#include "histogramSettings.h"

class SettingsWidget : public QWidget
{
  Q_OBJECT

  public:
    			SettingsWidget();
    			~SettingsWidget();
    void		setActiveTab ( int );

  private:
    GeneralSettings*	general;
    CaptureSettings*	capture;
    ProfileSettings*	profiles;
    FilterSettings*	filters;
    AutorunSettings*	autorun;
    HistogramSettings*	histogram;
    QVBoxLayout*	vbox;
    QTabWidget*		tabSet;
    QHBoxLayout*	buttonBox;
    QPushButton*	cancelButton;
    QPushButton*	saveButton;

  public slots:
    void		storeSettings ( void );

};
