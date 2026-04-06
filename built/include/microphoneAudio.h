/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file microphoneAudio.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef MICROPHONEAUDIO_H
#define MICROPHONEAUDIO_H

#include "movieAudio.h"
class MovieAudio;
class MovieAudioCursor;

/**
 * Class MicrophoneAudio provides the means to read raw audio samples from a
 * microphone.
 */
class EXPCL_PANDA_MOVIES MicrophoneAudio : public MovieAudio {

 PUBLISHED:
  virtual ~MicrophoneAudio();

  static int                 get_num_options();
  static PT(MicrophoneAudio) get_option(int n);
  MAKE_SEQ(get_options, get_num_options, get_option);

  INLINE int get_channels() const;
  INLINE int get_rate() const;

  MAKE_SEQ_PROPERTY(options, get_num_options, get_option);
  MAKE_PROPERTY(channels, get_channels);
  MAKE_PROPERTY(rate, get_rate);

  virtual PT(MovieAudioCursor) open() = 0;

public:
  static void find_all_microphones();

protected:
  int _channels;
  int _rate;

  static pvector<PT(MicrophoneAudio)> _all_microphones;

 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudio::init_type();
    register_type(_type_handle, "MicrophoneAudio",
                  MovieAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "microphoneAudio.I"

#endif
