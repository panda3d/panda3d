// Filename: audio_null_traits.h
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

// yes, this needs to be outside the ifdef protection
#include "audio_trait.h"

#ifdef AUDIO_USE_NULL

#ifndef __AUDIO_NULL_TRAITS_H__
#define __AUDIO_NULL_TRAITS_H__

class NullPlaying;

class EXPCL_PANDA NullSound : public AudioTraits::SoundClass {
public:
  INLINE NullSound(void);
  virtual ~NullSound(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeleteSoundFunc* get_destroy(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;
  static void destroy(AudioTraits::SoundClass*);
};

class EXPCL_PANDA NullPlaying : public AudioTraits::PlayingClass {
public:
  INLINE NullPlaying(AudioTraits::SoundClass*);
  virtual ~NullPlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA NullPlayer : public AudioTraits::PlayerClass {
public:
  INLINE NullPlayer(void);
  virtual ~NullPlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*);
  virtual void stop_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
};

#include "audio_null_traits.I"

#endif /* __AUDIO_NULL_TRAITS_H__ */
#endif /* AUDIO_USE_NULL */
