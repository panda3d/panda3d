/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudioCursor.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef MOVIEAUDIOCURSOR_H
#define MOVIEAUDIOCURSOR_H

#include "pandabase.h"
#include "namable.h"
#include "texture.h"
#include "pointerTo.h"
class MovieAudio;

/**
 * A MovieAudio is actually any source that provides a sequence of audio
 * samples.  That could include an AVI file, a microphone, or an internet TV
 * station.  A MovieAudioCursor is a handle that lets you read data
 * sequentially from a MovieAudio.
 *
 * Thread safety: each individual MovieAudioCursor must be owned and accessed
 * by a single thread.  It is OK for two different threads to open the same
 * file at the same time, as long as they use separate MovieAudioCursor
 * objects.
 */
class EXPCL_PANDA_MOVIES MovieAudioCursor : public TypedWritableReferenceCount {

PUBLISHED:
  MovieAudioCursor(MovieAudio *src);
  virtual ~MovieAudioCursor();
  INLINE PT(MovieAudio) get_source() const;
  INLINE int audio_rate() const;
  INLINE int audio_channels() const;
  INLINE double length() const;
  INLINE bool can_seek() const;
  INLINE bool can_seek_fast() const;
  INLINE double tell() const;
  INLINE void skip_samples(int n);
  INLINE bool aborted() const;
  virtual int ready() const;
  virtual void seek(double offset);
  void read_samples(int n, Datagram *dg);
  vector_uchar read_samples(int n);

public:
  virtual int read_samples(int n, int16_t *data);

protected:
  PT(MovieAudio) _source;
  int _audio_rate;
  int _audio_channels;
  double _length;
  bool _can_seek;
  bool _can_seek_fast;
  bool _aborted;
  double _last_seek;
  int64_t _samples_read;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieAudioCursor",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieAudioCursor.I"
#include "movieAudio.h"

#endif
