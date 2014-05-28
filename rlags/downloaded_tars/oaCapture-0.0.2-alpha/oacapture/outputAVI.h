/*****************************************************************************
 *
 * outputAVI.h -- class declaration
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

extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

#include "outputHandler.h"

class OutputAVI : public OutputHandler
{
  public:
    			OutputAVI ( int, int, int, int, int );
    			~OutputAVI();
    int			openOutput ( void );
    int			addFrame ( void* );
    int			outputExists ( void );
    int			outputWritable ( void );
    void		closeOutput();

  private:
    AVStream*		addVideoStream ( AVFormatContext*, enum AVCodecID );
    int			openVideo ( AVStream* );
    void		closeVideo ( AVStream* );
    AVFrame*		allocatePicture ( enum PixelFormat, int, int );

    AVOutputFormat*	outputFormat;
    AVFormatContext*	formatContext;
    AVStream*		videoStream;
    AVFrame*		picture;
    int			frameSize;
    int			videoOutputBufferSize;
    uint8_t*		videoOutputBuffer;
    int			xSize;
    int			ySize;
    int			fpsNumerator;
    int			fpsDenominator;
    enum AVPixelFormat	actualPixelFormat;
    enum AVPixelFormat	storedPixelFormat;
};
