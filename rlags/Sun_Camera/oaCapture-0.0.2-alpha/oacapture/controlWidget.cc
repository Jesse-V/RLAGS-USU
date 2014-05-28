/*****************************************************************************
 *
 * controlWidget.cc -- class for the control widget in the UI
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

extern "C" {
#include "oacam.h"
}

#include "config.h"
#include "controlWidget.h"
#include "state.h"


ControlWidget::ControlWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  gainLabel = new QLabel ( tr ( "Gain" ), this );
  exposureLabel = new QLabel ( tr ( "Exposure (ms)" ), this );
  gammaLabel = new QLabel ( tr ( "Gamma" ), this );
  brightnessLabel = new QLabel ( tr ( "Brightness" ), this );
  framerateLabel = new QLabel ( tr ( "Framerate (fps)" ), this );
  expRangeLabel = new QLabel ( tr ( "Exp. Range (ms)" ), this );

  gainSlider = new QSlider ( Qt::Horizontal, this );
  exposureSlider = new QSlider ( Qt::Horizontal, this );
  gammaSlider = new QSlider ( Qt::Horizontal, this );
  brightnessSlider = new QSlider ( Qt::Horizontal, this );
  framerateSlider = new QSlider ( Qt::Horizontal, this );

  gainSpinbox = new QSpinBox ( this );
  exposureSpinbox = new QSpinBox ( this );
  gammaSpinbox = new QSpinBox ( this );
  brightnessSpinbox = new QSpinBox ( this );
  framerateMenu = new QComboBox ( this );

  expMenu = new QComboBox ( this );
  QStringList exposures;
  exposures << "0 - 5" << "5 - 30" << "30 - 100" << "100 - 1000";
  exposures << "1000 - 10000";
  minSettings[0] = 0;
  minSettings[1] = 5;
  minSettings[2] = 30;
  minSettings[3] = 100;
  maxSettings[0] = 5;
  maxSettings[1] = 30;
  maxSettings[2] = 100;
  maxSettings[3] = 1000;
  expMenu->addItems ( exposures );
  expMenu->setCurrentIndex ( config.exposureMenuOption );
  usingAbsoluteExposure = 0;
  ignoreExposureMenuChanges = 0;
  useExposureDropdown = 1;

  grid = new QGridLayout ( this );
  grid->addWidget ( gainLabel, 0, 0 );
  grid->addWidget ( gammaLabel, 1, 0 );
  grid->addWidget ( brightnessLabel, 2, 0 );
  grid->addWidget ( framerateLabel, 3, 0 );
  grid->addWidget ( exposureLabel, 4, 0 );
  grid->addWidget ( expRangeLabel, 5, 0 );
  
  grid->addWidget ( gainSlider, 0, 1 );
  grid->addWidget ( gammaSlider, 1, 1 );
  grid->addWidget ( brightnessSlider, 2, 1 );
  grid->addWidget ( framerateSlider, 3, 1 );
  grid->addWidget ( exposureSlider, 4, 1 );
  grid->addWidget ( expMenu, 5, 1 );

  grid->addWidget ( gainSpinbox, 0, 2 );
  grid->addWidget ( gammaSpinbox, 1, 2 );
  grid->addWidget ( brightnessSpinbox, 2, 2 );
  grid->addWidget ( framerateMenu, 3, 2 );
  grid->addWidget ( exposureSpinbox, 4, 2 );

  grid->setColumnStretch ( 1, 3 );

  framerateLabel->hide();
  framerateSlider->hide();
  framerateMenu->hide();

  setTitle ( tr ( "Control" ));
  setLayout ( grid );

  gainSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( gainSlider, SIGNAL( sliderMoved ( int )), gainSpinbox,
      SLOT( setValue( int )));
  connect ( gainSlider, SIGNAL( valueChanged ( int )), gainSpinbox,
      SLOT( setValue( int )));
  connect ( gainSpinbox, SIGNAL( valueChanged ( int )), gainSlider,
      SLOT( setValue( int )));
  connect ( gainSpinbox, SIGNAL( valueChanged ( int )), this,
      SLOT( updateGain ( int )));

  exposureSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( exposureSlider, SIGNAL( sliderMoved ( int )), exposureSpinbox,
      SLOT( setValue( int )));
  connect ( exposureSlider, SIGNAL( valueChanged ( int )), exposureSpinbox,
      SLOT( setValue( int )));
  connect ( exposureSpinbox, SIGNAL( valueChanged ( int )), exposureSlider,
      SLOT( setValue( int )));
  connect ( exposureSpinbox, SIGNAL( valueChanged ( int )), this,
      SLOT( updateExposure( int )));

  gammaSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( gammaSlider, SIGNAL( sliderMoved ( int )), gammaSpinbox,
      SLOT( setValue( int )));
  connect ( gammaSlider, SIGNAL( valueChanged ( int )), gammaSpinbox,
      SLOT( setValue( int )));
  connect ( gammaSpinbox, SIGNAL( valueChanged ( int )), gammaSlider,
      SLOT( setValue( int )));
  connect ( gammaSpinbox, SIGNAL( valueChanged ( int )), this,
      SLOT( updateGamma( int )));

  brightnessSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( brightnessSlider, SIGNAL( sliderMoved ( int )), brightnessSpinbox,
      SLOT( setValue( int )));
  connect ( brightnessSlider, SIGNAL( valueChanged ( int )), brightnessSpinbox,
      SLOT( setValue( int )));
  connect ( brightnessSpinbox, SIGNAL( valueChanged ( int )), brightnessSlider,
      SLOT( setValue( int )));
  connect ( brightnessSpinbox, SIGNAL( valueChanged ( int )), this,
      SLOT( updateBrightness( int )));

  framerateSlider->setFocusPolicy ( Qt::TabFocus );
  connect ( framerateSlider, SIGNAL( sliderMoved ( int )), framerateMenu,
      SLOT( setCurrentIndex( int )));
  connect ( framerateSlider, SIGNAL( valueChanged ( int )), framerateMenu,
      SLOT( setCurrentIndex( int )));
  connect ( framerateSlider, SIGNAL( sliderReleased()), this,
      SLOT( frameRateChanged()));
  connect ( framerateMenu, SIGNAL( currentIndexChanged ( int )),
      framerateSlider, SLOT( setValue( int )));
  connect ( framerateMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( frameRateChanged()));

  connect ( expMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( exposureMenuChanged ( int )));

  ignoreFrameRateChanges = 0;
}


ControlWidget::~ControlWidget()
{
  delete grid;
  delete expMenu;
  delete framerateMenu;
  delete brightnessSpinbox;
  delete gammaSpinbox;
  delete exposureSpinbox;
  delete gainSpinbox;
  delete framerateSlider;
  delete brightnessSlider;
  delete gammaSlider;
  delete exposureSlider;
  delete gainSlider;
  delete expRangeLabel;
  delete framerateLabel;
  delete brightnessLabel;
  delete gammaLabel;
  delete exposureLabel;
  delete gainLabel;
}


void
ControlWidget::configure ( void )
{
  int	min, max, step, def;

  // These ones we just want off all the time
  if ( state.camera->hasControl ( OA_CTRL_AUTOGAIN )) {
    state.camera->setControl ( OA_CTRL_AUTOGAIN, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_HUE_AUTO )) {
    state.camera->setControl ( OA_CTRL_HUE_AUTO, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_BRIGHTNESS )) {
    state.camera->setControl ( OA_CTRL_AUTO_BRIGHTNESS, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_EXPOSURE )) {
    state.camera->setControl ( OA_CTRL_AUTO_EXPOSURE, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_GAMMA )) {
    state.camera->setControl ( OA_CTRL_AUTO_GAMMA, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_WHITE_BALANCE )) {
    state.camera->setControl ( OA_CTRL_AUTO_WHITE_BALANCE, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_RED_BALANCE )) {
    state.camera->setControl ( OA_CTRL_AUTO_RED_BALANCE, 0 );
  }
  if ( state.camera->hasControl ( OA_CTRL_AUTO_BLUE_BALANCE )) {
    state.camera->setControl ( OA_CTRL_AUTO_BLUE_BALANCE, 0 );
  }

  if ( state.camera->hasControl ( OA_CTRL_GAIN )) {
    state.camera->controlRange ( OA_CTRL_GAIN, &min, &max, &step, &def );
    if ( config.gainValue  < min || config.gainValue > max ) {
      config.gainValue = def;
    }
    gainSlider->setRange ( min, max );
    gainSlider->setSingleStep ( step );
    gainSlider->setValue ( config.gainValue );
    gainSpinbox->setRange ( min, max );
    gainSpinbox->setSingleStep ( step );
    gainSpinbox->setValue ( config.gainValue );
    gainSlider->show();
    gainSpinbox->show();
  } else {
    gainSlider->hide();
    gainSpinbox->hide();
  }

  if ( state.camera->hasControl ( OA_CTRL_GAMMA )) {
    state.camera->controlRange ( OA_CTRL_GAMMA, &min, &max, &step, &def );
    if ( config.gammaValue  < min || config.gammaValue > max ) {
      config.gammaValue = def;
    }
    gammaSlider->setRange ( min, max );
    gammaSlider->setSingleStep ( step );
    gammaSlider->setValue ( config.gammaValue );
    gammaSpinbox->setRange ( min, max );
    gammaSpinbox->setSingleStep ( step );
    gammaSpinbox->setValue ( config.gammaValue );
    gammaSlider->show();
    gammaSpinbox->show();
  } else {
    gammaSlider->hide();
    gammaSpinbox->hide();
  }

  if ( state.camera->hasControl ( OA_CTRL_BRIGHTNESS )) {
    state.camera->controlRange ( OA_CTRL_BRIGHTNESS, &min, &max, &step, &def );
    if ( config.brightnessValue  < min || config.brightnessValue > max ) {
      config.brightnessValue = def;
    }
    brightnessSlider->setRange ( min, max );
    brightnessSlider->setSingleStep ( step );
    brightnessSlider->setValue ( config.brightnessValue );
    brightnessSpinbox->setRange ( min, max );
    brightnessSpinbox->setSingleStep ( step );
    brightnessSpinbox->setValue ( config.brightnessValue );
    brightnessSlider->show();
    brightnessSpinbox->show();
  } else {
    brightnessSlider->hide();
    brightnessSpinbox->hide();
  }

  // Exposure is a bit more messy.  If we have an absolute exposure time
  // then prefer that.  If we don't, but have some sort of exposure control
  // use that, but the labels will need adjusting.  If we have neither then
  // the controls need to be disabled.
  // At the same time we need to create a new set of labels for the
  // exposure combobox.

  int updateExposureControls = 0;
  int setting;

  if ( state.camera->hasControl ( OA_CTRL_EXPOSURE_ABSOLUTE )) {
    state.camera->controlRange ( OA_CTRL_EXPOSURE_ABSOLUTE, &min, &max, &step,
      &def );
    usingAbsoluteExposure = 1;
    // The settings from libopenastro are supposed to be in units of 100us
    // (because that's the granularity offered by V4L2), so we need to
    // convert those to millisecons by dividing by 10.  This may mean we
    // lose a bit of resolution at the bottom end.  So be it.
    min /= 10;
    if ( min < 1 ) { min = 1; }
    max /= 10;
    if ( max < 1 ) { max = 1; }
    step /= 10;
    if ( step < 1 ) { step = 1; }
    def /= 10;
    if ( def < 1 ) { def = 1; }
    if ( config.exposureAbsoluteValue  < min ||
        config.exposureAbsoluteValue > max ) {
      config.exposureAbsoluteValue = def;
    }
    setting = config.exposureAbsoluteValue;
    updateExposureControls = 1;
    exposureLabel->setText ( tr ( "Exposure (ms)" ));
    expRangeLabel->setText ( tr ( "Exp. Range (ms)" ));
  } else {
    if ( state.camera->hasControl ( OA_CTRL_EXPOSURE )) {
      usingAbsoluteExposure = 0;
      state.camera->controlRange ( OA_CTRL_EXPOSURE, &min, &max, &step, &def );
      if ( config.exposureValue  < min ||
          config.exposureValue > max ) {
        config.exposureValue = def;
      }
      setting = config.exposureValue;
      updateExposureControls = 1;
      exposureLabel->setText ( tr ( "Exposure" ));
      expRangeLabel->setText ( tr ( "Exp. Range" ));
    } else {
      exposureSlider->hide();
      exposureSpinbox->hide();
      expRangeLabel->hide();
      expMenu->hide();
    }
  }

  if ( updateExposureControls ) {

    // now we need to work out the ranges for the combobox
    // if the range is no greater than 256 we hide the combobox and
    // set the slider to the appropriate range.
    // otherwise for non-absolute exposures we divide the range up into
    // five ranges and for absolute have increasing wider scales

    int diff = max - min;
    int showMin = 0, showMax = 0;
    if ( usingAbsoluteExposure || ( diff / step + 1 ) > 256 ) {

      QStringList exposures;
      int v1 = min;
      int v2;
      int items = 0, showItem = 0;
      do {
        if ( usingAbsoluteExposure ) {
          v2 = v1;
          if ( !v2 ) { v2++; }
          v2 *= 10;
          if ( v2 > max ) { v2 = max; }
        } else {
         v2 = min + diff / step / 5 + 1;
        }
        exposures << QString::number ( v1 ) + " - " + QString::number ( v2 );
        if ( setting >= v1 && setting <= v2 ) {
          showMin = v1;
          showMax = v2;
          showItem = items;
        }
        minSettings[ items ] = v1;
        maxSettings[ items ] = v2;
        items++;
        v1 = v2;
        // we have to stop this somewhere...
      } while ( items < 7 && v2 < max );

      // that should be the string list done.  now delete all the old items
      // and add the new string list and set the current item

      ignoreExposureMenuChanges = 1;
      expMenu->clear();
      expMenu->addItems ( exposures );
      config.exposureMenuOption = showItem;
      ignoreExposureMenuChanges = 0;
      expMenu->setCurrentIndex ( showItem );
      expRangeLabel->show();
      expMenu->show();
      useExposureDropdown = 1;
    } else {
      showMin = min;
      showMax = max;
      expRangeLabel->hide();
      expMenu->hide();
      useExposureDropdown = 0;
    }

    // finally we can set the sliders up

    exposureSlider->setRange ( showMin, showMax );
    exposureSlider->setSingleStep ( step );
    exposureSlider->setValue ( setting );
    exposureSpinbox->setRange ( showMin, showMax );
    exposureSpinbox->setSingleStep ( step );
    exposureSpinbox->setValue ( setting );
    exposureSlider->show();
    exposureSpinbox->show();

    if ( usingAbsoluteExposure ) {
      if ( setting ) {
        state.mainWindow->showFPSMaxValue ( 1000 / setting );
      }
    } else {
      state.mainWindow->clearFPSMaxValue();
    }
  }

  updateFrameRates();
}


void
ControlWidget::updateFrameRates ( void )
{
  if ( !state.camera ) {
    return;
  }
  if ( !state.camera->hasFrameRateSupport()) {
    framerateLabel->hide();
    framerateSlider->hide();
    framerateMenu->hide();
    return;
  }

  if ( state.camera->hasFixedFrameRates ( config.imageSizeX,
      config.imageSizeY )) {

    // Ugly sorting of the frame rates here.  We don't know what
    // order we'll get them in so we have to sort them into rate
    // order for the slider to work.

    FRAMERATE* rates = state.camera->frameRates ( config.imageSizeX,
        config.imageSizeY );
    QMap<float,QString> sortMap;
    QMap<float,int> numeratorMap;
    QMap<float,int> denominatorMap;
    int i;
    for ( i = 0; i < OA_MAX_FRAMERATES && rates[i].denominator; i++ ) {
      float frameTime = float ( rates[i].numerator ) /
          float ( rates[i].denominator );
      QString rateStr;
      if ( 1 == rates[i].numerator ) {
        rateStr = QString::number ( rates[i].denominator );
      } else {
        rateStr = QString::number ( rates[i].denominator ) + "/" +
            QString::number ( rates[i].numerator );
      }
      sortMap [ frameTime ] = rateStr;
      numeratorMap [ frameTime ] = rates[i].numerator;
      denominatorMap [ frameTime ] = rates[i].denominator;
    }

    QStringList rateList;
    QMap<float,QString>::iterator j;
    QMap<float,int>::iterator n, d;
    frameRateNumerator.clear();
    frameRateDenominator.clear();
    int numItems = 0, showItem = 0;
    for ( j = sortMap.begin(), n = numeratorMap.begin(),
        d = denominatorMap.begin(); j != sortMap.end(); j++, n++, d++ ) {
      rateList << *j;
      frameRateNumerator << *n;
      frameRateDenominator << *d;
      if ( *n == config.frameRateNumerator &&
          *d == config.frameRateDenominator ) {
        showItem = numItems;
      }
      numItems++;
    }

    ignoreFrameRateChanges = 1;
    framerateMenu->clear();
    framerateMenu->addItems ( rateList );
    framerateSlider->setRange ( 0, numItems - 1 );
    framerateSlider->setSingleStep ( 1 );
    config.frameRateNumerator = frameRateNumerator[ showItem ];
    config.frameRateDenominator = frameRateDenominator[ showItem ];
    ignoreFrameRateChanges = 0;
    framerateMenu->setCurrentIndex ( showItem );
    // unfortunately the above statement may not change the frame rate even
    // though the rate has changed, because its position in the menu might
    // be the same, so we have to do the update manually;
    frameRateChanged();

    framerateLabel->show();
    framerateSlider->show();
    framerateMenu->show();

  } else {
    fprintf ( stderr, "Camera::hasFixedFrameRates failed\n" );
    framerateLabel->hide();
    framerateSlider->hide();
    framerateMenu->hide();
  }
}


void
ControlWidget::exposureMenuChanged ( int index )
{
  if ( ignoreExposureMenuChanges ) {
    return;
  }

  int newMin = minSettings[ index ];
  int newMax = maxSettings[ index ];
  int newSetting;

  if ( usingAbsoluteExposure ) {
    newSetting = config.exposureAbsoluteValue;
  } else {
    newSetting = config.exposureValue;
  }
  if ( newSetting < newMin ) { newSetting = newMin; }
  if ( newSetting > newMax ) { newSetting = newMax; }
  
  exposureSlider->setRange ( newMin, newMax );
  exposureSlider->setValue ( newSetting );
  exposureSpinbox->setRange ( newMin, newMax );
  exposureSpinbox->setValue ( newSetting );

  if ( !state.camera->hasFrameRateSupport()) {
    if ( usingAbsoluteExposure ) {
      theoreticalFPSNumerator = newSetting;
      theoreticalFPSDenominator = 1000;
      theoreticalFPS = 1000 / newSetting;
    } else {
      // we don't really have a clue here, so set it to 1?
      theoreticalFPSNumerator = 1;
      theoreticalFPSDenominator = 1;
      theoreticalFPS = 1;
    }
  }
  if ( usingAbsoluteExposure ) {
    if ( newSetting ) {
      state.mainWindow->showFPSMaxValue ( 1000 / newSetting );
    }
  } else {
    state.mainWindow->clearFPSMaxValue();
  }
}


void
ControlWidget::updateExposure ( int value )
{
  if ( usingAbsoluteExposure ) {
    if ( value ) {
      state.mainWindow->showFPSMaxValue ( 1000 / value );
    }
    state.camera->setControl ( OA_CTRL_EXPOSURE_ABSOLUTE, value );
    config.exposureAbsoluteValue = value;
  } else {
    state.mainWindow->clearFPSMaxValue();
    state.camera->setControl ( OA_CTRL_EXPOSURE, value );
    config.exposureValue = value;
  }
}


void
ControlWidget::frameRateChanged ( void )
{
  if ( ignoreFrameRateChanges ) {
    return;
  }

  int index = framerateSlider->sliderPosition();
  if ( framerateSlider->isSliderDown()) {
    return;
  }

  /*
   * Leaving this in means the first frame setting for a camera breaks
   *
  if ( config.frameRateNumerator == frameRateNumerator[ index ] &&
      config.frameRateDenominator == frameRateDenominator[ index ] ) {
    return;
  }
  */

  config.frameRateNumerator = frameRateNumerator[ index ];
  config.frameRateDenominator = frameRateDenominator[ index ];
  theoreticalFPSNumerator = config.frameRateNumerator;
  theoreticalFPSDenominator = config.frameRateDenominator;
  theoreticalFPS = config.frameRateDenominator /
      config.frameRateNumerator;

  if ( !state.camera ) {
    return;
  }

  state.camera->setFrameInterval ( config.frameRateNumerator,
      config.frameRateDenominator );
}


