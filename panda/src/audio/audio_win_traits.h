// Filename: audio_win_traits.h
// Created by:  cary (27Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_WIN_TRAITS_H__
#define __AUDIO_WIN_TRAITS_H__

#include "audio_trait.h"
#include <filename.h>

#include <windows.h>
#include <dsound.h>
#include <dmusici.h>

class EXPCL_PANDA WinSample : public AudioTraits::SampleClass {
private:
  LPDIRECTSOUNDBUFFER _channel;
  BYTE* _data;
  DWORD _len;
public:
  INLINE WinSample(void);
  virtual ~WinSample(void);

  virtual float length(void);
  virtual AudioTraits::SampleClass::SampleStatus status(void);
public:
  // these are used by the laoders
  BYTE* lock(void);
  void  unlock(void);
  static WinSample* load_wav(Filename);
  static void destroy(AudioTraits::SampleClass*);
  // these are used by the player
  INLINE LPDIRECTSOUNDBUFFER get_channel(void);
};

class EXPCL_PANDA WinMusic : public AudioTraits::MusicClass {
private:
  IDirectMusicPerformance* _performance;
  IDirectMusicSegment* _music;
  IDirectSoundBuffer* _buffer;
  IDirectMusicPort* _synth;
  BYTE* _data;
  DWORD _len;

  void init(void);
public:
  INLINE WinMusic(void);
  virtual ~WinMusic(void);

  virtual AudioTraits::MusicClass::MusicStatus status(void);
  // these are used by the loaders
  static WinMusic* load_midi(Filename);
  static void destroy(AudioTraits::MusicClass*);
  // these are used by the players
  INLINE IDirectMusicPerformance* get_performance(void);
  INLINE IDirectMusicSegment* get_music(void);
};

class EXPCL_PANDA WinPlayer : public AudioTraits::PlayerClass {
public:
  INLINE WinPlayer(void);
  virtual ~WinPlayer(void);

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
public:
  // used by the readers
  static WinPlayer* get_instance(void);
private:
  static WinPlayer* _global_instance;
};

#include "audio_win_traits.I"

#endif /* __AUDIO_WIN_TRAITS_H__ */
