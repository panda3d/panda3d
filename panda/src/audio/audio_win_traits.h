// Filename: audio_win_traits.h
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

#ifdef AUDIO_USE_WIN32
#ifndef __AUDIO_WIN_TRAITS_H__
#define __AUDIO_WIN_TRAITS_H__

#error You should try the Miles version.

#include <filename.h>

#include <windows.h>
#include <dsound.h>
#include <dmusici.h>
#include "config_audio.h"

class WinSamplePlaying;

class EXPCL_PANDA WinSample : public AudioTraits::SoundClass {
public:
  BYTE* _data;
  DWORD _len;
  WAVEFORMATEX _info;
public:
  INLINE WinSample();
  virtual ~WinSample();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;

  // used by play_sound
  INLINE DWORD get_length() const;
  INLINE WAVEFORMATEX get_format() const;
public:
  static WinSample* load_wav(Filename);
  static WinSample* load_raw(unsigned char*, unsigned long);
};

class EXPCL_PANDA WinMusic : public AudioTraits::SoundClass {
private:
  IDirectMusicPerformance* _performance;
  IDirectMusicSegment* _music;
  IDirectSoundBuffer* _buffer;
  IDirectMusicPort* _synth;

  void init();
public:
  INLINE WinMusic();
  virtual ~WinMusic();

  virtual float length() const;
  virtual AudioTraits::PlayingClass* get_state() const;
  virtual AudioTraits::PlayerClass* get_player() const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate() const;
  // these are used by the loaders
  static WinMusic* load_midi(Filename);
  // these are used by the players
  INLINE IDirectMusicPerformance* get_performance() const;
  INLINE IDirectMusicSegment* get_music();
};

class EXPCL_PANDA WinSamplePlaying : public AudioTraits::PlayingClass {
private:
  LPDIRECTSOUNDBUFFER _channel;
  BYTE* _data;
public:
  WinSamplePlaying(AudioTraits::SoundClass*);
  ~WinSamplePlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
  static void destroy(AudioTraits::PlayingClass*);
  // these are used by the laoders
  BYTE* lock();
  void  unlock();
  // these are used by the player
  INLINE LPDIRECTSOUNDBUFFER get_channel();
};

class EXPCL_PANDA WinMusicPlaying : public AudioTraits::PlayingClass {
public:
  WinMusicPlaying(AudioTraits::SoundClass*);
  ~WinMusicPlaying();

  virtual AudioTraits::PlayingClass::PlayingStatus status();
  static void destroy(AudioTraits::PlayingClass*);
  INLINE IDirectMusicPerformance* get_performance() const;
};

class EXPCL_PANDA WinSamplePlayer : public AudioTraits::PlayerClass {
public:
  INLINE WinSamplePlayer();
  virtual ~WinSamplePlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static WinSamplePlayer* get_instance();
private:
  static WinSamplePlayer* _global_instance;
};

class EXPCL_PANDA WinMusicPlayer : public AudioTraits::PlayerClass {
public:
  INLINE WinMusicPlayer();
  virtual ~WinMusicPlayer();

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static WinMusicPlayer* get_instance();
private:
  static WinMusicPlayer* _global_instance;
};

#include "audio_win_traits.I"

#endif /* __AUDIO_WIN_TRAITS_H__ */
#endif /* AUDIO_USE_WIN32 */
