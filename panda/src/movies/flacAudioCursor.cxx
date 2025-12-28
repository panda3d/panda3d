/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flacAudioCursor.cxx
 * @author rdb
 * @date 2013-08-23
 */

#include "flacAudioCursor.h"
#include "flacAudio.h"
#include "virtualFileSystem.h"
#include "config_movies.h"

#define DR_FLAC_IMPLEMENTATION
extern "C" {
  #include "dr_flac.h"
}

/**
 * Callback passed to dr_flac to implement file I/O via the VirtualFileSystem.
 */
static size_t cb_read_proc(void *user, void *buffer, size_t size) {
  std::istream *stream = (std::istream *)user;
  nassertr(stream != nullptr, 0);

  stream->read((char *)buffer, size);

  if (stream->eof()) {
    // Gracefully handle EOF.
    stream->clear();
  }

  return stream->gcount();
}

/**
 * Callback passed to dr_flac to implement file seeking via the VirtualFileSystem.
 */
static drflac_bool32 cb_seek_proc(void* user, int offset, drflac_seek_origin origin) {
  std::istream* stream = static_cast<std::istream*>(user);
  nassertr(stream != nullptr, DRFLAC_FALSE);

  std::ios_base::seekdir dir;
  switch (origin) {
    case DRFLAC_SEEK_SET:
      dir = std::ios_base::beg;
      break;
    case DRFLAC_SEEK_CUR:
      dir = std::ios_base::cur;
      break;
    default:
      return DRFLAC_FALSE;
  }

  stream->seekg(offset, dir);
  return !stream->fail() ? DRFLAC_TRUE : DRFLAC_FALSE;
}

/**
 * Callback passed to dr_flac to report the current stream position.
 */
static drflac_bool32 cb_tell_proc(void *user, drflac_int64 *pCursor) {
  std::istream *stream = (std::istream *)user;
  nassertr(stream != nullptr, DRFLAC_FALSE);

  *pCursor = (drflac_int64)stream->tellg();
  return !stream->fail() ? DRFLAC_TRUE : DRFLAC_FALSE;
}

TypeHandle FlacAudioCursor::_type_handle;

/**
 * Reads the .wav header from the indicated stream.  This leaves the read
 * pointer positioned at the start of the data.
 */
FlacAudioCursor::
FlacAudioCursor(FlacAudio *src, std::istream *stream) :
  MovieAudioCursor(src),
  _is_valid(false),
  _drflac(nullptr),
  _stream(stream)
{
  nassertv(stream != nullptr);
  nassertv(stream->good());

  _drflac = drflac_open(&cb_read_proc, &cb_seek_proc, &cb_tell_proc, (void *)stream, nullptr);

  if (_drflac == nullptr) {
    movies_cat.error()
      << "Failed to open FLAC file.\n";
    _is_valid = false;
  }

  _length = _drflac->totalPCMFrameCount / (double)_drflac->sampleRate;

  _audio_channels = _drflac->channels;
  _audio_rate = _drflac->sampleRate;

  _can_seek = true;
  _can_seek_fast = _can_seek;

  _is_valid = true;
}

/**
 * xxx
 */
FlacAudioCursor::
~FlacAudioCursor() {
  if (_drflac != nullptr) {
    drflac_close(_drflac);
  }
  if (_stream != nullptr) {
    VirtualFileSystem::close_read_file(_stream);
  }
}

/**
 * Seeks to a target location.  Afterward, the packet_time is guaranteed to be
 * less than or equal to the specified time.
 */
void FlacAudioCursor::
seek(double t) {
  t = std::max(t, 0.0);

  uint64_t sample = t * _drflac->sampleRate;

  if (drflac_seek_to_pcm_frame(_drflac, sample)) {
    _last_seek = sample / (double)_drflac->sampleRate;
    _samples_read = 0;
  }
}

/**
 * Read audio samples from the stream.  N is the number of samples you wish to
 * read.  Your buffer must be equal in size to N * channels.  Multiple-channel
 * audio will be interleaved.
 */
int FlacAudioCursor::
read_samples(int n, int16_t *data) {
  n = (int)drflac_read_pcm_frames_s16(_drflac, n, data);
  _samples_read += n;
  return n;
}
