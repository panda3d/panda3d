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
  protected:
    SoundClass* _sound;
    float _volume;
  public:
    PlayingClass(SoundClass* s) : _sound(s), _volume(1.) {}
    virtual ~PlayingClass();

    enum PlayingStatus { BAD, READY, PLAYING } ;

    virtual PlayingStatus status() = 0;
    INLINE void set_volume(float v) { _volume = v; }
    INLINE float get_volume() const { return _volume; }
  };
  class EXPCL_PANDA PlayerClass {
  public:
    PlayerClass() {}
    virtual ~PlayerClass();

    virtual void play_sound(SoundClass*, PlayingClass*, float) = 0;
    virtual void stop_sound(SoundClass*, PlayingClass*) = 0;
    virtual void set_volume(PlayingClass*, float) = 0;
    virtual bool adjust_volume(PlayingClass*) = 0;
  };
};

// this is really ugly.  But since we have to be able to include/compile
// all of the driver files on any system, I need to centralize a switch
// for which one is real.
#ifdef HAVE_SYS_SOUNDCARD_H
#define AUDIO_USE_LINUX
#elif defined(WIN32_VC)
#define AUDIO_USE_WIN32
#elif defined(HAVE_MIKMOD)
#define AUDIO_USE_MIKMOD
#else
#define AUDIO_USE_NULL
#endif

#endif /* __AUDIO_TRAIT_H__ */
