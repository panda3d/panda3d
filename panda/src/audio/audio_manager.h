// Filename: audio_manager.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "audio_trait.h"
#include "audio_sample.h"
#include "audio_music.h"

#include <ipc_mutex.h>
#include <ipc_thread.h>

class EXPCL_PANDA AudioManager {
private:
  INLINE AudioManager(void);

  void ns_play(AudioSample*);
  void ns_play(AudioMusic*);
  void ns_spawn_update(void);
  void ns_shutdown(void);
  void ns_set_volume(AudioSample*, int);
  void ns_set_volume(AudioMusic*, int);

  static AudioManager* get_ptr(void);
  static void* spawned_update(void*);

  typedef void UpdateFunc(void);
  typedef void ShutdownFunc(void);
  static AudioManager* _global_ptr;
  static UpdateFunc* _update_func;
  static ShutdownFunc* _shutdown_func;
  static mutex _manager_mutex;
  static bool* _quit;
  static thread* _spawned;
public:
  virtual ~AudioManager(void);

  static void set_update_func(UpdateFunc*);
  static void set_shutdown_func(ShutdownFunc*);

  INLINE static void play(AudioSample*);
  INLINE static void play(AudioMusic*);
  INLINE static void update(void);
  INLINE static void spawn_update(void);
  INLINE static void shutdown(void);
  INLINE static void set_volume(AudioSample*, int);
  INLINE static void set_volume(AudioMusic*, int);
};

#include "audio_manager.I"

#endif /* __AUDIO_MANAGER_H__ */
