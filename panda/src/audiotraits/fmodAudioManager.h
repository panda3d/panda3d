// Filename: fmodAudioManager.h
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

#ifndef __FMOD_AUDIO_MANAGER_H__
#define __FMOD_AUDIO_MANAGER_H__

#include <pandabase.h>
#ifdef HAVE_FMOD //[

#include "audioManager.h"
class FmodAudioSound;
#include "filename.h"
#include "pmap.h"
#include "pset.h"

#include <fmod.h>

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
  virtual void set_cache_limit(int);
  virtual int get_cache_limit();

  virtual void set_volume(float);
  virtual float get_volume();
  
  virtual void set_active(bool);
  virtual bool get_active();
  void stop_all_sounds();

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
    void *data; // the memory-mapped audio file.
  } SoundCacheEntry;
  typedef pmap<string, SoundCacheEntry > SoundMap;
  SoundMap _sounds;

  typedef pset<FmodAudioSound* > AudioSet;
  // The offspring of this manager:
  AudioSet _soundsOnLoan;

  void release_sound(FmodAudioSound *audioSound);

  static int _active_managers;
  bool _is_valid;
  bool _active;
  
  void* load(const Filename& filename, const size_t size) const;
  size_t get_file_size(const Filename& filename) const;

  friend FmodAudioSound;
};

EXPCL_FMOD_AUDIO PT(AudioManager) Create_AudioManager();

#endif //]

#endif /* __FMOD_AUDIO_MANAGER_H__ */
