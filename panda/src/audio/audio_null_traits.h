// Filename: audio_null_traits.h
// Created by:  cary (25Sep00)
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

// yes, this needs to be outside the ifdef protection
#include "audio_trait.h"

#ifdef AUDIO_USE_NULL

#ifndef __AUDIO_NULL_TRAITS_H__
#define __AUDIO_NULL_TRAITS_H__

class NullPlaying;

class EXPCL_PANDA NullSound : public AudioTraits::SoundClass {
public:
  INLINE NullSound();
  virtual ~NullSound();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;
};

class EXPCL_PANDA NullPlaying : public AudioTraits::PlayingClass {
public:
  INLINE NullPlaying(AudioTraits::SoundClass*);
  virtual ~NullPlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA NullPlayer : public AudioTraits::PlayerClass {
public:
  INLINE NullPlayer();
  virtual ~NullPlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float, int);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
};

#include "audio_null_traits.I"

#endif /* __AUDIO_NULL_TRAITS_H__ */
#endif /* AUDIO_USE_NULL */
