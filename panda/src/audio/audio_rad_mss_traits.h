// Filename: audio_rad_mss_traits.h
// Created by:  cary (27Sep00)
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

#ifdef AUDIO_USE_RAD_MSS //[
#ifndef __AUDIO_RAD_MSS_TRAITS_H__
#define __AUDIO_RAD_MSS_TRAITS_H__

#include <filename.h>

#include <windows.h>
#include <dsound.h>
#include <dmusici.h>
#include "config_audio.h"
#include "mss.h"

class MilesPlaying;

class EXPCL_PANDA MilesSound : public AudioTraits::SoundClass {
public:
  INLINE MilesSound(HAUDIO audio);
  virtual ~MilesSound();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;

public:
  static MilesSound* load(Filename);
  static MilesSound* load_raw(unsigned char* data, unsigned long size);
private:
  HAUDIO _audio;
  friend MilesPlaying;
};

class MilesPlayer;

class EXPCL_PANDA MilesPlaying : public AudioTraits::PlayingClass {
public:
  MilesPlaying(AudioTraits::SoundClass*);
  ~MilesPlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
  static void destroy(AudioTraits::PlayingClass*);

private:
  HAUDIO _audio;
  friend MilesPlayer;
};

class EXPCL_PANDA MilesPlayer : public AudioTraits::PlayerClass {
public:
  INLINE MilesPlayer();
  virtual ~MilesPlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, 
                          float start_time,
                          int loop);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float volume);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static MilesPlayer* get_instance();
private:
  static MilesPlayer* _global_instance;
};

#include "audio_rad_mss_traits.I"

#endif /* __AUDIO_RAD_MSS_TRAITS_H__ */
#endif //]
