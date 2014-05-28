/*****************************************************************************
 *
 * previewWidget.cc -- class for the preview window in the UI (and more)
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
#include <stdint.h>

extern "C" {
#include "oacam.h"
}

#include "config.h"
#include "previewWidget.h"
#include "outputHandler.h"
#include "histogramWidget.h"
#include "state.h"


// FIX ME -- Lots of this stuff needs refactoring or placing elsewhere
// as it's really not anything to do with the actual preview window
// any more

PreviewWidget::PreviewWidget ( QWidget* parent ) : QFrame ( parent )
{
  currentZoom = 100;
  int zoomFactor = state.zoomWidget->getZoomFactor();
  currentZoomX = config.imageSizeX * zoomFactor / 100;
  currentZoomY = config.imageSizeY * zoomFactor / 100;
  setGeometry ( 0, 0, currentZoomX, currentZoomY );
  // setAttribute( Qt::WA_NoSystemBackground, true );
  connect ( state.camera, SIGNAL( frameGrabbed ( void*, int )), this,
    SLOT ( updatePreview ( void*, int )));
  image = 0;
  lastCapturedFramesUpdateTime = 0;
  capturedFramesDisplayInterval = 200;
  lastDisplayUpdateTime = 0;
  frameDisplayInterval = 1000/15; // display frames per second
  previewEnabled = 1;
  videoFramePixelFormat = OA_PIX_FMT_RGB24;
  framesInLastSecond = 0;
  secondForFrameCount = 0;
  secondForTemperature = 0;
  flipX = 0;
  flipY = 0;
  previewImageBuffer = writeImageBuffer = 0;

  for ( int i = 0; i < 256; i++ ) {
    greyscaleColourTable.push_back ( QColor ( i, i, i ).rgb());
  }
}


PreviewWidget::~PreviewWidget()
{
  if ( previewImageBuffer ) {
    free ( previewImageBuffer );
  }
  if ( writeImageBuffer ) {
    free ( writeImageBuffer );
  }
  if ( image ) {
    delete image;
  }
}


void
PreviewWidget::updatePreview ( void* imageData, int length )
{
  // assign the temporary buffers for image transforms if they
  // don't already exist

  if ( !previewImageBuffer ) {
    previewBufferLength = config.imageSizeX * config.imageSizeY * 3;
    if (!( previewImageBuffer = malloc ( previewBufferLength ))) {
      qWarning() << "preview image buffer malloc failed";
      return;
    }
  }
  if ( !writeImageBuffer ) {
    writeBufferLength = config.imageSizeX * config.imageSizeY * 3;
    if (!( writeImageBuffer = malloc ( writeBufferLength ))) {
      qWarning() << "write image buffer malloc failed";
      return;
    }
  }

  if ( image ) {
    delete image;
    image = 0;
  }

  // FIX ME -- this is not the right way to fix the problem of this
  // code executing after the camera has been disconnected.  The race
  // condition remains, but the window is much smaller.
  // Probably should copy the data to a buffer that doesn't get removed
  // by the camera class, but then we have to deal with resizing on
  // resolution changes etc.

  if ( state.camera->captureRunning()) {
    struct timeval	t;
    int			doDisplay = 0;
    int			doHistogram = 0;

    // write straight from the data if possible
    void* previewBuffer = imageData;
    void* writeBuffer = imageData;

    // do a vertical/horizontal flip if required
    if ( flipX || flipY ) {
      ( void ) memcpy ( writeImageBuffer, imageData, length );
      processFlip ( writeImageBuffer, length );
      // both preview and write will come from this buffer for the
      // time being.  This may change later on
      previewBuffer = writeImageBuffer;
      writeBuffer = writeImageBuffer;
    }

    int reducedGreyscaleBitDepth = 0;
    if ( OA_PIX_FMT_GREY16BE == videoFramePixelFormat ||
        OA_PIX_FMT_GREY16LE == videoFramePixelFormat ) {
      // if we're not already using the previewImageBuffer for preview we
      // need to copy the data to it from whatever we were going to use
      if ( previewBuffer != previewImageBuffer ) {
        ( void ) memcpy ( previewImageBuffer, previewBuffer, length );
      }
      convert16To8Bit ( previewImageBuffer, length );
      previewBuffer = previewImageBuffer;
      reducedGreyscaleBitDepth = 1;
    }

    ( void ) gettimeofday ( &t, 0 );
    unsigned long now = ( unsigned long ) t.tv_sec * 1000 +
        ( unsigned long ) t.tv_usec / 1000;

    if ( previewEnabled ) {
      if (( lastDisplayUpdateTime + frameDisplayInterval ) < now ) {
        lastDisplayUpdateTime = now;
        doDisplay = 1;

        QImage* newImage;
        QImage* swappedImage = 0;
        if ( OA_PIX_FMT_GREY8 == videoFramePixelFormat ||
            reducedGreyscaleBitDepth ) {
          newImage = new QImage (( uint8_t* ) previewBuffer, config.imageSizeX,
              config.imageSizeY, config.imageSizeX, QImage::Format_Indexed8 );
          newImage->setColorTable ( greyscaleColourTable );
          swappedImage = newImage;
        } else {
          newImage = new QImage (( const uint8_t* ) previewBuffer,
              config.imageSizeX, config.imageSizeY, QImage::Format_RGB888 );
          if ( OA_PIX_FMT_BGR24 == videoFramePixelFormat ) {
            swappedImage = new QImage ( newImage->rgbSwapped());
          } else {
            swappedImage = newImage;
          }
        }

        int zoomFactor = state.zoomWidget->getZoomFactor();
        if ( zoomFactor && zoomFactor != currentZoom ) {
        currentZoom = zoomFactor;
          currentZoomX = config.imageSizeX * zoomFactor / 100;
          currentZoomY = config.imageSizeY * zoomFactor / 100;
          setGeometry ( 0, 0, currentZoomX, currentZoomY );
        }
        // FIX ME -- optimise this 100% check again
//      if ( currentZoom != 100 ) {
          QImage scaledImage = swappedImage->scaled ( currentZoomX,
            currentZoomY );
          image = new QImage ( scaledImage );
          if ( swappedImage != newImage ) {
            delete swappedImage;
          }
          delete newImage;
//      } else {
//        image = newImage;
//      }
      }
    }

    OutputHandler* output = state.captureWidget->getOutputHandler();
    if ( output ) {
      output->addFrame ( writeBuffer );
      if (( lastCapturedFramesUpdateTime + capturedFramesDisplayInterval ) <
          now ) {
        state.mainWindow->setCapturedFrames ( output->getFrameCount());
        lastCapturedFramesUpdateTime = now;
      }
    }

    state.camera->releaseImageData();

    framesInLastSecond++;
    if ( t.tv_sec != secondForFrameCount ) {
      secondForFrameCount = t.tv_sec;
      state.mainWindow->setActualFrameRate ( framesInLastSecond );
      framesInLastSecond = 0;
      if ( state.histogramOn ) {
        state.histogramWidget->process ( writeBuffer, length,
            videoFramePixelFormat );
        doHistogram = 1;
      }
    }

    if ( hasTemp && t.tv_sec != secondForTemperature && t.tv_sec % 5 == 0 ) {
      state.mainWindow->setTemperature ( state.camera->getTemperature());
      secondForTemperature = t.tv_sec;
    }

    if ( doDisplay ) {
      update();
    }

    // check histogram control here just in case it got changed
    // this ought to be done rather more safely
    if ( state.histogramOn && state.histogramWidget && doHistogram ) {
      state.histogramWidget->update();
    }

    if ( output && config.limitEnabled ) {
      // start and current times here are in ms, but the limit value is in
      // secs, so rather than ( current - start ) / time * 100 to get the
      // %age, we do ( current - start ) / time / 10
      state.mainWindow->setProgress (( now -
          state.captureWidget->recordingStartTime ) / config.limitValue / 10 );
      if ( now > state.captureWidget->recordingEndTime ) {
        state.captureWidget->doStopRecording();
        if ( state.autorunEnabled ) {
          // returns non-zero if more runs are left
          if ( state.captureWidget->singleAutorunFinished()) {
            state.autorunStartNext = now + 1000 * config.autorunDelay;
          }
        }
      }
    }

    if ( state.autorunEnabled && state.autorunStartNext &&
        now > state.autorunStartNext ) {
      state.autorunStartNext = 0;
      state.captureWidget->startNewAutorun();
    }
  }
}


void
PreviewWidget::setBuffer ( QImage* buffer )
{
  image = buffer;
}


void
PreviewWidget::configure ( void )
{
  setGeometry ( 0, 0, config.imageSizeX, config.imageSizeY );
}


void
PreviewWidget::updatePreviewSize ( void )
{
  int zoomFactor = state.zoomWidget->getZoomFactor();
  currentZoom = zoomFactor;
  currentZoomX = config.imageSizeX * zoomFactor / 100;
  currentZoomY = config.imageSizeY * zoomFactor / 100;
  reticleCentreX = currentZoomX / 2;
  reticleCentreY = currentZoomY / 2;
  setGeometry ( 0, 0, currentZoomX, currentZoomY );

  int newBufferLength = config.imageSizeX * config.imageSizeY * 3;
  if ( newBufferLength > previewBufferLength ) {
    if (!( previewImageBuffer = realloc ( previewImageBuffer,
        newBufferLength ))) {
      qWarning() << "preview image buffer realloc failed";
    }
    previewBufferLength = newBufferLength;
  }
}


void
PreviewWidget::paintEvent ( QPaintEvent* event )
{
  Q_UNUSED( event );

  if ( image ) {
/*
    QPainter painter;
    painter.begin ( this );
    painter.drawImage ( 0, 0, *image );
    painter.end();
*/
    QPainter painter ( this );
    painter.drawImage ( 0, 0, *image );

    if ( config.showReticle ) {
      painter.setRenderHint ( QPainter::Antialiasing, false );
      painter.setPen ( QPen ( Qt::red, 2, Qt::SolidLine, Qt::FlatCap ));
      painter.drawEllipse ( reticleCentreX - 50, reticleCentreY - 50,
        100, 100 );
      painter.drawEllipse ( reticleCentreX - 100, reticleCentreY - 100,
        200, 200 );
      painter.drawEllipse ( reticleCentreX - 200, reticleCentreY - 200,
        400, 400 );
      painter.drawLine ( reticleCentreX, reticleCentreY - 5,
        reticleCentreX, reticleCentreY + 5 );
      painter.drawLine ( reticleCentreX - 5, reticleCentreY,
        reticleCentreX + 5, reticleCentreY );
    }
  }
}


