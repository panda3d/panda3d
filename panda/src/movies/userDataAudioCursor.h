// Filename: userDataAudioCursor.h
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef USERDATAAUDIOCURSOR_H
#define USERDATAAUDIOCURSOR_H

#include "pandabase.h"
#include "luse.h"
#include "pointerTo.h"
#include "pointerToArray.h"
class UserDataAudio;

////////////////////////////////////////////////////////////////////
//       Class : UserDataAudioCursor
// Description : A UserDataAudioCursor is a means to manually
//               supply a sequence of raw audio samples.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES UserDataAudioCursor : public MovieAudioCursor {

PUBLISHED:
  UserDataAudioCursor(UserDataAudio *src);
  virtual ~UserDataAudioCursor();
  
public:
  virtual void read_samples(int n, PN_int16 *data);
  friend class UserDataAudio;

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
