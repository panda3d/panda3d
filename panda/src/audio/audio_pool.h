// Filename: audio_pool.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_POOL_H__
#define __AUDIO_POOL_H__

#include "audio_sound.h"
#include "audio_trait.h"
#include "config_audio.h"
#include <map>
#include <pandabase.h>
#include <filename.h>
#include <pointerTo.h>

class EXPCL_PANDA AudioPool {
private:
  INLINE AudioPool(void);

  bool ns_has_sound(Filename filename);
  AudioSound* ns_load_sound(Filename filename);
  void ns_release_sound(AudioSound* sound);
  void ns_release_all_sounds(void);
  string ns_get_nth_sound_name(int) const;

  static AudioPool* get_ptr(void);

  static AudioPool *_global_ptr;
  typedef map<string, PT(AudioTraits::SoundClass) > SoundMap;
  SoundMap _sounds;
public:
  typedef AudioTraits::SoundClass* SoundLoadFunc(Filename);

PUBLISHED:
  INLINE static bool has_sound(const string& filename);
  INLINE static bool verify_sound(const string& filename);
  INLINE static AudioSound* load_sound(const string& filename);
  INLINE static void release_sound(AudioSound* sound);
  INLINE static void release_all_sounds(void);
  INLINE static int get_num_loaded_sounds(void);
  INLINE static string get_nth_sound_name(int);
  static void register_sound_loader(const string&, SoundLoadFunc*);
};

#include "audio_pool.I"

#endif /* __AUDIO_POOL_H__ */
