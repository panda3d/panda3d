// Filename: audio_trait.h
// Created by:  frang (06Jul00)
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

#ifndef __AUDIO_TRAIT_H__
#define __AUDIO_TRAIT_H__

#include <pandabase.h>
#include <referenceCount.h>

#if ! defined(HAVE_RAD_MSS)
  #error where is rad mss?
#endif

class EXPCL_PANDA AudioTraits {
public:
  class SoundClass;
  class PlayingClass;
  class PlayerClass;

  typedef void DeleteSoundFunc(SoundClass*);
  typedef void DeletePlayingFunc(PlayingClass*);

  class EXPCL_PANDA SoundClass : public ReferenceCount {
  public:
    SoundClass() {}
    virtual ~SoundClass();

    virtual float length() const = 0;
    virtual PlayingClass* get_state() const = 0;
    virtual PlayerClass* get_player() const = 0;
    virtual DeletePlayingFunc* get_delstate() const = 0;
  };
  class EXPCL_PANDA PlayingClass {
  public:
    PlayingClass(SoundClass* s) 
        : _sound(s), _volume(1.0), _category(EFFECT) {}
    virtual ~PlayingClass();

    enum Category { EFFECT, MUSIC }; // sync with AudioSound.
    
    enum PlayingStatus { BAD, READY, PLAYING };

    virtual PlayingStatus status() = 0;
    INLINE void set_volume(float v) { _volume = v; }
    INLINE float get_volume() const { return _volume; }
    
    void set_category(Category category) { _category=category; }
    Category get_category() const { return _category; }
  protected:
    SoundClass* _sound;
    float _volume;
    Category _category;
  };
  class EXPCL_PANDA PlayerClass {
  public:
    PlayerClass() {}
    virtual ~PlayerClass();

    virtual void play_sound(SoundClass*, PlayingClass*, float start_time, int loop) = 0;
    virtual void stop_sound(SoundClass*, PlayingClass*) = 0;
    virtual void set_volume(PlayingClass*, float) = 0;
    virtual bool adjust_volume(PlayingClass*) = 0;
  };
};

#endif /* __AUDIO_TRAIT_H__ */
