// Filename: milesAudioManager.h
// Created by:  skyler (June 6, 2001)
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

#ifndef __MILES_AUDIO_MANAGER_H__ //[
#define __MILES_AUDIO_MANAGER_H__

#include <pandabase.h>
#ifdef HAVE_RAD_MSS //[

#include "audioManager.h"
#include "milesAudioSound.h"

class EXPCL_MILES_AUDIO MilesAudioManager: public AudioManager {
public:
  MilesAudioManager();
  ~MilesAudioManager();

  PT(AudioSound) get_sound(const string& file_name);
  void drop_sound(const string& file_name);

  void set_volume(float volume);
  float get_volume();
  
  void set_active(bool active);
  bool get_active();

private:
  typedef pmap<string, HAUDIO > SoundMap;
  SoundMap _sounds;
  typedef pset<MilesAudioSound* > AudioSet;
  AudioSet _soundsOnLoan;
  float _volume;
  bool _active;
  // keep a count for startup and shutdown:
  static int _active_managers;
  static HDLSFILEID _dls_field;
  
  HAUDIO load(Filename file_name);
  // Tell the manager that the sound dtor was called.
  void release_sound(MilesAudioSound* audioSound);
  
  friend MilesAudioSound;
};

EXPCL_MILES_AUDIO PT(AudioManager) Create_AudioManager();


#endif //]

#endif //]


