// Filename: ffmpegAudioCursor.cxx
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifdef HAVE_FFMPEG

#include "ffmpegAudioCursor.h"
extern "C" {
  #include "avcodec.h"
  #include "avformat.h"
}

TypeHandle FfmpegAudioCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudioCursor::
FfmpegAudioCursor(FfmpegAudio *src) :
  MovieAudioCursor(src),
  _filename(src->_filename),
  _packet(0),
  _packet_data(0),
  _format_ctx(0),
  _audio_ctx(0),
  _buffer(0),
  _buffer_alloc(0)
{
  string url = "pandavfs:";
  url += _filename;
  if (av_open_input_file(&_format_ctx, url.c_str(), NULL, 0, NULL)!=0) {
    cleanup();
    return;
  }

  if (av_find_stream_info(_format_ctx)<0) {
    cleanup();
    return;
  }

  // Find the audio stream
  for(int i=0; i<_format_ctx->nb_streams; i++) {
    if(_format_ctx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) {
      _audio_index = i;
      _audio_ctx = _format_ctx->streams[i]->codec;
      _audio_timebase = av_q2d(_format_ctx->streams[i]->time_base);
      _audio_rate = _audio_ctx->sample_rate;
      _audio_channels = _audio_ctx->channels;
    }
  }

  if (_audio_ctx == 0) {
    cleanup();
    return;
  }

  AVCodec *pAudioCodec=avcodec_find_decoder(_audio_ctx->codec_id);
  if(pAudioCodec == 0) {
    cleanup();
    return;
  }
  if(avcodec_open(_audio_ctx, pAudioCodec)<0) {
    cleanup();
    return;
  }

  _length = (_format_ctx->duration * 1.0) / AV_TIME_BASE;
  _can_seek = true;
  _can_seek_fast = true;

  _packet = new AVPacket;
  _buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE / 2;
  _buffer_alloc = new PN_int16[_buffer_size + 128];
  if ((_packet == 0)||(_buffer_alloc == 0)) {
    cleanup();
    return;
  }
  memset(_packet, 0, sizeof(AVPacket));

  // Align the buffer to a 16-byte boundary
  // The ffmpeg codec likes this, because it uses SSE/SSE2.
  _buffer = _buffer_alloc;
  while (((size_t)_buffer) & 15) {
    _buffer += 1;
  }

  fetch_packet();
  _initial_dts = _packet->dts;
  _last_seek = 0;
  _samples_read = 0;
  _buffer_head = 0;
  _buffer_tail = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudioCursor::
~FfmpegAudioCursor() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::cleanup
//       Access: Public
//  Description: Reset to a standard inactive state.
////////////////////////////////////////////////////////////////////
void FfmpegAudioCursor::
cleanup() {
  if (_packet) {
    if (_packet->data) {
      av_free_packet(_packet);
    }
    delete _packet;
    _packet = 0;
  }
  if (_buffer_alloc) {
    delete[] _buffer_alloc;
    _buffer_alloc = 0;
    _buffer = 0;
  }
  if ((_audio_ctx)&&(_audio_ctx->codec)) {
    avcodec_close(_audio_ctx);
  }
  _audio_ctx = 0;
  if (_format_ctx) {
    av_close_input_file(_format_ctx);
    _format_ctx = 0;
  }
  _audio_ctx = 0;
  _audio_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::fetch_packet
//       Access: Protected
//  Description: Fetches an audio packet and stores it in the
//               packet buffer.  Also sets packet_size and packet_data.
////////////////////////////////////////////////////////////////////
void FfmpegAudioCursor::
fetch_packet() {
  if (_packet->data) {
    av_free_packet(_packet);
  }
  while (av_read_frame(_format_ctx, _packet) >= 0) {
    if (_packet->stream_index == _audio_index) {
      _packet_size = _packet->size;
      _packet_data = _packet->data;
      return;
    }
    av_free_packet(_packet);
  }
  _packet->data = 0;
  _packet_size = 0;
  _packet_data = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::reload_buffer
//       Access: Protected
//  Description: Reloads the audio buffer by decoding audio packets
//               until one of those audio packets finally yields
//               some samples.  If we encounter the end of the
//               stream, we synthesize silence.
////////////////////////////////////////////////////////////////////
void FfmpegAudioCursor::
reload_buffer() {


  while (_buffer_head == _buffer_tail) {
    // If we're out of packets, generate silence.
    if (_packet->data == 0) {
      _buffer_head = 0;
      _buffer_tail = _buffer_size;
      memset(_buffer, 0, _buffer_size * 2);
      return;
    } else if (_packet_size > 0) {
      int bufsize = _buffer_size * 2;
      int len = avcodec_decode_audio(_audio_ctx, _buffer, &bufsize,
                                     _packet_data, _packet_size);
      movies_debug("avcodec_decode_audio returned " << len);
      if (len <= 0) {
        break;
      }
      _packet_data += len;
      _packet_size -= len;
      if (bufsize > 0) {
        _buffer_head = 0;
        _buffer_tail = (bufsize/2);
        return;
      }
    } else {
      fetch_packet();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::seek
//       Access: Protected
//  Description: Seeks to a target location.  Afterward, the
//               packet_time is guaranteed to be less than or
//               equal to the specified time.
////////////////////////////////////////////////////////////////////
void FfmpegAudioCursor::
seek(double t) {
  PN_int64 target_ts = (PN_int64)(t / _audio_timebase);
  if (target_ts < (PN_int64)(_initial_dts)) {
    // Attempts to seek before the first packet will fail.
    target_ts = _initial_dts;
  }
  if (av_seek_frame(_format_ctx, _audio_index, target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
    movies_cat.error() << "Seek failure. Shutting down movie.\n";
    cleanup();
    return;
  }
  avcodec_close(_audio_ctx);
  AVCodec *pAudioCodec=avcodec_find_decoder(_audio_ctx->codec_id);
  if(pAudioCodec == 0) {
    cleanup();
    return;
  }
  if(avcodec_open(_audio_ctx, pAudioCodec)<0) {
    cleanup();
    return;
  }
  _buffer_head = 0;
  _buffer_tail = 0;
  fetch_packet();
  double ts = _packet->dts * _audio_timebase;
  if (t > ts) {
    int skip = (int)((t-ts) * _audio_rate);
    read_samples(skip, 0);
  }
  _last_seek = t;
  _samples_read = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudioCursor::read_samples
//       Access: Public, Virtual
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.
//               Multiple-channel audio will be interleaved.
////////////////////////////////////////////////////////////////////
void FfmpegAudioCursor::
read_samples(int n, PN_int16 *data) {

  //movies_debug("here!!! FfmpegAudioCursor n="<<n);

  int desired = n * _audio_channels;

  // give up after 100 tries to fetch data
  int give_up_after = 100;

  while (desired && give_up_after > 0) {

    if (_buffer_head == _buffer_tail) {
      reload_buffer();
      give_up_after --;
      movies_debug("reload_buffer will give up in "<<give_up_after);
    }
    int available = _buffer_tail - _buffer_head;
    int ncopy = (desired > available) ? available : desired;
    if (ncopy) {
      if (data != 0) {
        memcpy(data, _buffer + _buffer_head, ncopy * 2);
        data += ncopy;
      }
      desired -= ncopy;
      _buffer_head += ncopy;
    }

  }
  _samples_read += n;
}

////////////////////////////////////////////////////////////////////

#endif // HAVE_FFMPEG
