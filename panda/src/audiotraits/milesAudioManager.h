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
#include "mss.h"
#include "pset.h"
#include "pmap.h"

class MilesAudioSound;

class EXPCL_MILES_AUDIO MilesAudioManager: public AudioManager {
public:
  // See AudioManager.h for documentation.
  
  MilesAudioManager();
  ~MilesAudioManager();

  bool is_valid();
  
  PT(AudioSound) get_sound(const string& file_name);
  void uncache_sound(const string& file_name);
  void clear_cache();
  void set_cache_limit(int count);
  int get_cache_limit();

  void set_volume(float volume);
  float get_volume();
  
  void set_active(bool active);
  bool get_active();

private:
  // The sound cache/pool:
  typedef pmap<string, HAUDIO > SoundMap;
  SoundMap _sounds;
  // The offspring of this manager:
  typedef pset<MilesAudioSound* > AudioSet;
  AudioSet _soundsOnLoan;
  // State:
  float _volume;
  bool _active;
  int _cache_limit;
  // keep a count for startup and shutdown:
  static int _active_managers;
  // Optional Downloadable Sound field for software midi:
  static HDLSFILEID _dls_field;
  
  bool _is_valid;
  
  HAUDIO load(Filename file_name);
  // Tell the manager that the sound dtor was called.
  void release_sound(MilesAudioSound* audioSound);
  
  void uncache_a_sound();

  // utility function that should be moved to another class:
  void get_registry_entry(HKEY base, 
                          const char* subKeyName, 
                          const char* keyName, 
                          string& result);
  // get the default dls file path:
  void get_gm_file_path(string& result);
  
  friend MilesAudioSound;
};

EXPCL_MILES_AUDIO PT(AudioManager) Create_AudioManager();


#endif //]

#endif //]


