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

#include "pandabase.h"
#ifdef HAVE_RAD_MSS //[

#include "audioManager.h"
#include "mss.h"
#include "pset.h"
#include "pmap.h"
#include "pdeque.h"

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
  void set_cache_limit(unsigned int count);
  unsigned int get_cache_limit() const;

  void set_volume(float volume);
  float get_volume() const;
  
  void set_active(bool active);
  bool get_active() const;

  void set_concurrent_sound_limit(unsigned int limit = 0);
  unsigned int get_concurrent_sound_limit() const;

  void reduce_sounds_playing_to(unsigned int count);

  void stop_all_sounds();

  // Optional Downloadable Sound field for software midi:
  // made public so C atexit fn can access it
  static HDLSFILEID _dls_field;

private:
  // The sound cache:
  typedef pmap<string, HAUDIO > SoundMap;
  SoundMap _sounds;

  typedef pset<MilesAudioSound* > AudioSet;
  // The offspring of this manager:
  AudioSet _sounds_on_loan;

  typedef pset<MilesAudioSound* > SoundsPlaying;
  // The sounds from this manager that are currently playing:
  SoundsPlaying _sounds_playing;

  // The Least Recently Used mechanism:
  typedef pdeque<const string* > LRU;
  LRU _lru;
  // State:
  float _volume;
  bool _active;
  int _cache_limit;
  // keep a count for startup and shutdown:
  static int _active_managers;
  unsigned int _concurrent_sound_limit;
  
  bool _is_valid;
  bool _hasMidiSounds;
  
  HAUDIO load(Filename file_name);
  // Tell the manager that the sound dtor was called.
  void release_sound(MilesAudioSound* audioSound);
  
  void most_recently_used(const string& path);
  void uncache_a_sound();

  void starting_sound(MilesAudioSound* audio);
  void stoping_sound(MilesAudioSound* audio);

  // utility function that should be moved to another class:
  bool get_registry_entry(HKEY base, 
                          const char* subKeyName, 
                          const char* keyName, 
                          string& result);
  // get the default dls file path:
  void get_gm_file_path(string& result);

  void force_midi_reset();

  // These are "callback" functions that implement vfs-style I/O for
  // Miles.
  static U32 AILCALLBACK vfs_open_callback(const char *filename, U32 *file_handle);
  static U32 AILCALLBACK vfs_read_callback(U32 file_handle, void *buffer, U32 bytes);
  static S32 AILCALLBACK vfs_seek_callback(U32 file_handle, S32 offset, U32 type);
  static void AILCALLBACK vfs_close_callback(U32 file_handle);
  
  friend class MilesAudioSound;
};

EXPCL_MILES_AUDIO PT(AudioManager) Create_AudioManager();


#endif //]

#endif //]


