// Filename: audio_manager.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "audio_trait.h"
#include "audio_sound.h"
#include "config_audio.h"

#include <ipc_mutex.h>
#include <ipc_thread.h>
#include <set>

class EXPCL_PANDA AudioManager {
private:
  INLINE AudioManager(void);

  void ns_play(AudioSound*);
  void ns_stop(AudioSound*);
  void ns_set_loop(AudioSound*, bool);
  bool ns_get_loop(AudioSound*);
  void ns_set_volume(AudioSound*, float);
  void ns_spawn_update(void);
  void ns_shutdown(void);
  void ns_update(void);

  static AudioManager* get_ptr(void);
  static void* spawned_update(void*);

  typedef void UpdateFunc(void);
  typedef void ShutdownFunc(void);
  typedef set<PT(AudioSound)> LoopSet;
  static AudioManager* _global_ptr;
  static UpdateFunc* _update_func;
  static ShutdownFunc* _shutdown_func;
  static mutex _manager_mutex;
  static bool* _quit;
  static thread* _spawned;
  static LoopSet* _loopset;
public:
  virtual ~AudioManager(void);

  static void set_update_func(UpdateFunc*);
  static void set_shutdown_func(ShutdownFunc*);

PUBLISHED:
  INLINE static void play(AudioSound*);
  INLINE static void stop(AudioSound*);
  INLINE static void set_loop(AudioSound*, bool);
  INLINE static bool get_loop(AudioSound*);
  INLINE static void set_volume(AudioSound*, float);
  INLINE static void update(void);
  INLINE static void spawn_update(void);
  INLINE static void shutdown(void);
};

#include "audio_manager.I"

#endif /* __AUDIO_MANAGER_H__ */
