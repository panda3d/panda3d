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
  nassertr(stream != nullptr, false);

  stream->read((char *)buffer, size);

  if (stream->eof()) {
    // Gracefully handle EOF.
    stream->clear();
  }

  return stream->gcount();
}

/**
 * Callback passed to dr_flac to implement file I/O via the VirtualFileSystem.
 */
static bool cb_seek_proc(void *user, int offset) {
  std::istream *stream = (std::istream *)user;
  nassertr(stream != nullptr, false);

  stream->seekg(offset, std::ios::cur);
  return !stream->fail();
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

  _drflac = drflac_open(&cb_read_proc, &cb_seek_proc, (void *)stream);

  if (_drflac == nullptr) {
    movies_cat.error()
      << "Failed to open FLAC file.\n";
    _is_valid = false;
  }

  _length = (_drflac->totalSampleCount / _drflac->channels) / (double)_drflac->sampleRate;

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

  if (drflac_seek_to_sample(_drflac, sample * _drflac->channels)) {
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
  int desired = n * _audio_channels;
  n = drflac_read_s16(_drflac, desired, data) / _audio_channels;
  _samples_read += n;
  return n;
}
