// Filename: vorbisAudioCursor.h
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

#ifndef VORBISAUDIOCURSOR_H
#define VORBISAUDIOCURSOR_H

#include "pandabase.h"
#include "movieAudioCursor.h"

#ifdef HAVE_VORBIS

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class VorbisAudio;

////////////////////////////////////////////////////////////////////
//       Class : VorbisAudioCursor
// Description : Interfaces with the libvorbisfile library to
//               implement decoding of Ogg Vorbis audio files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES VorbisAudioCursor : public MovieAudioCursor {
PUBLISHED:
  VorbisAudioCursor(VorbisAudio *src, istream *stream);
  virtual ~VorbisAudioCursor();
  virtual void seek(double offset);

public:
  virtual void read_samples(int n, PN_int16 *data);

  bool _is_valid;

private:
  // Callbacks passed to libvorbisfile that read via VFS.
  static size_t cb_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
  static int cb_seek_func(void *datasource, ogg_int64_t offset, int whence);
  static int cb_close_func(void *datasource);
  static long cb_tell_func(void *datasource);

protected:
#ifndef CPPPARSER
  OggVorbis_File _ov;
#endif

  int _bitstream;
  double _byte_rate;
  int _block_align;
  int _bytes_per_sample;
  bool _is_float;

  streampos _data_start;
  streampos _data_pos;
  size_t _data_size;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudioCursor::init_type();
    register_type(_type_handle, "VorbisAudioCursor",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vorbisAudioCursor.I"

#endif // HAVE_VORBIS

#endif // VORBISAUDIOCURSOR_H
