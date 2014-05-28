/*****************************************************************************
 *
 * cameraWidget.cc -- class for the camera widget in the UI
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
#include "cameraWidget.h"
#include "controlWidget.h"
#include "state.h"


CameraWidget::CameraWidget ( QWidget* parent ) : QGroupBox ( parent )
{
  preview = new QCheckBox ( tr ( "Preview" ), this );
  preview->setToolTip ( tr ( "Turn preview on/off" ));
  preview->setChecked ( config.preview );
  connect ( preview, SIGNAL( stateChanged ( int )), parent,
      SLOT( changePreviewState ( int )));

  nightMode = new QCheckBox ( tr ( "Night Mode" ), this );
  nightMode->setToolTip ( tr ( "Turn night colours on/off" ));
  connect ( nightMode, SIGNAL( stateChanged ( int )), state.mainWindow,
      SLOT( setNightMode ( int )));
  nightMode->setChecked ( config.nightMode );

  sixteenBit = new QCheckBox ( tr ( "16-bit" ), this );
  sixteenBit->setToolTip ( tr ( "Capture in 16-bit mode" ));
  sixteenBit->setChecked ( config.sixteenBit );
  connect ( sixteenBit, SIGNAL( stateChanged ( int )), this,
      SLOT( set16Bit ( int )));

  binning2x2 = new QCheckBox ( tr ( "2x2 Binning" ), this );
  binning2x2->setToolTip ( tr ( "Enable 2x2 binning in camera" ));
  binning2x2->setChecked ( config.binning2x2 );
  connect ( binning2x2, SIGNAL( stateChanged ( int )), this,
      SLOT( setBinning ( int )));

  setTitle ( tr ( "Camera" ));
  box = new QVBoxLayout;
  box->addWidget ( preview );
  box->addWidget ( nightMode );
  box->addWidget ( sixteenBit );
  box->addWidget ( binning2x2 );
  setLayout ( box );
}


CameraWidget::~CameraWidget()
{
  delete box;
  delete binning2x2;
  delete sixteenBit;
  delete nightMode;
  delete preview;
}


void
CameraWidget::configure ( void )
{
  sixteenBit->setEnabled ( state.camera->has16Bit() ? 1 : 0 );
  binning2x2->setEnabled ( state.camera->hasBinning ( 2 ) ? 1 : 0 );
}


void
CameraWidget::setBinning ( int newState )
{
  if ( newState == Qt::Unchecked ) {
    state.camera->setControl ( OA_CTRL_BINNING, 1 );
    config.binning2x2 = 0;
  } else {
    state.camera->setControl ( OA_CTRL_BINNING, 2 );
    config.binning2x2 = 1;
  }
  state.imageWidget->configure();
  state.imageWidget->resetResolution();
}


void
CameraWidget::enableBinningControl ( int state )
{
  binning2x2->setEnabled ( state );
}


void
CameraWidget::set16Bit ( int newState )
{
  if ( state.camera ) {
    int format;
    if ( newState == Qt::Unchecked ) {
      state.camera->setBitDepth ( 8 );
      config.sixteenBit = 0;
    } else {
      state.camera->setBitDepth ( 16 );
      config.sixteenBit = 1;
    }
    format = state.camera->getPixelFormatForBitDepth ( 0 );
    state.previewWidget->setVideoFramePixelFormat ( format );
  }
  return;
}


void
CameraWidget::updateFromConfig ( void )
{
  if ( sixteenBit->isEnabled()) {
    sixteenBit->setChecked ( config.sixteenBit );
  }
  if ( binning2x2->isEnabled()) {
    binning2x2->setChecked ( config.binning2x2 );
  }
}