void
PreviewWidget::setCapturedFramesDisplayInterval ( int millisecs )
{
  capturedFramesDisplayInterval = millisecs;
}


void
PreviewWidget::setEnabled ( int state )
{
  previewEnabled = state;
}


void
PreviewWidget::setVideoFramePixelFormat ( int format )
{
  videoFramePixelFormat = format;
}


void
PreviewWidget::enableTempDisplay ( int state )
{
  hasTemp = state;
}


void
PreviewWidget::enableFlipX ( int state )
{
  flipX = state;
}


void
PreviewWidget::enableFlipY ( int state )
{
  flipY = state;
}


void
PreviewWidget::processFlip ( void* imageData, int length )
{
  uint8_t* data = ( uint8_t* ) imageData;

  switch ( videoFramePixelFormat ) {
    case OA_PIX_FMT_GREY8:
      processFlip8Bit ( data, length );
      break;
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_GREY16LE:
      processFlip16Bit ( data, length );
      break;
    case OA_PIX_FMT_RGB24:
    case OA_PIX_FMT_BGR24:
      processFlip24BitColour ( data, length );
      break;
    default:
      qWarning() << __FUNCTION__ << " unable to flip format " <<
          videoFramePixelFormat;
      break;
  }
}


void
PreviewWidget::processFlip8Bit ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 1;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2-- = s;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX;
        p2 = p1 + config.imageSizeX - 1;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2-- = s;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( int y = config.imageSizeY - 1; y > config.imageSizeY / 2; y-- ) {
        p2 = imageData + y * config.imageSizeX;
        for ( int x = 0; x < config.imageSizeX; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::processFlip16Bit ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 2;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 2;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX;
        p2 = p1 + ( config.imageSizeX - 1 ) * 2;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 2;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( int y = config.imageSizeY - 1; y > config.imageSizeY / 2; y-- ) {
        p2 = imageData + y * config.imageSizeX * 2;
        for ( int x = 0; x < config.imageSizeX * 2; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::processFlip24BitColour ( uint8_t* imageData, int length )
{
  if ( flipX && flipY ) {
    uint8_t* p1 = imageData;
    uint8_t* p2 = imageData + length - 3;
    uint8_t s;
    while ( p1 < p2 ) {
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2++ = s;
      s = *p1;
      *p1++ = *p2;
      *p2 = s;
      p2 -= 5;
    }
  } else {
    if ( flipX ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      for ( int y = 0; y < config.imageSizeY; y++ ) {
        p1 = imageData + y * config.imageSizeX * 3;
        p2 = p1 + ( config.imageSizeX - 1 ) * 3;
        while ( p1 < p2 ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
          s = *p1;
          *p1++ = *p2;
          *p2 = s;
          p2 -= 5;
        }
      }
    }
    if ( flipY ) {
      uint8_t* p1;
      uint8_t* p2;
      uint8_t s;
      p1 = imageData;
      for ( int y = config.imageSizeY - 1; y > config.imageSizeY / 2; y-- ) {
        p2 = imageData + y * config.imageSizeX * 3;
        for ( int x = 0; x < config.imageSizeX * 3; x++ ) {
          s = *p1;
          *p1++ = *p2;
          *p2++ = s;
        }
      }
    }
  }
}


void
PreviewWidget::convert16To8Bit ( void* imageData, int length )
{
  uint8_t* t = ( uint8_t* ) imageData;
  uint8_t* s = ( uint8_t* ) imageData;

  if ( OA_PIX_FMT_GREY16LE == videoFramePixelFormat ) {
    s++;
  }
  for ( int i = 0; i < length; i += 2, s += 2 ) {
    *t++ = *s;
  }
}
