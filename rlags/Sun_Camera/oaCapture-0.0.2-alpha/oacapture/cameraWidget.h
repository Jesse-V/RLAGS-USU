/*****************************************************************************
 *
 * cameraWidget.h -- class declaration
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

extern "C" {
#include "oacam.h"
}

#include "config.h"

class CameraWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			CameraWidget ( QWidget* parent = 0 );
    			~CameraWidget();
    void                configure ( void );
    void                enableBinningControl ( int );
    void                updateFromConfig ( void );

  private:
    QCheckBox*		sixteenBit;
    QCheckBox*		binning2x2;
    QCheckBox*		preview;
    QCheckBox*		nightMode;
    QVBoxLayout*	box;

  public slots:
    void		setBinning ( int );
    void		set16Bit ( int );
};
