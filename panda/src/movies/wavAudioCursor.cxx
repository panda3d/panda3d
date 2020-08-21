/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wavAudioCursor.cxx
 * @author rdb
 * @date 2013-08-23
 */

#include "wavAudioCursor.h"
#include "config_movies.h"
#include "virtualFileSystem.h"
#include "wavAudio.h"

// Tables for decompressing mu-law and A-law wav files.
static int16_t mulaw_table[256] = {
  -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
  -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
  -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
  -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
   -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
   -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
   -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
   -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
   -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
   -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
    -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
    -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
    -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
    -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
    -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
     -56,   -48,   -40,   -32,   -24,   -16,    -8,    -1,
   32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
   23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
   15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
   11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
    7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
    5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
    3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
    2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
    1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
    1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
     876,   844,   812,   780,   748,   716,   684,   652,
     620,   588,   556,   524,   492,   460,   428,   396,
     372,   356,   340,   324,   308,   292,   276,   260,
     244,   228,   212,   196,   180,   164,   148,   132,
     120,   112,   104,    96,    88,    80,    72,    64,
      56,    48,    40,    32,    24,    16,     8,     0
};

static int16_t alaw_table[256] = {
  -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
  -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
  -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
  -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
  -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
  -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
  -11008,-10496,-12032,-11520,-8960, -8448, -9984, -9472,
  -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
  -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
  -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
  -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40,
  -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
  -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
  -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
  -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
  -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
   5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
   7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
   2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
   3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
   22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
   30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
   11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472,
   15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
   344,   328,   376,   360,   280,   264,   312,   296,
   472,   456,   504,   488,   408,   392,   440,   424,
   88,    72,    120,   104,    24,     8,    56,    40,
   216,   200,   248,   232,   152,   136,   184,   168,
   1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
   1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
   688,   656,   752,   720,   560,   528,   624,   592,
   944,   912,   1008,  976,   816,   784,   880,   848
};

TypeHandle WavAudioCursor::_type_handle;

/**
 * Reads the .wav header from the indicated stream.  This leaves the read
 * pointer positioned at the start of the data.
 */
