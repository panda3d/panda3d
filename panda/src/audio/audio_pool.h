// Filename: audio_pool.h
// Created by:  cary (22Sep00)
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

#ifndef __AUDIO_POOL_H__
#define __AUDIO_POOL_H__

#include "audio_sound.h"
#include "audio_trait.h"
#include "config_audio.h"
#include "pmap.h"
#include <pandabase.h>
#include <filename.h>
#include <pointerTo.h>

// The AudioPool is a cache of SoundClass objects.  When retrieving
// a sound, you will actually get an AudioSound object (which you own).
class EXPCL_PANDA AudioPool {
public:
  typedef AudioTraits::SoundClass* SoundLoadFunc(Filename);

PUBLISHED:
  INLINE static bool has_sound(const string& filename);
  INLINE static bool verify_sound(const string& filename);
  
  INLINE static AudioSound* load_sound(const string& filename);
  
  INLINE static void release_sound(AudioSound* sound);
  INLINE static void release_all_sounds();
  
  INLINE static int get_num_loaded_sounds();
  INLINE static string get_nth_sound_name(int);
  
  static void register_sound_loader(const string&, SoundLoadFunc*);

private:
  INLINE AudioPool();

  bool ns_has_sound(Filename filename);
  PT(AudioTraits::SoundClass) call_sound_loader(Filename fileName);
  AudioSound* ns_load_sound(Filename filename);
  void ns_release_sound(AudioSound* sound);
  void ns_release_all_sounds();
  string ns_get_nth_sound_name(int) const;

  static AudioPool* get_ptr();

  static AudioPool *_global_ptr;
  typedef pmap<string, PT(AudioTraits::SoundClass) > SoundMap;
  SoundMap _sounds;
};

#include "audio_pool.I"

#endif /* __AUDIO_POOL_H__ */