void
ControlWidget::updateGain ( int value )
{
  config.gainValue = value;
  state.camera->setControl ( OA_CTRL_GAIN, value );
}


void
ControlWidget::updateGamma ( int value )
{
  config.gammaValue = value;
  state.camera->setControl ( OA_CTRL_GAMMA, value );
}


void
ControlWidget::updateBrightness ( int value )
{
  config.brightnessValue = value;
  state.camera->setControl ( OA_CTRL_BRIGHTNESS, value );
}


void
ControlWidget::enableFPSControl ( int state )
{
  framerateSlider->setEnabled ( state );
  framerateMenu->setEnabled ( state );
}


int
ControlWidget::getFPSNumerator ( void )
{
  return theoreticalFPSNumerator;
}


int
ControlWidget::getFPSDenominator ( void )
{
  return theoreticalFPSDenominator;
}


void
ControlWidget::updateFromConfig ( void )
{
  int setFrameRate = -1;
  int min, max, step, def;
  int exposureSetting = -1;

  if ( state.camera ) {
    if ( state.camera->hasControl ( OA_CTRL_GAIN )) {
      state.camera->controlRange ( OA_CTRL_GAIN, &min, &max, &step, &def );
      if ( config.gainValue  < min ) {
        config.gainValue = min;
      }
      if ( config.gainValue  > max ) {
        config.gainValue = max;
      }
    }

    if ( state.camera->hasControl ( OA_CTRL_GAMMA )) {
      state.camera->controlRange ( OA_CTRL_GAMMA, &min, &max, &step, &def );
      if ( config.gammaValue  < min ) {
        config.gammaValue = min;
      }
      if ( config.gammaValue  > max ) {
        config.gammaValue = max;
      }
    }

    if ( state.camera->hasControl ( OA_CTRL_BRIGHTNESS )) {
      state.camera->controlRange ( OA_CTRL_BRIGHTNESS, &min, &max, &step,
          &def );
      if ( config.brightnessValue  < min ) {
        config.brightnessValue = min;
      }
      if ( config.brightnessValue  > max ) {
        config.brightnessValue = max;
      }
    }

    if ( state.camera->hasControl ( OA_CTRL_EXPOSURE_ABSOLUTE )) {
      state.camera->controlRange ( OA_CTRL_EXPOSURE_ABSOLUTE, &min, &max,
          &step, &def );
      min /= 10;
      if ( min < 1 ) { min = 1; }
      max /= 10;
      if ( max < 1 ) { max = 1; }
 
      if ( config.exposureAbsoluteValue  < min ) {
        config.exposureAbsoluteValue = min;
      }
      if ( config.exposureAbsoluteValue > max ) {
        config.exposureAbsoluteValue = max;
      }
      exposureSetting = config.exposureAbsoluteValue;
    } else {
      if ( state.camera->hasControl ( OA_CTRL_EXPOSURE )) {
        state.camera->controlRange ( OA_CTRL_EXPOSURE, &min, &max, &step,
            &def );
        if ( config.exposureValue  < min ) {
          config.exposureValue = min;
        }
        if ( config.exposureValue > max ) {
          config.exposureValue = max;
        }
        exposureSetting = config.exposureAbsoluteValue;
      }
    }

    if ( state.camera->hasFrameRateSupport()) {

      updateFrameRates();

      FRAMERATE* rates = state.camera->frameRates ( config.imageSizeX,
          config.imageSizeY );

      for ( int i = 0; setFrameRate < 0 && i < OA_MAX_FRAMERATES &&
          rates[i].denominator; i++ ) {
        if ( rates[i].numerator == config.frameRateNumerator &&
            rates[i].denominator == config.frameRateDenominator ) {
          setFrameRate = i;
        }
      }
    }
  }

  gainSpinbox->setValue ( config.gainValue );
  gammaSpinbox->setValue ( config.gammaValue );
  brightnessSpinbox->setValue ( config.brightnessValue );

  if ( useExposureDropdown ) {
    int foundDropdownValue = -1;
    int numItems = expMenu->count();
    for ( int i = 0; i < numItems; i++ ) {
      if ( exposureSetting >= minSettings[i] && exposureSetting <=
          maxSettings[i] ) {
        foundDropdownValue = i;
      }
    }
    if ( foundDropdownValue == -1 ) {
      qWarning() << "can't find new exposure setting in dropdown";
    } else {
      expMenu->setCurrentIndex ( foundDropdownValue );
      exposureSlider->setRange ( minSettings[ foundDropdownValue ],
          maxSettings[ foundDropdownValue ] );
      exposureSpinbox->setRange ( minSettings[ foundDropdownValue ],
          maxSettings[ foundDropdownValue ] );
    }
  }
  exposureSpinbox->setValue ( exposureSetting );

  if ( setFrameRate >= 0 ) {
    framerateMenu->setCurrentIndex ( setFrameRate );
  }

  if ( usingAbsoluteExposure ) {
    if ( exposureSetting ) {
      state.mainWindow->showFPSMaxValue ( 1000 / exposureSetting );
    }
  } else {
    state.mainWindow->clearFPSMaxValue();
  }

}
