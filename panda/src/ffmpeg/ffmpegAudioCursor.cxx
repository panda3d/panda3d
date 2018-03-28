/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ffmpegAudioCursor.cxx
 * @author jyelon
 * @date 2007-08-01
 */

#include "config_ffmpeg.h"
#include "ffmpegAudioCursor.h"

#include "ffmpegAudio.h"
extern "C" {
  #include "libavutil/dict.h"
  #include "libavutil/opt.h"
  #include "libavcodec/avcodec.h"
  #include "libavformat/avformat.h"
}

#ifdef HAVE_SWRESAMPLE
extern "C" {
  #include "libswresample/swresample.h"
}
#endif

TypeHandle FfmpegAudioCursor::_type_handle;

#if LIBAVFORMAT_VERSION_MAJOR < 53
  #define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#endif

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
// More recent versions of ffmpeg no longer define this.
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

/**
 * xxx
 */
FfmpegAudioCursor::
FfmpegAudioCursor(FfmpegAudio *src) :
  MovieAudioCursor(src),
  _filename(src->_filename),
  _packet(0),
  _packet_data(0),
  _format_ctx(0),
  _audio_ctx(0),
  _resample_ctx(0),
  _buffer(0),
  _buffer_alloc(0),
  _frame(0)
{
  if (!_ffvfile.open_vfs(_filename)) {
    cleanup();
    return;
  }

  _format_ctx = _ffvfile.get_format_context();
  nassertv(_format_ctx != NULL);

  if (avformat_find_stream_info(_format_ctx, NULL) < 0) {
    cleanup();
    return;
  }

  // As of libavformat version 57.41.100, AVStream.codec is deprecated in favor
  // of AVStream.codecpar.  Fortunately, the two structures have
  // similarly-named members, so we can just switch out the declaration.
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
  AVCodecParameters *codecpar;
#else
  AVCodecContext *codecpar;
#endif

  // Find the audio stream
  AVStream *stream = nullptr;
  for (int i = 0; i < (int)_format_ctx->nb_streams; i++) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
    codecpar = _format_ctx->streams[i]->codecpar;
#else
    codecpar = _format_ctx->streams[i]->codec;
#endif
    if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      _audio_index = i;
      stream = _format_ctx->streams[i];
      break;
    }
  }

  if (stream == nullptr) {
    cleanup();
    return;
  }

  _audio_timebase = av_q2d(stream->time_base);
  _audio_rate = codecpar->sample_rate;
  _audio_channels = codecpar->channels;

  AVCodec *pAudioCodec = avcodec_find_decoder(codecpar->codec_id);
  if (pAudioCodec == 0) {
    cleanup();
    return;
  }

  _audio_ctx = avcodec_alloc_context3(pAudioCodec);

  if (_audio_ctx == nullptr) {
    cleanup();
    return;
  }

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
  avcodec_parameters_to_context(_audio_ctx, codecpar);
#else
  avcodec_copy_context(_audio_ctx, codecpar);
#endif

  AVDictionary *opts = NULL;
  av_dict_set(&opts, "request_sample_fmt", "s16", 0);
  if (avcodec_open2(_audio_ctx, pAudioCodec, NULL) < 0) {
    cleanup();
    return;
  }

  av_dict_free(&opts);

  // Set up the resample context if necessary.
  if (_audio_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
#ifdef HAVE_SWRESAMPLE
    ffmpeg_cat.debug()
      << "Codec does not use signed 16-bit sample format.  Setting up swresample context.\n";

    _resample_ctx = swr_alloc();
    av_opt_set_int(_resample_ctx, "in_channel_layout", _audio_ctx->channel_layout, 0);
    av_opt_set_int(_resample_ctx, "out_channel_layout", _audio_ctx->channel_layout, 0);
    av_opt_set_int(_resample_ctx, "in_sample_rate", _audio_ctx->sample_rate, 0);
    av_opt_set_int(_resample_ctx, "out_sample_rate", _audio_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(_resample_ctx, "in_sample_fmt", _audio_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(_resample_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    if (swr_init(_resample_ctx) != 0) {
      ffmpeg_cat.error()
        << "Failed to set up resample context.\n";
      _resample_ctx = NULL;
    }
#else
    ffmpeg_cat.error()
      << "Codec does not use signed 16-bit sample format, but support for libswresample has not been enabled.\n";
#endif
  }

  _length = (_format_ctx->duration * 1.0) / AV_TIME_BASE;
  _can_seek = true;
  _can_seek_fast = true;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
  _frame = av_frame_alloc();
#else
  _frame = avcodec_alloc_frame();
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
  _packet = av_packet_alloc();
#else
  _packet = new AVPacket;
#endif

  _buffer_size = AVCODEC_MAX_AUDIO_FRAME_SIZE / 2;
  _buffer_alloc = new int16_t[_buffer_size + 64];

  // Allocate enough space for 1024 samples per channel.
  if ((_packet == 0)||(_buffer_alloc == 0)) {
    cleanup();
    return;
  }
  memset(_packet, 0, sizeof(AVPacket));

  // Align the buffer to a 64-byte boundary The ffmpeg codec likes this,
  // because it uses SSESSE2.
  _buffer = _buffer_alloc;
  while (((size_t)_buffer) & 31) {
    _buffer += 1;
  }

  fetch_packet();
  _initial_dts = _packet->dts;
  _last_seek = 0;
  _samples_read = 0;
  _buffer_head = 0;
  _buffer_tail = 0;
}

/**
 * xxx
 */
FfmpegAudioCursor::
~FfmpegAudioCursor() {
  cleanup();
}

/**
 * Reset to a standard inactive state.
 */
void FfmpegAudioCursor::
cleanup() {
  if (_frame) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
    av_frame_free(&_frame);
#else
    avcodec_free_frame(&_frame);
#endif
    _frame = NULL;
  }

  if (_packet) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_free(&_packet);
#else
    if (_packet->data) {
      av_free_packet(_packet);
    }
    delete _packet;
    _packet = NULL;
#endif
  }

  if (_buffer_alloc) {
    delete[] _buffer_alloc;
    _buffer_alloc = 0;
    _buffer = NULL;
  }

  if ((_audio_ctx)&&(_audio_ctx->codec)) {
    avcodec_close(_audio_ctx);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 52, 0)
    avcodec_free_context(&_audio_ctx);
#else
    delete _audio_ctx;
#endif
  }
  _audio_ctx = NULL;

  if (_format_ctx) {
    _ffvfile.close();
    _format_ctx = NULL;
  }

