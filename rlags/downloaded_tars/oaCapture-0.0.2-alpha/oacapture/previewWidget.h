/*****************************************************************************
 *
 * previewWidget.h -- class declaration
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
#include <stdint.h>

extern "C" {
#include "oacam.h"
}

#include "config.h"

class PreviewWidget : public QFrame
{
  Q_OBJECT

  public:
    			PreviewWidget ( QWidget* parent = 0 );
    			~PreviewWidget();
    void		updatePreviewSize ( void );
    void		configure ( void );
    void		setBuffer ( QImage* );
    void		setCapturedFramesDisplayInterval ( int );
    void		setEnabled ( int );
    void		setVideoFramePixelFormat ( int );
    void		enableTempDisplay ( int );
    void		enableFlipX ( int );
    void		enableFlipY ( int );

  public slots:
    void		updatePreview ( void*, int );

  protected:
    void		paintEvent ( QPaintEvent* );

  private:
    QImage*		image;
    int			currentZoom;
    int			currentZoomX;
    int			currentZoomY;
    int			capturedFramesDisplayInterval;
    unsigned long	lastCapturedFramesUpdateTime;
    int			frameDisplayInterval;
    unsigned long	lastDisplayUpdateTime;
    int			previewEnabled;
    int			videoFramePixelFormat;
    unsigned int	framesInLastSecond;
    long		secondForFrameCount;
    long		secondForTemperature;
    int			hasTemp;
    int			reticleCentreX;
    int			reticleCentreY;
    int			flipX;
    int			flipY;
    void*		previewImageBuffer;
    int			previewBufferLength;
    void*		writeImageBuffer;
    int			writeBufferLength;

    void		processFlip ( void*, int );
    void		processFlip8Bit ( uint8_t*, int );
    void		processFlip16Bit ( uint8_t*, int );
    void		processFlip24BitColour ( uint8_t*, int );
    void		convert16To8Bit ( void*, int );

    QVector<QRgb>	greyscaleColourTable;
};
