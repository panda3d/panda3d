// Filename: audio_manager.h
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

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "audio_trait.h"
#include "audio_sound.h"
#include "config_audio.h"

#include <ipc_mutex.h>
#include <ipc_thread.h>
#include "pset.h"

class EXPCL_PANDA AudioManager {
private:
  INLINE AudioManager();

  void copy_loopset();
  void ns_play(AudioSound*, float);
  void ns_stop(AudioSound*);
  void ns_set_loop(AudioSound*, bool);
  bool ns_get_loop(AudioSound*);
  void ns_set_volume(AudioSound*, float);
  void ns_spawn_update();
  void ns_shutdown();
  void ns_update();

  static AudioManager* get_ptr();
  static void* spawned_update(void*);

  typedef void UpdateFunc();
  typedef void ShutdownFunc();
  typedef pset<AudioSound*> LoopSet;
  static AudioManager* _global_ptr;
  static UpdateFunc* _update_func;
  static ShutdownFunc* _shutdown_func;
  static mutex _manager_mutex;
  static bool* _quit;
  static thread* _spawned;
  static LoopSet* _loopset;
  static LoopSet* _loopcopy;
  static bool _sfx_active;
  static bool _music_active;
  static bool _hard_sfx_active;
  static bool _hard_music_active;
  static float _master_sfx_volume;
  static float _master_music_volume;
  static bool _master_volume_change;
public:
  virtual ~AudioManager();

  static void set_update_func(UpdateFunc*);
  static void set_shutdown_func(ShutdownFunc*);

PUBLISHED:
  INLINE static void play(AudioSound*, float = 0.);
  INLINE static void stop(AudioSound*);
  INLINE static void set_loop(AudioSound*, bool);
  INLINE static bool get_loop(AudioSound*);
  INLINE static void set_volume(AudioSound*, float);
  INLINE static void update();
  INLINE static void spawn_update();
  INLINE static void shutdown();
  INLINE static void set_master_sfx_volume(float);
  INLINE static void set_master_music_volume(float);
  INLINE static float get_master_sfx_volume();
  INLINE static float get_master_music_volume();
  INLINE static void set_all_sound_active(bool);
  INLINE static bool get_all_sound_active();
  INLINE static void set_sfx_active(bool);
  INLINE static void set_music_active(bool);
  INLINE static bool get_sfx_active();
  INLINE static bool get_music_active();
public:
  INLINE static void set_hard_sfx_active(bool);
  INLINE static void set_hard_music_active(bool);
};

#include "audio_manager.I"

#endif /* __AUDIO_MANAGER_H__ */
