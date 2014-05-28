/*****************************************************************************
 *
 * controlWidget.h -- class declaration
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

class ControlWidget : public QGroupBox
{
  Q_OBJECT

  public:
    			ControlWidget ( QWidget* parent = 0 );
    			~ControlWidget();
    void		configure();
    void		updateFrameRates ( void );
    void		enableFPSControl ( int );
    int			getFPSNumerator ( void );
    int			getFPSDenominator ( void );
    void		updateFromConfig ( void );

  public slots:
    void		exposureMenuChanged ( int );
    void		frameRateChanged ( void );
    void		updateExposure ( int );
    void		updateGain ( int );
    void		updateGamma ( int );
    void		updateBrightness ( int );

  private:
    QSlider*		gainSlider;
    QSlider*		exposureSlider;
    QSlider*		gammaSlider;
    QSlider*		brightnessSlider;
    QSlider*		framerateSlider;
    QSpinBox*		gainSpinbox;
    QSpinBox*		exposureSpinbox;
    QSpinBox*		gammaSpinbox;
    QSpinBox*		brightnessSpinbox;
    QLabel*		exposureLabel;
    QLabel*		expRangeLabel;
    QLabel*		framerateLabel;
    QComboBox*		expMenu;
    QComboBox*		framerateMenu;
    int			minSettings[8];
    int			maxSettings[8];
    int			usingAbsoluteExposure; // avoids checking camera
    int			ignoreExposureMenuChanges;
    int			ignoreFrameRateChanges;
    QList<int>		frameRateNumerator;
    QList<int>		frameRateDenominator;
    QLabel*		gainLabel;
    QLabel*		gammaLabel;
    QLabel*		brightnessLabel;
    QGridLayout*	grid;
    int			theoreticalFPSNumerator;
    int			theoreticalFPSDenominator;
    float		theoreticalFPS;
    int			useExposureDropdown;
};
