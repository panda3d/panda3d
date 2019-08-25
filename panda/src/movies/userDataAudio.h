/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userDataAudio.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef USERDATAAUDIO_H
#define USERDATAAUDIO_H

#include "movieAudio.h"
#include "datagramIterator.h"
#include "pdeque.h"

class MovieAudioCursor;
class UserDataAudioCursor;

/**
 * A UserDataAudio is a way for the user to manually supply raw audio samples.
 * remove_after_read means the data will be removed if read once.  Else data
 * will be stored (enable looping and seeking). Expects data as 16 bit signed
 * (word); Example for stereo: 1.word = 1.channel,2.word = 2.channel, 3.word =
 * 1.channel,4.word = 2.channel, etc.
 */
class EXPCL_PANDA_MOVIES UserDataAudio : public MovieAudio {

 PUBLISHED:
  UserDataAudio(int rate, int channels, bool remove_after_read=true);
  virtual ~UserDataAudio();
  virtual PT(MovieAudioCursor) open();

  void append(int16_t *data, int n);
  void append(DatagramIterator *src, int len=0x40000000);
  void append(const vector_uchar &);
  void done(); // A promise not to write any more samples.

 private:
  int read_samples(int n, int16_t *data);
  void update_cursor();
  int _desired_rate;
  int _desired_channels;
  UserDataAudioCursor *_cursor;
  pdeque<int16_t> _data;
  bool _aborted;
  bool _remove_after_read;
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
