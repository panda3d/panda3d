// Filename: audio_pool.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_POOL_H__
#define __AUDIO_POOL_H__

#include "audio_sample.h"
#include "audio_music.h"
#include <map>
#include <pandabase.h>
#include <filename.h>
#include <pointerTo.h>

class EXPCL_PANDA AudioPool {
private:
  INLINE AudioPool(void);

  bool ns_has_sample(Filename filename);
  AudioSample* ns_load_sample(Filename filename);
  void ns_release_sample(AudioSample* sample);
  void ns_release_all_samples(void);

  bool ns_has_music(Filename filename);
  AudioMusic* ns_load_music(Filename filename);
  void ns_release_music(AudioMusic* music);
  void ns_release_all_music(void);

  static AudioPool* get_ptr(void);

  static AudioPool *_global_ptr;
  typedef map<string, PT(AudioSample) > SampleMap;
  SampleMap _samples;
  typedef map<string, PT(AudioMusic) > MusicMap;
  MusicMap _music;
public:
  typedef void SampleLoadFunc(AudioTraits::SampleClass**,
			      AudioTraits::PlayerClass**,
			      AudioTraits::DeleteSampleFunc**, Filename);

  INLINE static bool has_sample(const string& filename);
  INLINE static bool verify_sample(const string& filename);
  INLINE static AudioSample* load_sample(const string& filename);
  INLINE static void release_sample(AudioSample* sample);
  INLINE static void release_all_samples(void);
  static void register_sample_loader(const string&, SampleLoadFunc*);

  typedef void MusicLoadFunc(AudioTraits::MusicClass**,
			     AudioTraits::PlayerClass**,
			     AudioTraits::DeleteMusicFunc**, Filename);

  INLINE static bool has_music(const string& filename);
  INLINE static bool verify_music(const string& filename);
  INLINE static AudioMusic* load_music(const string& filename);
  INLINE static void release_music(AudioMusic* music);
  INLINE static void release_all_music(void);
  static void register_music_loader(const string&, MusicLoadFunc*);
};

#include "audio_pool.I"

#endif /* __AUDIO_POOL_H__ */
