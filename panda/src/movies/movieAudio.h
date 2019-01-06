/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudio.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef MOVIEAUDIO_H
#define MOVIEAUDIO_H

#include "pandabase.h"
#include "namable.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"

class MovieAudioCursor;

#ifdef NOTIFY_DEBUG //[
  // Non-release build:
  #define movies_debug(msg) \
  if (movies_cat.is_debug()) { \
      movies_cat->debug() << msg << std::endl; \
  } else {}
#else //][
  // Release build:
  #define movies_debug(msg) ((void)0);
#endif //]

/**
 * A MovieAudio is actually any source that provides a sequence of audio
 * samples.  That could include an AVI file, a microphone, or an internet TV
 * station.
 *
 * The difference between a MovieAudio and a MovieAudioCursor is like the
 * difference between a filename and a file handle.  The MovieAudio just
 * indicates a particular movie.  The MovieAudioCursor is what allows access.
 */
class EXPCL_PANDA_MOVIES MovieAudio : public TypedWritableReferenceCount, public Namable {
 PUBLISHED:
  explicit MovieAudio(const std::string &name = "Blank Audio");
  virtual ~MovieAudio();
  virtual PT(MovieAudioCursor) open();
  static PT(MovieAudio) get(const Filename &name);

  INLINE const Filename &get_filename() const;
  MAKE_PROPERTY(filename, get_filename);

 protected:
  Filename _filename;

 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieAudio",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "movieAudio.I"

#endif
