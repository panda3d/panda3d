// Filename: audio_null_traits.h
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_NULL_TRAITS_H__
#define __AUDIO_NULL_TRAITS_H__

#include "audio_trait.h"

class EXPCL_PANDA NullSample : public AudioTraits::SampleClass {
public:
  INLINE NullSample(void);
  virtual ~NullSample(void);

  virtual float length(void);
  virtual AudioTraits::SampleClass::SampleStatus status(void);
};

class EXPCL_PANDA NullMusic : public AudioTraits::MusicClass {
public:
  INLINE NullMusic(void);
  virtual ~NullMusic(void);

  virtual AudioTraits::MusicClass::MusicStatus status(void);
};

class EXPCL_PANDA NullPlayer : public AudioTraits::PlayerClass {
public:
  INLINE NullPlayer(void);
  virtual ~NullPlayer(void);

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
};

#include "audio_null_traits.I"

#endif /* __AUDIO_NULL_TRAITS_H__ */