WavAudioCursor::
WavAudioCursor(WavAudio *src, std::istream *stream) :
  MovieAudioCursor(src),
  _is_valid(false),
  _stream(stream),
  _reader(stream, false),
  _format(F_pcm),
  _data_pos(0),
  _data_size(0)
{
  nassertv(stream != nullptr);

  // Beginning of "RIFF" chunk.
  unsigned char magic[4];
  if (_reader.extract_bytes(magic, 4) != 4 || memcmp(magic, "RIFF", 4) != 0) {
    movies_cat.error()
      << ".wav file is not a valid RIFF file.\n";
    return;
  }

  unsigned int chunk_size = _reader.get_uint32();

  if (_reader.extract_bytes(magic, 4) != 4 || memcmp(magic, "WAVE", 4) != 0) {
    movies_cat.error()
      << ".wav file is a RIFF file but does not start with a WAVE chunk.\n";
    return;
  }

  // Find a "fmt " subchunk followed by a "data" subchunk.
  bool have_fmt = false, have_data = false;
  unsigned int bytes_read = 4;

  while ((!have_fmt || !have_data) && _stream->good() && (bytes_read + 8) < chunk_size) {

    _reader.extract_bytes(magic, 4);
    unsigned int subchunk_size = _reader.get_uint32();

    if (memcmp(magic, "fmt ", 4) == 0) {
      // The format chunk specifies information about the storage.
      nassertv(subchunk_size >= 16);
      have_fmt = true;

      _format = (Format) _reader.get_uint16();

      _audio_channels = _reader.get_uint16();
      _audio_rate = _reader.get_uint32();

      _byte_rate = (double) _reader.get_uint32();
      _block_align = _reader.get_uint16();

      // We can round up to next multiple of 8.
      uint16_t bps = _reader.get_uint16();
      bps = (bps + 7) & 0xfff8;

      // How many bytes in this chunk we've read so far.
      unsigned int read_bytes = 16;

      // See if there is an extra header to read.
      if (_format == F_extensible) {
        unsigned short ext_size = _reader.get_uint16();
        read_bytes += 2;

        if (ext_size >= 8) {
          /*n_valid_bits =*/ _reader.get_uint16();
          /*speaker_mask =*/ _reader.get_uint32();
          _format = (Format) _reader.get_uint16();

          read_bytes += 8;
        }
      }

      switch (_format) {
      case F_pcm:
        if (bps != 8 && bps != 16 && bps != 24 && bps != 32 && bps != 64) {
          movies_cat.error()
            << "Unsupported number of bits per sample for PCM storage: " << bps << "\n";
          return;
        }
        break;

      case F_float:
        if (bps != 32 && bps != 64) {
          movies_cat.error()
            << "Unsupported number of bits per sample for float storage: " << bps << "\n";
          return;
        }
        break;

      case F_alaw:
      case F_mulaw:
        if (bps != 8) {
          movies_cat.error()
            << ".wav file with A-law or mu-law compression must specify 8 bits per sample, not " << bps << ".\n";
          return;
        }
        break;

      default:
        movies_cat.error()
          << "Unsupported .wav format " << _format << ".  Only PCM, float, A-law and mu-law encodings are supported.\n";
        return;
      }

      _bytes_per_sample = bps / 8;

      // Skip to the end of the chunk.
      if (subchunk_size > read_bytes) {
        _reader.skip_bytes(subchunk_size - read_bytes);
      }

    } else if (memcmp(magic, "data", 4) == 0) {
      // The data chunk contains the actual sammples.
      if (!have_fmt) {
        movies_cat.error()
          << ".wav file specifies 'data' chunk before 'fmt ' chunk.\n";
        break;
      }

      // Excellent!  We've reached the beginning of the data.  Write down
      // where we are and don't move until we want to start reading the data.
      _data_start = stream->tellg();
      _data_size = subchunk_size;
      have_data = true;
      break;

    } else {
      // A chunk we do not recognize.  Just skip it.
      _reader.skip_bytes(subchunk_size);
    }

    bytes_read += subchunk_size + 8;
  }

  // If we bailed out prematurely, there must have been an error.
  if (_stream->eof()) {
    movies_cat.error()
      << "Reached end of file while reading .wav file header.\n";

  } else if (!_stream->good()) {
    movies_cat.error()
      << "Stream error while reading .wav file header.\n";
    return;
  }

  if (!have_fmt) {
    movies_cat.error()
      << ".wav file did not specify a 'fmt ' chunk.\n";
    return;
  }

  if (!have_data) {
    movies_cat.error()
      << ".wav file did not specify a 'data' chunk.\n";
    return;
  }

  // We can always seek by skipping bytes, rereading if necessary.
  _can_seek = true;

  // How to tell if a stream is seekable?  We'll set it to true, and then
  // change it to false as soon as we find out that we can't.
  _can_seek_fast = true;

  if (_block_align != _audio_channels * _bytes_per_sample) {
    movies_cat.warning()
      << ".wav file specified an unexpected block alignment of " << _block_align
      << ", expected " << (_audio_channels * _bytes_per_sample) << ".\n";
    _block_align = _audio_channels * _bytes_per_sample;
  }

  if (_byte_rate != _audio_rate * _block_align) {
    movies_cat.warning()
      << ".wav file specified an unexpected byte rate of " << _byte_rate
      << ", expected " << (_audio_rate * _block_align) << ".\n";
    _byte_rate = _audio_rate * _block_align;
  }

  _length = _data_size / _byte_rate;
  _is_valid = true;
}

/**
 * xxx
 */
WavAudioCursor::
~WavAudioCursor() {
  if (_stream != nullptr) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_stream);
  }
}

/**
 * Seeks to a target location.  Afterward, the packet_time is guaranteed to be
 * less than or equal to the specified time.
 */
