/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flacAudioCursor.h
 * @author rdb
 * @date 2013-08-23
 */

#ifndef FLACAUDIOCURSOR_H
#define FLACAUDIOCURSOR_H

#include "pandabase.h"
#include "movieAudioCursor.h"

#define DR_FLAC_NO_STDIO
extern "C" {
  #include "dr_flac.h"
}

class FlacAudio;

/**
 * Implements decoding of FLAC audio files.
 *
 * @see FlacAudio
 * @since 1.10.0
 */
class EXPCL_PANDA_MOVIES FlacAudioCursor : public MovieAudioCursor {
PUBLISHED:
  explicit FlacAudioCursor(FlacAudio *src, std::istream *stream);
  virtual ~FlacAudioCursor();
  virtual void seek(double offset);

public:
  virtual int read_samples(int n, int16_t *data);

  bool _is_valid;

protected:
  drflac *_drflac;
  std::istream *_stream;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudioCursor::init_type();
    register_type(_type_handle, "FlacAudioCursor",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "flacAudioCursor.I"

#endif // FLACAUDIOCURSOR_H
