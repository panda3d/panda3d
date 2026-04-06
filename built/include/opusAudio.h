/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file opusAudio.h
 * @author rdb
 * @date 2017-05-24
 */

#ifndef OPUSAUDIO_H
#define OPUSAUDIO_H

#include "pandabase.h"
#include "movieAudio.h"

#ifdef HAVE_OPUS

class OpusAudioCursor;

/**
 * Interfaces with the libopusfile library to implement decoding of Opus
 * audio files.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_MOVIES OpusAudio : public MovieAudio {
PUBLISHED:
  OpusAudio(const Filename &name);
  virtual ~OpusAudio();
  virtual PT(MovieAudioCursor) open();

  static PT(MovieAudio) make(const Filename &name);

private:
  friend class OpusAudioCursor;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "OpusAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "opusAudio.I"

#endif // HAVE_OPUS

#endif // OPUSAUDIO_H