void WavAudioCursor::
seek(double t) {
  t = std::max(t, 0.0);
  std::streampos pos = _data_start + (std::streampos) std::min((size_t) (t * _byte_rate), _data_size);

  std::streambuf *buf = _stream->rdbuf();

  if (_can_seek_fast) {
    if (buf->pubseekpos(pos, std::ios::in) != pos) {
      // Clearly, we can't seek fast.  Fall back to the case below.
      _can_seek_fast = false;
    }
  }

  // Get the current position of the cursor in the file.
  std::streampos current = buf->pubseekoff(0, std::ios::cur, std::ios::in);

  if (!_can_seek_fast) {
    if (pos > current) {
      // It is ahead of our current position.  Skip ahead.
      _stream->ignore(pos - current);
      current = pos;

    } else if (pos < current) {
      // Can we seek to the beginning?  Some streams, such as ZStream, let us
      // rewind the stream.
      if (buf->pubseekpos(0, std::ios::in) == (std::streampos)0) {
        if (pos > _data_start && movies_cat.is_info()) {
          Filename fn = get_source()->get_filename();
          movies_cat.info()
            << "Unable to seek backwards in " << fn.get_basename()
            << "; seeking to beginning and skipping " << pos << " bytes.\n";
        }
        _stream->ignore(pos);
        current = pos;
      } else {
        // No; close and reopen the file.
        Filename fn = get_source()->get_filename();
        movies_cat.warning()
          << "Unable to seek backwards in " << fn.get_basename()
          << "; reopening and skipping " << pos << " bytes.\n";

        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        std::istream *stream = vfs->open_read_file(get_source()->get_filename(), true);
        if (stream != nullptr) {
          vfs->close_read_file(_stream);
          stream->ignore(pos);
          _stream = stream;
          _reader = StreamReader(stream, false);
          current = pos;
        } else {
          movies_cat.error()
            << "Unable to reopen " << fn << ".\n";
          _can_seek = false;
        }
      }
    }
  }

  _data_pos = (size_t)current - _data_start;
  _last_seek = _data_pos / _byte_rate;
  _samples_read = 0;
}

/**
 * Read audio samples from the stream.  N is the number of samples you wish to
 * read.  Your buffer must be equal in size to N * channels.  Multiple-channel
 * audio will be interleaved.
 */
int WavAudioCursor::
read_samples(int n, int16_t *data) {
  int desired = n * _audio_channels;
  int read_samples = std::min(desired, ((int) (_data_size - _data_pos)) / _bytes_per_sample);

  if (read_samples <= 0) {
    return 0;
  }

  switch (_format) {
  case F_pcm:
    // Linear PCM storage.
    switch (_bytes_per_sample) {
    case 1:
      // Upsample.  Also, 8-bits samples are stored unsigned.
      for (int i = 0; i < read_samples; ++i) {
        data[i] = (_reader.get_uint8() - 128) * 258/*ish*/;
      }
      break;

    case 2:
      // Our native format.  Read directly from file.
      read_samples = _reader.extract_bytes((unsigned char*) data, read_samples * 2) / 2;
      break;

    case 3: {
      // The scale factor happens to be 256 for 24-bit samples.  That means we
      // can just read the most significant bytes.
      for (int i = 0; i < read_samples; ++i) {
        _reader.skip_bytes(1);
        data[i] = _reader.get_int16();
      }
      break;

    } case 4: {
      // Downsample.
      const int32_t scale_factor = 0x7fffffff / 0x7fff;

      for (int i = 0; i < read_samples; ++i) {
        data[i] = _reader.get_int32() / scale_factor;
      }
      break;

    } case 8: {
      // Downsample.
      const int64_t scale_factor = 0x7fffffffffffffffLL / 0x7fffLL;

      for (int i = 0; i < read_samples; ++i) {
        data[i] = _reader.get_int64() / scale_factor;
      }
      break;

    } default:
      // Huh?
      read_samples = 0;
    }
    break;

  case F_float:
    // IEEE float storage.
    switch (_bytes_per_sample) {
    case 4:
      for (int i = 0; i < read_samples; ++i) {
        data[i] = (int16_t) (_reader.get_float32() * 0x7fff);
      }
      break;

    case 8:
      for (int i = 0; i < read_samples; ++i) {
        data[i] = (int16_t) (_reader.get_float64() * 0x7fff);
      }
      break;

    default:
      read_samples = 0;
    }
    break;

  case F_alaw:
    for (int i = 0; i < read_samples; ++i) {
      data[i] = alaw_table[_reader.get_uint8()];
    }
    break;

  case F_mulaw:
    for (int i = 0; i < read_samples; ++i) {
      data[i] = mulaw_table[_reader.get_uint8()];
    }
    break;

  default:
    read_samples = 0;
  }

  // Fill the rest of the buffer with silence.
  if (read_samples < desired) {
    memset(data + read_samples, 0, (desired - read_samples) * 2);
    n = read_samples / _audio_channels;
  }

  _data_pos = _stream->tellg() - _data_start;
  _samples_read += n;
  return n;
}
