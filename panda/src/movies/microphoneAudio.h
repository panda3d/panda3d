// Filename: microphoneAudio.h
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

#ifndef MICROPHONEAUDIO_H
#define MICROPHONEAUDIO_H

#include "movieAudio.h"
class MovieAudio;
class MovieAudioCursor;

////////////////////////////////////////////////////////////////////
//       Class : MicrophoneAudio
// Description : Class MicrophoneAudio provides the means to read
//               raw audio samples from a microphone.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES MicrophoneAudio : public MovieAudio {

 PUBLISHED:
  virtual ~MicrophoneAudio();

  static int                 get_num_options();
  static PT(MicrophoneAudio) get_option(int n);
  
  INLINE int get_channels() const;
  INLINE int get_rate() const;
  
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
    TypedWritableReferenceCount::init_type();
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
