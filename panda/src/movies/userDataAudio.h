// Filename: userDataAudio.h
// Created by: jyelon (02Jul07)
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

#ifndef USERDATAAUDIO_H
#define USERDATAAUDIO_H

#include "movieAudio.h"
#include "datagramIterator.h"
#include "pdeque.h"

class MovieAudioCursor;
class UserDataAudioCursor;

////////////////////////////////////////////////////////////////////
//       Class : UserDataAudio
// Description : A UserDataAudio is a way for the user to manually
//               supply raw audio samples. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES UserDataAudio : public MovieAudio {

 PUBLISHED:
  UserDataAudio(int rate, int channels);
  virtual ~UserDataAudio();
  virtual PT(MovieAudioCursor) open();

  void append(PN_int16 *data, int n);
  void append(DatagramIterator *src, int len=0x40000000);
  void append(const string &str);
  void done(); // A promise not to write any more samples.  

 private:
  void read_samples(int n, PN_int16 *data);
  void update_cursor();
  int _desired_rate;
  int _desired_channels;
  UserDataAudioCursor *_cursor;
  pdeque<PN_int16> _data;
  bool _aborted;
  friend class UserDataAudioCursor;
  
 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudio::init_type();
    register_type(_type_handle, "UserDataAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "userDataAudio.I"

#endif
