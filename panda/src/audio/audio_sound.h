// Filename: audio_sound.h
// Created by:  cary (17Oct00)
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
PUBLISHED:
  virtual ~AudioSound();
  INLINE bool operator==(const AudioSound&) const;
  INLINE bool operator!=(const AudioSound&) const;

  enum Category { EFFECT, MUSIC }; // sync with AudioTraits::PlayingClass.
  enum SoundStatus { BAD, READY, PLAYING };

  float length() const;
  SoundStatus status() const;
  
  void set_category(Category category) {
    _state->set_category(
      (AudioTraits::PlayingClass::Category)(category)
    );
  }
  Category get_category() const {
    return (Category)(_state->get_category());
  }

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

  INLINE AudioTraits::PlayerClass* get_player() const;
  INLINE AudioTraits::SoundClass* get_sound() const;
  INLINE AudioTraits::PlayingClass* get_state() const;

  friend class AudioPool;
  friend class AudioManager;

public:
  // type stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    /*
    TypedObject::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedObject::get_class_type());
    */
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "audio_sound.I"

#endif /* __AUDIO_SOUND_H__ */
