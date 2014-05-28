/*****************************************************************************
 *
 * imageWidget.cc -- class for the image controls in the UI
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
#include "imageWidget.h"
#include "controlWidget.h"
#include "state.h"


ImageWidget::ImageWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  outerBox = new QVBoxLayout ( this );
  buttonBox = new QHBoxLayout();
  sizeBox = new QHBoxLayout();

  roi = new QRadioButton ( tr ( "Use ROI" ), this );
  roi->setToolTip ( tr ( "Manually select a region of interest" ));
  max = new QRadioButton ( tr ( "Max Size" ), this );
  max->setToolTip ( tr ( "Set maximum size image frame" ));
  buttonBox->addWidget ( roi );
  buttonBox->addWidget ( max );
  if ( config.useROI ) {
    roi->setChecked ( true );
    max->setChecked ( false );
  } else {
    roi->setChecked ( false );
    max->setChecked ( true );
  }
  buttonGroup = new QButtonGroup ( this );
  buttonGroup->addButton ( roi );
  buttonGroup->addButton ( max );
  buttonGroup->setExclusive ( true );
  connect ( max, SIGNAL( clicked()), this, SLOT( setMaxImageSize()));
  // FIX ME -- add signals

  xSize = new QLineEdit ( this );
  ySize = new QLineEdit ( this );
  x = new QLabel ( " x ", this );

  xSize->setMaxLength ( 4 );
  xSize->setFixedWidth ( 45 );
  ySize->setMaxLength ( 4 );
  ySize->setFixedWidth ( 45 );
  QString xStr, yStr;
  if ( config.imageSizeX > 0 ) {
    xStr = QString::number ( config.imageSizeX );
  } else {
    xStr = "";
    config.imageSizeX = 0;
  }
  xSize->setText ( xStr );
  if ( config.imageSizeY > 0 ) {
    yStr = QString::number ( config.imageSizeY );
  } else {
    yStr = "";
    config.imageSizeY = 0;
  }
  ySize->setText ( yStr );
  // FIX ME -- add signals for text boxes etc.

  ignoreResolutionChanges = 0;
  resMenu = new QComboBox ( this );
  QStringList resolutions;
  resolutions << "640x480" << "1280x960";
  resMenu->addItems ( resolutions );
  connect ( resMenu, SIGNAL( currentIndexChanged ( int )), this,
      SLOT( resolutionChanged ( int )));

  sizeBox->addWidget ( xSize );
  sizeBox->addWidget ( x );
  sizeBox->addWidget ( ySize );
  sizeBox->addWidget ( resMenu );

  setTitle ( tr ( "Image" ));
  outerBox->addLayout ( buttonBox );
  outerBox->addLayout ( sizeBox );

  setLayout ( outerBox );
}


ImageWidget::~ImageWidget()
{
  delete resMenu;
  delete x;
  delete ySize;
  delete xSize;
  delete buttonGroup;
  delete max;
  delete roi;
  delete sizeBox;
  delete buttonBox;
  delete outerBox;
}


void
ImageWidget::configure ( void )
{
  if ( state.camera->hasFixedFrameSizes()) {

    // This includes a particularly ugly way to sort the resolutions using
    // a QMap and some further QMap abuse to be able to find the X and Y
    // resolutions should we not have a setting that matches the config and
    // happen to choose the max size option
    FRAMESIZE* res = state.camera->frameSizes();
    QMap<int,QString> sortMap;
    QMap<int,int> xRes, yRes;
    QString showItemStr;
    int i, firstKey = -1, lastKey = -1;
    for ( i = 0; i < OA_MAX_RESOLUTIONS && res[i].x; i++ ) {
      int numPixels = res[i].x * res[i].y;
      QString resStr = QString::number ( res[i].x ) + "x" +
          QString::number ( res[i].y );
      if ( res[i].x == config.imageSizeX && res[i].y == config.imageSizeY ) {
        showItemStr = resStr;
      }
      sortMap [ numPixels ] = resStr;
      xRes [ numPixels ] = res[i].x;
      yRes [ numPixels ] = res[i].y;
      if ( lastKey < numPixels ) {
        lastKey = numPixels;
      }
      if ( -1 == firstKey || firstKey > numPixels ) {
        firstKey = numPixels;
      }
    }

    int numItems = 0, showItem = -1, showXRes = -1, showYRes = -1;
    QStringList resolutions;
    QMap<int,QString>::iterator j;
    QMap<int,int>::iterator x, y;
    XResolutions.clear();
    YResolutions.clear();
    for ( j = sortMap.begin(), x = xRes.begin(), y = yRes.begin();
        j != sortMap.end(); j++, x++, y++ ) {
      resolutions << *j;
      XResolutions << *x;
      YResolutions << *y;
      if ( *j == showItemStr ) {
        showItem = numItems;
        showXRes = *x;
        showYRes = *y;
      }
      numItems++;
    }

    ignoreResolutionChanges = 1;
    resMenu->clear();
    resMenu->addItems ( resolutions );
    ignoreResolutionChanges = 0;

    if ( showItem >= 0 ) {
      config.imageSizeX = showXRes;
      config.imageSizeY = showYRes;
    } else {
      showItem = max->isChecked() ? numItems - 1: 0;
      if ( showItem ) {
        config.imageSizeX = xRes[ lastKey ];
        config.imageSizeY = yRes[ lastKey ];
      } else {
        config.imageSizeX = xRes[ firstKey ];
        config.imageSizeY = yRes[ firstKey ];
      }
    }
    // There's a gotcha here for cameras that only support a single
    // resolution, as the index won't actually change, and the slot
    // won't get called.  So, we have to get the current index and
    // if it is 0, call resolution changed manually
    if ( resMenu->currentIndex() == showItem ) {
      resolutionChanged ( showItem );
    } else {
      resMenu->setCurrentIndex ( showItem );
    }
    xSize->setText ( QString::number ( config.imageSizeX ));
    ySize->setText ( QString::number ( config.imageSizeY ));
    xSize->setEnabled ( 0 );
    ySize->setEnabled ( 0 );
    if ( 1 == numItems ) {
      max->setEnabled ( 0 );
      roi->setEnabled ( 0 );
    } else {
      max->setEnabled ( 1 );
      roi->setEnabled ( 1 );
    }
  } else {
    fprintf ( stderr, "Camera::hasFixedFrameSizes failed\n" );
  }
}


void
ImageWidget::resolutionChanged ( int index )
{
  // changes to this function may need to be replicated in
  // updateFromConfig()
  if ( !state.camera || ignoreResolutionChanges ||
      !state.camera->isInitialised()) {
    return;
  }

  if ( index == ( resMenu->count() - 1 )) {
    // last item -- max size
    max->setChecked ( true );
  } else {
    // Neither should be checked in this instance
    buttonGroup->setExclusive ( false );
    max->setChecked ( false );
    roi->setChecked ( false );
    buttonGroup->setExclusive ( true );
  }
  config.imageSizeX = XResolutions[ index ];
  config.imageSizeY = YResolutions[ index ];
  xSize->setText ( QString::number ( config.imageSizeX ));
  ySize->setText ( QString::number ( config.imageSizeY ));

  state.camera->delayFrameRateChanges();
  if ( state.controlWidget ) {
    state.controlWidget->updateFrameRates();
  }
  state.camera->setROI ( config.imageSizeX, config.imageSizeY );
  if ( state.previewWidget ) {
    state.previewWidget->updatePreviewSize();
  }
}


void
ImageWidget::enableAllControls ( int state )
{
  // actually we need to save the state if we're disabling because it
  // doesn't make sense to re-enable them all

  bool xSizeState, ySizeState, roiState, maxState, resMenuState;

  if ( state ) {
    xSizeState = xSizeSavedState;
    ySizeState = ySizeSavedState;
    roiState = roiSavedState;
    maxState = maxSavedState;
    resMenuState = resMenuSavedState;
  } else {
    xSizeSavedState = xSize->isEnabled();
    ySizeSavedState = ySize->isEnabled();
    roiSavedState = roi->isEnabled();
    maxSavedState = max->isEnabled();
    resMenuSavedState = resMenu->isEnabled();
    xSizeState = 0;
    ySizeState = 0;
    roiState = 0;
    maxState = 0;
    resMenuState = 0;
  }
  xSize->setEnabled ( xSizeState );
  ySize->setEnabled ( ySizeState );
  max->setEnabled ( maxState );
  roi->setEnabled ( roiState );
  resMenu->setEnabled ( resMenuState );
}


void
ImageWidget::setMaxImageSize ( void )
{
  resMenu->setCurrentIndex ( resMenu->count() - 1 );
}


void
ImageWidget::resetResolution ( void )
{
  resolutionChanged ( resMenu->currentIndex());
}


void
ImageWidget::updateFromConfig ( void )
{
  int numRes = resMenu->count();
  if ( numRes ) {
    int index = 0;
    for ( int i = 0; i < numRes && 0 == index; i++ ) {
      if ( XResolutions[ i ] == config.imageSizeX && YResolutions[ i ] ==
          config.imageSizeY ) {
        index = i;
      }
    }

    if (( numRes - 1 ) == index ) {
      // last item -- max size
      max->setChecked ( true );
    } else {
      // Neither should be checked in this instance
      buttonGroup->setExclusive ( false );
      max->setChecked ( false );
      roi->setChecked ( false );
      buttonGroup->setExclusive ( true );
    }
    // need to ignore changes here because changing the index will
    // call resolutionChanged() and we're doing its work ourselves
    ignoreResolutionChanges = 1;
    resMenu->setCurrentIndex ( index );
    ignoreResolutionChanges = 0;
    xSize->setText ( QString::number ( config.imageSizeX ));
    ySize->setText ( QString::number ( config.imageSizeY ));

    if ( state.camera ) {
      state.camera->delayFrameRateChanges();
    }
    if ( state.controlWidget ) {
      state.controlWidget->updateFrameRates();
    }
    if ( state.camera ) {
      state.camera->setROI ( config.imageSizeX, config.imageSizeY );
    }
    if ( state.previewWidget ) {
      state.previewWidget->updatePreviewSize();
    }
  }
}
