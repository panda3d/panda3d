// Filename: wavAudioCursor.h
// Created by: rdb (23Aug13)
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

#ifndef WAVAUDIOCURSOR_H
#define WAVAUDIOCURSOR_H

#include "pandabase.h"
#include "movieAudioCursor.h"
#include "streamReader.h"

class WavAudio;

////////////////////////////////////////////////////////////////////
//       Class : WavAudioCursor
// Description : Used for reading PCM .wav files.  Supported formats
//               are linear PCM, IEEE float, A-law and mu-law.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES WavAudioCursor : public MovieAudioCursor {
PUBLISHED:
  WavAudioCursor(WavAudio *src, istream *stream);
  virtual ~WavAudioCursor();
  virtual void seek(double offset);

public:
  virtual void read_samples(int n, PN_int16 *data);

  bool _is_valid;

protected:
  // Format codes as used by wave files.
  enum Format {
    F_pcm = 0x0001,
    F_float = 0x0003,
    F_alaw = 0x0006,
    F_mulaw = 0x0007,

    F_extensible = 0xfffe,
  };

  istream *_stream;
  StreamReader _reader;

  Format _format;
  double _byte_rate;
  int _block_align;
  int _bytes_per_sample;

  streampos _data_start;
  streampos _data_pos;
  size_t _data_size;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudioCursor::init_type();
    register_type(_type_handle, "WavAudioCursor",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wavAudioCursor.I"

#endif // WAVAUDIOCURSOR_H
