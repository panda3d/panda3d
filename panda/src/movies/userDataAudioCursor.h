/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userDataAudioCursor.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef USERDATAAUDIOCURSOR_H
#define USERDATAAUDIOCURSOR_H

#include "pandabase.h"
#include "luse.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "movieAudioCursor.h"

class UserDataAudio;

/**
 * A UserDataAudioCursor is a means to manually supply a sequence of raw audio
 * samples.
 */
class EXPCL_PANDA_MOVIES UserDataAudioCursor : public MovieAudioCursor {

PUBLISHED:
  UserDataAudioCursor(UserDataAudio *src);
  virtual ~UserDataAudioCursor();

public:
  virtual int read_samples(int n, int16_t *data);
  virtual int ready() const;
  virtual void seek(double offset);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudioCursor::init_type();
    register_type(_type_handle, "UserDataAudioCursor",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "userDataAudioCursor.I"
#include "userDataAudio.h"

#endif
