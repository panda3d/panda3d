// Filename: fmodAudioSound.h
// Created by:  cort (January 22, 2003)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __FMOD_AUDIO_SOUND_H__
#define __FMOD_AUDIO_SOUND_H__

#include <pandabase.h>
#ifdef HAVE_FMOD //[

#include "audioSound.h"
class FmodAudioManager;
#include <fmod.h>


class EXPCL_FMOD_AUDIO FmodAudioSound : public AudioSound {
public:
  ~FmodAudioSound();
  
  // For best compatability, set the loop_count, start_time,
  // volume, and balance, prior to calling play().  You may
  // set them while they're playing, but it's implementation
  // specific whether you get the results.
  void play();
  void stop();
  
  // loop: false = play once; true = play forever.
  // inits to false.
  void set_loop(bool loop=true);
  bool get_loop() const;
  
  // loop_count: 0 = forever; 1 = play once; n = play n times.
  // inits to 1.
  void set_loop_count(unsigned long loop_count=1);
  unsigned long get_loop_count() const;
  
  // 0 = begining; length() = end.
  // inits to 0.0.
  void set_time(float start_time=0.0);
  float get_time() const;
  
  // 0 = minimum; 1.0 = maximum.
  // inits to 1.0.
  void set_volume(float volume=1.0);
  float get_volume() const;
  
  // -1.0 is hard left
  // 0.0 is centered
  // 1.0 is hard right
  // inits to 0.0.
  void set_balance(float balance_right=0.0);
  float get_balance() const;

  // inits to manager's state.
  void set_active(bool active=true);
  bool get_active() const;
  
  const string& get_name() const;
  
  // return: playing time in seconds.
  float length() const;

  AudioSound::SoundStatus status() const;

protected:

private:
  PT(FmodAudioManager) _manager;
  FSOUND_STREAM *_audio;
  string _file_name;
  float _volume; // 0..1.0
  float _balance; // -1..1
  unsigned long _loop_count;
  mutable float _length; // in seconds.
  bool _active;
  bool _paused;
  int _channel;

  FmodAudioSound(FmodAudioManager* manager, FSOUND_STREAM *audio_data,
		 string file_name, float length=0.0f);

  // forbidden functions!
  FmodAudioSound(const FmodAudioSound& rhs) {}
  const FmodAudioSound& operator=(const FmodAudioSound& rhs) { return *this; }

  friend class FmodAudioManager;
};

#include "fmodAudioSound.I"

#endif //]

#endif /* __FMOD_AUDIO_SOUND_H__ */
