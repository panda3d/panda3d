// Filename: fmodAudioManager.h
// Created by:  cort (January 22, 2003)
// Prior system by: cary
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

#ifndef __FMOD_AUDIO_MANAGER_H__
#define __FMOD_AUDIO_MANAGER_H__

#include "pandabase.h"
#ifdef HAVE_FMOD //[

#include "audioManager.h"
class FmodAudioSound;
#include "filename.h"
#include "pdeque.h"
#include "pmap.h"
#include "pset.h"

#include <fmod.h> // Is fmod.h really a system file?  I think maybe this should be "fmod.h".

class EXPCL_FMOD_AUDIO FmodAudioManager : public AudioManager {
  // All of these methods are stubbed out to some degree.
  // If you're looking for a starting place for a new AudioManager,
  // please consider looking at the milesAudioManager.
  
public:
  FmodAudioManager();
  virtual ~FmodAudioManager();
  
  virtual bool is_valid();
  
  virtual PT(AudioSound) get_sound(const string&);
  virtual void uncache_sound(const string&);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual int get_cache_limit();

  // Indicates that the given sound was the most recently used.
  void most_recently_used(const string& path);

  // Uncaches the least recently used sound.
  void uncache_a_sound();

  virtual void set_volume(float);
  virtual float get_volume();
  
  virtual void set_active(bool);
  virtual bool get_active();

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;

  virtual void reduce_sounds_playing_to(unsigned int count);

  virtual void stop_all_sounds();

protected:
  // increment or decrement the refcount of the given file's cache entry.
  // sounds can only be uncached when their refcounts are zero.
  void inc_refcount(const string& file_name);
  void dec_refcount(const string& file_name);
private:
  typedef struct {
    size_t size; // size of the data field, in bytes
    unsigned int refcount; // how many AudioSound objects are referencing me?
    bool stale; // can this entry be  purged from the cache?
    char *data; // the memory-mapped audio file.
  } SoundCacheEntry;
  typedef pmap<string, SoundCacheEntry > SoundMap;
  SoundMap _sounds;

  typedef pset<FmodAudioSound* > AudioSet;
  // The offspring of this manager:
  AudioSet _soundsOnLoan;
  unsigned int _concurrent_sound_limit;

  // The Least Recently Used mechanism:
  typedef pdeque<string> LRU;
  LRU _lru;

  void release_sound(FmodAudioSound *audioSound);

  int _cache_limit;
  static int _active_managers;
  bool _is_valid;
  bool _active;
  
  char* load(const Filename& filename, size_t &size) const;

  friend class FmodAudioSound;
};

EXPCL_FMOD_AUDIO PT(AudioManager) Create_AudioManager();

#endif //]

#endif /* __FMOD_AUDIO_MANAGER_H__ */
