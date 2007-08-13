// Filename: ffmpegVideo.cxx
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifdef HAVE_FFMPEG

#include "ffmpegVideo.h"
#include "config_movies.h"
#include "avcodec.h"
#include "avformat.h"

TypeHandle FfmpegVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
FfmpegVideo(const Filename &name) :
  MovieVideo(name),
  _filename(name),
  _samples_read(0),
  _format_ctx(0),
  _video_index(-1),
  _audio_index(-1),
  _video_ctx(0),
  _audio_ctx(0),
  _frame(0),
  _frame_out(0),
  _time_correction_next(0)
{
  string osname = _filename.to_os_specific();
  if (av_open_input_file(&_format_ctx, osname.c_str(), NULL, 0, NULL)!=0) {
    cleanup();
    return;
  }
  
  if (av_find_stream_info(_format_ctx)<0) {
    cleanup();
    return;
  }
  
  // Find the video and audio streams
  for(int i=0; i<_format_ctx->nb_streams; i++) {
    if(_format_ctx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      _video_index = i;
      _video_ctx = _format_ctx->streams[i]->codec;
      _video_timebase = av_q2d(_video_ctx->time_base);
    }
    if(_format_ctx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) {
      _audio_index = i;
      _audio_ctx = _format_ctx->streams[i]->codec;
      _audio_timebase = av_q2d(_audio_ctx->time_base);
    }
  }
  
  if (_video_ctx == 0) {
    cleanup();
    return;
  }

  AVCodec *pVideoCodec=avcodec_find_decoder(_video_ctx->codec_id);
  if(pVideoCodec == 0) {
    cleanup();
    return;
  }
  if(avcodec_open(_video_ctx, pVideoCodec)<0) {
    cleanup();
    return;
  }

  _size_x = _video_ctx->width;
  _size_y = _video_ctx->height;
  _num_components = (_video_ctx->pix_fmt==PIX_FMT_RGBA32) ? 4:3;
  
  if (_audio_ctx) {
    AVCodec *pAudioCodec=avcodec_find_decoder(_audio_ctx->codec_id);
    if (pAudioCodec == 0) {
      _audio_ctx = 0;
      _audio_index = -1;
    } else {
      if (avcodec_open(_audio_ctx, pAudioCodec)<0) {
        _audio_ctx = 0;
        _audio_index = -1;
      }
    }
  }
  
  _length = (_format_ctx->duration * 1.0) / AV_TIME_BASE;
  _can_seek = true;
  _can_seek_zero = true;

  memset(_time_corrections, 0, sizeof(_time_corrections));
  
  _frame = avcodec_alloc_frame();
  _frame_out = avcodec_alloc_frame();
  if ((_frame == 0)||(_frame_out == 0)) {
    cleanup();
    return;
  }
  
  read_ahead();
  _next_start = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Destructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
~FfmpegVideo() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::cleanup
//       Access: Public
//  Description: Reset to a standard inactive state.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
cleanup() {
  if (_format_ctx) {
    av_close_input_file(_format_ctx);
    _format_ctx = 0;
  }
  if (_frame) {
    av_free(_frame);
    _frame = 0;
  }
  if (_frame_out) {
    av_free(_frame_out);
    _frame_out = 0;
  }
  _video_ctx = 0;
  _audio_ctx = 0;
  _video_index = -1;
  _audio_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::read_ahead
//       Access: Protected
//  Description: The video prefetches the next video frame in order
//               to be able to implement the next_start query.
//               This function skips over audio packets, but in the
//               process, it updates the time_correction value.
//               If the stream is out of video packets, the packet
//               is empty.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
read_ahead() {
  if (_format_ctx == 0) {
    return;
  }
  
  AVPacket pkt;

  while(av_read_frame(_format_ctx, &pkt)>=0) {
    
    // Is this a packet from the video stream?
    // If so, store corrected timestamp and return.
    
    if (pkt.stream_index== _video_index) {
      double dts = pkt.dts * _video_timebase;
      double correction = get_time_correction();
      _next_start = dts + correction;
      cerr << "Video: " << dts << " " << dts+correction << "\n";
      if (_next_start < _last_start) {
        _next_start = _last_start;
      }
      int finished = 0;
      avcodec_decode_video(_video_ctx, _frame,
                           &finished, pkt.data, pkt.size);
      if (finished) {
        return;
      }
    }
    
    // Is this a packet from the audio stream?
    // If so, use it to calibrate the time correction value.
    
    if (pkt.stream_index== _audio_index) {
      double dts = pkt.dts * _audio_timebase;
      double real = (_samples_read * 1.0) / _audio_ctx->sample_rate;
      update_time_correction(real - dts);

      int16_t buffer[4096];
      uint8_t *audio_pkt_data = pkt.data;
      int audio_pkt_size = pkt.size;
      while(audio_pkt_size > 0) {
        int data_size = sizeof(buffer);
        int decoded = avcodec_decode_audio(_audio_ctx, buffer, &data_size,
                                           audio_pkt_data, audio_pkt_size);
        if(decoded <= 0) {
          break;
        }
        audio_pkt_data += decoded;
        audio_pkt_size -= decoded;
        if (data_size > 0) {
          _samples_read += data_size / (2 * _audio_ctx->channels);
        }
      }

      cerr << "Audio: " << dts << " " << real << "\n";
    }
    
    av_free_packet(&pkt);
  }
  
  _next_start = _next_start + 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::fetch_into_buffer
//       Access: Public, Virtual
//  Description: See MovieVideo::fetch_into_buffer.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
fetch_into_buffer(double time, unsigned char *data, bool rgba) {

  // If there was an error at any point, fetch black.
  if (_format_ctx==0) {
    memset(data,0,size_x()*size_y()*(rgba?4:3));
    return;
  }
  
  AVCodecContext *ctx = _format_ctx->streams[_video_index]->codec;
  nassertv(ctx->width  == size_x());
  nassertv(ctx->height == size_y());
  
  if (rgba) {
    avpicture_fill((AVPicture *)_frame_out, data, PIX_FMT_RGBA32, ctx->width, ctx->height);
    img_convert((AVPicture *)_frame_out, PIX_FMT_RGBA32, 
                (AVPicture *)_frame, ctx->pix_fmt, ctx->width, ctx->height);
  } else {
    avpicture_fill((AVPicture *)_frame_out, data, PIX_FMT_RGB24, ctx->width, ctx->height);
    img_convert((AVPicture *)_frame_out, PIX_FMT_RGB24, 
                (AVPicture *)_frame, ctx->pix_fmt, ctx->width, ctx->height);
  }

  _last_start = _next_start;
  read_ahead();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::make_copy
//       Access: Published, Virtual
//  Description: Make a copy of this MovieVideo with its own cursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) FfmpegVideo::
make_copy() const {
  return new FfmpegVideo(_filename);
}

////////////////////////////////////////////////////////////////////

#endif // HAVE_FFMPEG
