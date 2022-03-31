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
    // opusfile uses a seek with offset 0 to determine whether seeking is
    // supported, but this is not good enough.  We seek to the end and back.
    if (offset == 0) {
      std::streambuf *buf = in->rdbuf();
      std::streampos pos = buf->pubseekoff(0, std::ios::cur, std::ios::in);
      if (pos < 0) {
        return -1;
      }
      if (buf->pubseekoff(0, std::ios::end, std::ios::in) >= 0) {
        // It worked; seek back to the previous location.
        buf->pubseekpos(pos, std::ios::in);
        return 0;
      } else {
        return -1;
      }
    }
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

static const OpusFileCallbacks callbacks = {cb_read, cb_seek, cb_tell, nullptr};

TypeHandle OpusAudioCursor::_type_handle;

/**
 * Reads the .wav header from the indicated stream.  This leaves the read
 * pointer positioned at the start of the data.
 */
OpusAudioCursor::
OpusAudioCursor(OpusAudio *src, istream *stream) :
  MovieAudioCursor(src),
  _is_valid(false),
  _stream(stream),
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

  if (_stream != nullptr) {
    VirtualFileSystem::close_read_file(_stream);
    _stream = nullptr;
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
  ogg_int64_t sample = (ogg_int64_t)(t * 48000.0);
  int error = op_pcm_seek(_op, sample);

  // Special case for seeking to the beginning; if normal seek fails, we may
  // be able to explicitly seek to the beginning of the file and call op_open
  // again.  This allows looping compressed .opus files.
  if (error == OP_ENOSEEK && sample == 0) {
    if (_stream->rdbuf()->pubseekpos(0, std::ios::in) == (std::streampos)0) {
      OggOpusFile *op = op_open_callbacks((void *)_stream, &callbacks, nullptr, 0, nullptr);
      if (op != nullptr) {
        op_free(_op);
        _op = op;
      } else {
        movies_cat.error()
          << "Failed to reopen Opus file to seek to beginning.\n";
        return;
      }

      // Reset this field for good measure, just in case this changed.
      _audio_channels = op_channel_count(_op, -1);

      _last_seek = 0.0;
      _samples_read = 0;
      return;
    }
  }
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
int OpusAudioCursor::
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
      if (read_samples == 0 && _length == 1.0E10) {
        _length = op_pcm_tell(_op) / 48000.0;
      }
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
    n -= (end - data) / _audio_channels;
  }
  return n;
}

#endif // HAVE_OPUS
