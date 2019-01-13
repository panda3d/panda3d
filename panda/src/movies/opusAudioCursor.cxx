/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file opusAudioCursor.cxx
 * @author rdb
 * @date 2017-05-24
 */

#include "opusAudioCursor.h"

#include "config_movies.h"

#include "opusAudio.h"
#include "virtualFileSystem.h"

#ifdef HAVE_OPUS

#include <opus/opusfile.h>

using std::istream;

/**
 * Callbacks passed to libopusfile to implement file I/O via the
 * VirtualFileSystem.
 */
int cb_read(void *stream, unsigned char *ptr, int nbytes) {
  istream *in = (istream *)stream;
  nassertr(in != nullptr, -1);

  in->read((char *)ptr, nbytes);

  if (in->eof()) {
    // Gracefully handle EOF.
    in->clear();
  }

  return in->gcount();
}

int cb_seek(void *stream, opus_int64 offset, int whence) {
  if (!opus_enable_seek) {
    return -1;
  }

  istream *in = (istream *)stream;
  nassertr(in != nullptr, -1);

  switch (whence) {
  case SEEK_SET:
    in->seekg(offset, std::ios::beg);
    break;

  case SEEK_CUR:
    in->seekg(offset, std::ios::cur);
    break;

  case SEEK_END:
    in->seekg(offset, std::ios::end);
    break;

  default:
    movies_cat.error()
      << "Illegal parameter to seek in cb_seek\n";
    return -1;
  }

  if (in->fail()) {
    movies_cat.error()
      << "Failure to seek to byte " << offset;

    switch (whence) {
    case SEEK_CUR:
      movies_cat.error(false)
        << " from current location!\n";
      break;

    case SEEK_END:
      movies_cat.error(false)
        << " from end of file!\n";
      break;

    default:
      movies_cat.error(false) << "!\n";
    }

    return -1;
  }

  return 0;
}

opus_int64 cb_tell(void *stream) {
  istream *in = (istream *)stream;
  nassertr(in != nullptr, -1);

  return in->tellg();
}

int cb_close(void *stream) {
  istream *in = (istream *)stream;
  nassertr(in != nullptr, EOF);

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->close_read_file(in);
  return 0;
}

static const OpusFileCallbacks callbacks = {cb_read, cb_seek, cb_tell, cb_close};

TypeHandle OpusAudioCursor::_type_handle;

/**
 * Reads the .wav header from the indicated stream.  This leaves the read
 * pointer positioned at the start of the data.
 */
OpusAudioCursor::
OpusAudioCursor(OpusAudio *src, istream *stream) :
  MovieAudioCursor(src),
  _is_valid(false),
  _link(0)
{
  nassertv(stream != nullptr);
  nassertv(stream->good());

  int error = 0;
  _op = op_open_callbacks((void *)stream, &callbacks, nullptr, 0, &error);
  if (_op == nullptr) {
    movies_cat.error()
      << "Failed to read Opus file (error code " << error << ").\n";
    return;
  }

  ogg_int64_t samples = op_pcm_total(_op, -1);
  if (samples != OP_EINVAL) {
    // Opus timestamps are fixed at 48 kHz.
    _length = (double)samples / 48000.0;
  }

  _audio_channels = op_channel_count(_op, -1);
  _audio_rate = 48000;

  _can_seek = opus_enable_seek && op_seekable(_op);
  _can_seek_fast = _can_seek;

  _is_valid = true;
}

/**
 * xxx
 */
OpusAudioCursor::
~OpusAudioCursor() {
  if (_op != nullptr) {
    op_free(_op);
    _op = nullptr;
  }
}

/**
 * Seeks to a target location.  Afterward, the packet_time is guaranteed to be
 * less than or equal to the specified time.
 */
void OpusAudioCursor::
seek(double t) {
  if (!opus_enable_seek) {
    return;
  }

  t = std::max(t, 0.0);

  // Use op_time_seek_lap if cross-lapping is enabled.
  int error = op_pcm_seek(_op, (ogg_int64_t)(t * 48000.0));
  if (error != 0) {
    movies_cat.error()
      << "Seek failed (error " << error << ").  Opus stream may not be seekable.\n";
    return;
  }

  _last_seek = op_pcm_tell(_op) / 48000.0;
  _samples_read = 0;
}

/**
 * Read audio samples from the stream.  N is the number of samples you wish to
 * read.  Your buffer must be equal in size to N * channels.  Multiple-channel
 * audio will be interleaved.
 */
void OpusAudioCursor::
read_samples(int n, int16_t *data) {
  int16_t *end = data + (n * _audio_channels);

  while (data < end) {
    // op_read gives it to us in the exact format we need.  Nifty!
    int link;
    int read_samples = op_read(_op, data, end - data, &link);
    if (read_samples > 0) {
      data += read_samples * _audio_channels;
      _samples_read += read_samples;
    } else {
      break;
    }

    if (_link != link) {
      // It is technically possible for it to change parameters from one link
      // to the next.  However, we don't offer this flexibility.
      int channels = op_channel_count(_op, link);
      if (channels != _audio_channels) {
        movies_cat.error()
          << "Opus file has inconsistent channel count!\n";

        // We'll change it anyway.  Not sure what happens next.
        _audio_channels = channels;
      }

      _link = link;
    }
  }

  // Fill the rest of the buffer with silence.
  if (data < end) {
    memset(data, 0, (unsigned char *)end - (unsigned char *)data);
  }
}

#endif // HAVE_OPUS
