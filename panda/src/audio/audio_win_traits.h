// Filename: audio_win_traits.h
// Created by:  cary (27Sep00)
// 
////////////////////////////////////////////////////////////////////

// yes, this needs to be outside the ifdef protection
#include "audio_trait.h"

#ifdef AUDIO_USE_WIN32
#ifndef __AUDIO_WIN_TRAITS_H__
#define __AUDIO_WIN_TRAITS_H__

#include <filename.h>

#include <windows.h>
#include <dsound.h>
#include <dmusici.h>

class WinSamplePlaying;

class EXPCL_PANDA WinSample : public AudioTraits::SoundClass {
public:
  BYTE* _data;
  DWORD _len;
  WAVEFORMATEX _info;
public:
  INLINE WinSample(void);
  virtual ~WinSample(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;

  // used by play_sound
  INLINE DWORD get_length(void) const;
  INLINE WAVEFORMATEX get_format(void) const;
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

  void init(void);
public:
  INLINE WinMusic(void);
  virtual ~WinMusic(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;
  // these are used by the loaders
  static WinMusic* load_midi(Filename);
  // these are used by the players
  INLINE IDirectMusicPerformance* get_performance(void) const;
  INLINE IDirectMusicSegment* get_music(void);
};

class EXPCL_PANDA WinSamplePlaying : public AudioTraits::PlayingClass {
private:
  LPDIRECTSOUNDBUFFER _channel;
  BYTE* _data;
public:
  WinSamplePlaying(AudioTraits::SoundClass*);
  ~WinSamplePlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
  // these are used by the laoders
  BYTE* lock(void);
  void  unlock(void);
  // these are used by the player
  INLINE LPDIRECTSOUNDBUFFER get_channel(void);
};

class EXPCL_PANDA WinMusicPlaying : public AudioTraits::PlayingClass {
public:
  WinMusicPlaying(AudioTraits::SoundClass*);
  ~WinMusicPlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
  INLINE IDirectMusicPerformance* get_performance(void) const;
};

class EXPCL_PANDA WinSamplePlayer : public AudioTraits::PlayerClass {
public:
  INLINE WinSamplePlayer(void);
  virtual ~WinSamplePlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static WinSamplePlayer* get_instance(void);
private:
  static WinSamplePlayer* _global_instance;
};

class EXPCL_PANDA WinMusicPlayer : public AudioTraits::PlayerClass {
public:
  INLINE WinMusicPlayer(void);
  virtual ~WinMusicPlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
                          AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static WinMusicPlayer* get_instance(void);
private:
  static WinMusicPlayer* _global_instance;
};

#include "audio_win_traits.I"

#endif /* __AUDIO_WIN_TRAITS_H__ */
#endif /* AUDIO_USE_WIN32 */
