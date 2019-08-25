/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flacAudio.h
 * @author rdb
 * @date 2016-04-27
 */

#ifndef FLACAUDIO_H
#define FLACAUDIO_H

#include "pandabase.h"
#include "movieAudio.h"

class FlacAudioCursor;

/**
 * Reads FLAC audio files.  Ogg-encapsulated FLAC files are not supported.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_MOVIES FlacAudio : public MovieAudio {
PUBLISHED:
  FlacAudio(const Filename &name);
  virtual ~FlacAudio();
  virtual PT(MovieAudioCursor) open();

  static PT(MovieAudio) make(const Filename &name);

private:
  friend class FlacAudioCursor;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudio::init_type();
    register_type(_type_handle, "FlacAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // FLACAUDIO_H
