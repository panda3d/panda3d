// Filename: audio_sound.h
// Created by:  cary (17Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_SOUND_H__
#define __AUDIO_SOUND_H__

#include "audio_trait.h"
#include <typedReferenceCount.h>
#include <typedObject.h>
#include <namable.h>
#include <pointerTo.h>


class AudioPool;
class AudioManager;

/*
class EXPCL_PANDA AudioSound : public TypedObject, public Namable {
*/
class EXPCL_PANDA AudioSound : public TypedReferenceCount, public Namable {
private:
  PT(AudioTraits::SoundClass) _sound;
  AudioTraits::PlayingClass *_state;
  AudioTraits::PlayerClass *_player;
  AudioTraits::DeletePlayingFunc *_delstate;
protected:
  INLINE AudioSound(AudioTraits::SoundClass*, AudioTraits::PlayingClass*,
                    AudioTraits::PlayerClass*, AudioTraits::DeletePlayingFunc*,
                    const string&);
  INLINE AudioSound(const AudioSound&);
  INLINE AudioSound& operator=(const AudioSound&);

  INLINE AudioTraits::PlayerClass* get_player(void) const;
  INLINE AudioTraits::SoundClass* get_sound(void) const;
  INLINE AudioTraits::PlayingClass* get_state(void) const;

  friend class AudioPool;
  friend class AudioManager;
PUBLISHED:
  virtual ~AudioSound(void);
  INLINE bool operator==(const AudioSound&) const;
  INLINE bool operator!=(const AudioSound&) const;

  enum SoundStatus { BAD, READY, PLAYING } ;

  float length(void) const;
  SoundStatus status(void) const;
public:
  // type stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    /*
    TypedObject::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedObject::get_class_type());
    */
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "audio_sound.I"

#endif /* __AUDIO_SOUND_H__ */