#ifdef HAVE_SWRESAMPLE
  if (_resample_ctx) {
    swr_free(&_resample_ctx);
    _resample_ctx = NULL;
  }
#endif

  _audio_index = -1;
}

/**
 * Fetches an audio packet and stores it in the packet buffer.  Also sets
 * packet_size and packet_data.
 */
void FfmpegAudioCursor::
fetch_packet() {
  if (_packet->data) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_unref(_packet);
#else
    av_free_packet(_packet);
#endif
  }
  while (av_read_frame(_format_ctx, _packet) >= 0) {
    if (_packet->stream_index == _audio_index) {
      _packet_size = _packet->size;
      _packet_data = _packet->data;
      return;
    }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_unref(_packet);
#else
    av_free_packet(_packet);
#endif
  }
  _packet->data = 0;
  _packet_size = 0;
  _packet_data = 0;
}

/**
 * Reloads the audio buffer by decoding audio packets until one of those audio
 * packets finally yields some samples.  If we encounter the end of the
 * stream, we synthesize silence.
 */
bool FfmpegAudioCursor::
reload_buffer() {

  while (_buffer_head == _buffer_tail) {
    // If we're out of packets, generate silence.
    if (_packet->data == 0) {
      _buffer_head = 0;
      _buffer_tail = _buffer_size;
      memset(_buffer, 0, _buffer_size * 2);
      return true;
    } else if (_packet_size > 0) {
      int bufsize = _buffer_size * 2;
      int got_frame;

      AVPacket *pkt;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
      pkt = av_packet_alloc();
#else
      AVPacket _pkt;
      pkt = &_pkt;
      av_init_packet(pkt);
#endif
      pkt->data = _packet_data;
      pkt->size = _packet_size;

      int len = avcodec_decode_audio4(_audio_ctx, _frame, &got_frame, pkt);
      movies_debug("avcodec_decode_audio4 returned " << len);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
      av_packet_free(&pkt);
#else
      av_free_packet(pkt);
#endif

      bufsize = 0;
      if (got_frame) {
#ifdef HAVE_SWRESAMPLE
        if (_resample_ctx) {
          // Resample the data to signed 16-bit sample format.
          bufsize = swr_convert(_resample_ctx, (uint8_t **)&_buffer, _buffer_size / 2, (const uint8_t**)_frame->extended_data, _frame->nb_samples);
          bufsize *= _audio_channels * 2;
        } else
#endif
        {
          bufsize = _frame->linesize[0];
          memcpy(_buffer, _frame->data[0], bufsize);
        }
      }
#if LIBAVUTIL_VERSION_INT > AV_VERSION_INT(52, 19, 100)
      av_frame_unref(_frame);
#endif

      if (len < 0) {
        return false;
      } else if (len == 0){
        return true;
      }
      _packet_data += len;
      _packet_size -= len;
      if (bufsize > 0) {
        _buffer_head = 0;
        _buffer_tail = (bufsize/2);
        return true;
      }
    } else {
      fetch_packet();
    }
  }
  return true;
}

/**
 * Seeks to a target location.  Afterward, the packet_time is guaranteed to be
 * less than or equal to the specified time.
 */
void FfmpegAudioCursor::
seek(double t) {
  int64_t target_ts = (int64_t)(t / _audio_timebase);
  if (target_ts < (int64_t)(_initial_dts)) {
    // Attempts to seek before the first packet will fail.
    target_ts = _initial_dts;
  }
  if (av_seek_frame(_format_ctx, _audio_index, target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
    ffmpeg_cat.error() << "Seek failure. Shutting down movie.\n";
    cleanup();
    return;
  }
  avcodec_close(_audio_ctx);
  AVCodec *pAudioCodec = avcodec_find_decoder(_audio_ctx->codec_id);
  if(pAudioCodec == 0) {
    cleanup();
    return;
  }
  if (avcodec_open2(_audio_ctx, pAudioCodec, NULL) < 0) {
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

/**
 * Read audio samples from the stream.  N is the number of samples you wish to
 * read.  Your buffer must be equal in size to N * channels.  Multiple-channel
 * audio will be interleaved.
 */
void FfmpegAudioCursor::
read_samples(int n, int16_t *data) {
  int desired = n * _audio_channels;

  while (desired > 0) {
    if (_buffer_head == _buffer_tail) {
      if(!reload_buffer()){
        break;
      }
      movies_debug("read_samples() desired samples: " << desired << " N:" << n);
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
