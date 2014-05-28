/*****************************************************************************
 *
 * outputAVI.cc -- AVI output class
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <QtGui>

extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

#include "outputHandler.h"
#include "outputAVI.h"
#include "state.h"


OutputAVI::OutputAVI ( int x, int y, int n, int d, int fmt ) :
    OutputHandler ( x, y, n, d )
{
  if ( !state.libavStarted ) {
    av_register_all();
    av_log_set_level ( AV_LOG_QUIET );
    state.libavStarted = 1;
  }

  outputFormat = 0;
  formatContext = 0;
  videoStream = 0;
  frameCount = 0;
  xSize = x;
  ySize = y;
  fpsNumerator = n;
  fpsDenominator = d;
  storedPixelFormat = AV_PIX_FMT_RGB24;
  actualPixelFormat = AV_PIX_FMT_RGB24;
  switch ( fmt ) {
    case OA_PIX_FMT_BGR24:
      actualPixelFormat = AV_PIX_FMT_BGR24;
      break;
    case OA_PIX_FMT_GREY8:
      actualPixelFormat = AV_PIX_FMT_GRAY8;
      storedPixelFormat = AV_PIX_FMT_GRAY8;
      break;
    case OA_PIX_FMT_GREY16LE:
      actualPixelFormat = AV_PIX_FMT_GRAY16LE;
      storedPixelFormat = AV_PIX_FMT_GRAY16LE;
      break;
    case OA_PIX_FMT_GREY16BE:
      actualPixelFormat = AV_PIX_FMT_GRAY16BE;
      storedPixelFormat = AV_PIX_FMT_GRAY16BE;
      break;
  }

  fullSaveFilePath = "";
}


OutputAVI::~OutputAVI()
{
  // Probably nothing to do here for AVI files
}


int
OutputAVI::outputExists ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".avi";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), F_OK )) ? 0 : 1;
}


int
OutputAVI::outputWritable ( void )
{
  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".avi";
  }

  // FIX ME -- what if this returns an error?
  return ( access ( fullSaveFilePath.toStdString().c_str(), W_OK )) ? 0 : 1;
}


int
OutputAVI::openOutput ( void )
{
  int		e;
  char		errbuf[100];

  if ( fullSaveFilePath == "" ) {
    filenameRoot = getFilename();
    fullSaveFilePath = filenameRoot + ".avi";
  }

  if (!( outputFormat = av_guess_format ( "avi", 0, 0 ))) {
    qWarning() << "av_guess_format failed";
    return -1;
  }

  if (!( formatContext = avformat_alloc_context())) {
    qWarning() << "avformat_alloc_context failed";
    return -1;
  }

  formatContext->oformat = outputFormat;
  snprintf ( formatContext->filename, sizeof ( formatContext->filename ),
    "%s", fullSaveFilePath.toStdString().c_str());

  enum AVCodecID videoCodec;
  if ( AV_PIX_FMT_GRAY8 == storedPixelFormat ||
      AV_PIX_FMT_GRAY16BE == storedPixelFormat ||
      AV_PIX_FMT_GRAY16LE == storedPixelFormat ) {
    videoCodec = AV_CODEC_ID_RAWVIDEO;
  } else {
    videoCodec = AV_CODEC_ID_UTVIDEO;
  }
  if (!( videoStream = addVideoStream ( formatContext, videoCodec ))) {
    qWarning() << "add video stream failed";
    return -1;
  }

  av_dump_format ( formatContext, 0,
      fullSaveFilePath.toStdString().c_str(), 1 );

  if ( openVideo ( videoStream )) {
    qWarning() << "open video stream failed";
    return -1;
  }

  if (( e = avio_open ( &formatContext->pb,
      fullSaveFilePath.toStdString().c_str(), AVIO_FLAG_WRITE )) < 0 ) {
    av_strerror( e, errbuf, sizeof(errbuf));
    qWarning() << "open of " << fullSaveFilePath << " failed, error = " <<
        errbuf;
    return -1;
  }

  avformat_write_header ( formatContext, 0 );

  return 0;
}


int
OutputAVI::addFrame ( void* frame )
{
  AVCodecContext* codecContext = videoStream->codec;

  if ( actualPixelFormat != storedPixelFormat ) {
    if ( AV_PIX_FMT_BGR24 == actualPixelFormat ) {
      // Quick hack to swap the R and B bytes...
      uint8_t* t = picture->data[0];
      uint8_t* s = ( uint8_t*) frame;
      int l = 0;
      while ( l < frameSize ) {
        *t++ = *( s + 2 );
        *t++ = *( s + 1 );
        *t++ = *s;
        s += 3;
        l += 3;
      }
    } else {
      qWarning() << "unsupported pixel format in addFrame";
    }
  } else {
    memcpy ( picture->data[0], ( uint8_t* ) frame, frameSize );
  }

  int outputSize = avcodec_encode_video ( codecContext, videoOutputBuffer,
      videoOutputBufferSize, picture );
  int ret = 0;
  if ( outputSize > 0 ) {
    AVPacket packet;
    av_init_packet ( &packet );
    if ( codecContext->coded_frame->pts != AV_NOPTS_VALUE ) {
      packet.pts = av_rescale_q ( codecContext->coded_frame->pts,
          codecContext->time_base, videoStream->time_base );
    }
    packet.flags |= AV_PKT_FLAG_KEY;
    packet.stream_index = videoStream->index;
    packet.data = videoOutputBuffer;
    packet.size = outputSize;
    ret = av_write_frame ( formatContext, &packet );
  }
  if ( ret ) {
    qWarning() << "av_write_frame failed";
  }
  frameCount++;
  return ret;
}


void
OutputAVI::closeOutput ( void )
{
  av_write_trailer ( formatContext );
  closeVideo ( videoStream );

  for ( unsigned int i = 0; i < formatContext->nb_streams; i++ ) {
    av_freep ( &formatContext->streams[i]->codec );
    av_freep ( &formatContext->streams[i] );
  }

  avio_close ( formatContext->pb );
  av_free ( formatContext );

  formatContext = 0;
  outputFormat = 0;
  videoStream = 0;
}


AVStream*
OutputAVI::addVideoStream ( AVFormatContext* formatContext,
    enum AVCodecID codecId )
{
  AVCodecContext*	codecContext;
  AVStream*		stream;

  if (!( stream = avformat_new_stream ( formatContext, 0 ))) {
    qWarning() << "avformat_new_stream failed";
    return 0;
  }

  codecContext = stream->codec;
  codecContext->codec_id = codecId;
  codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
  codecContext->width = xSize;
  codecContext->height = ySize;
  codecContext->time_base.num = fpsNumerator;
  codecContext->time_base.den = fpsDenominator;
  codecContext->gop_size = 0;
  codecContext->pix_fmt = storedPixelFormat;

  if ( formatContext->flags & AVFMT_GLOBALHEADER ) {
    codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
  }

  return stream;
}


int
OutputAVI::openVideo ( AVStream* stream )
{
  AVCodecContext* codecContext = stream->codec;
  AVCodec* codec = avcodec_find_encoder ( codecContext->codec_id );
  if ( !codec ) {
    qWarning() << "codec not found" << codecContext->codec_id;
    return -1;
  }

  if ( avcodec_open ( codecContext, codec ) < 0 ) {
    qWarning() << "couldn't open codec";
    return -1;
  }

  videoOutputBufferSize = 5 * xSize * ySize;
  videoOutputBuffer = ( uint8_t* ) av_malloc ( videoOutputBufferSize );

  if (!( picture = allocatePicture ( codecContext->pix_fmt,
      codecContext->width, codecContext->height ))) {
    return -1;
  }

  return 0;
}


void
OutputAVI::closeVideo ( AVStream* stream )
{
  avcodec_close ( stream->codec );

  if ( videoOutputBuffer ) {
    av_free ( videoOutputBuffer );
    videoOutputBuffer = 0;
  }

  if ( picture ) {
    av_free ( picture->data[0] );
    av_free ( picture );
    picture = 0;
  }
}


AVFrame*
OutputAVI::allocatePicture ( enum PixelFormat format, int width, int height )
{
  AVFrame* picture = avcodec_alloc_frame();
  if ( !picture ) {
    qWarning() << "avcodec_alloc_frame failed";
    return 0;
  }
  frameSize = avpicture_get_size ( format, width, height );
  uint8_t* pictureBuffer = ( uint8_t* ) av_malloc ( frameSize );
  if ( !pictureBuffer ) {
    qWarning() << "pictureBuffer av_malloc failed";
    av_free ( picture );
    return 0;
  }

  avpicture_fill (( AVPicture* ) picture, pictureBuffer, format, width,
    height );
  return picture;
}
