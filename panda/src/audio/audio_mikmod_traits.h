// Filename: audio_mikmod_traits.h
// Created by:  frang (06Jul00)
// 
////////////////////////////////////////////////////////////////////

// yes, this should be outside
#include "audio_trait.h"

#ifdef AUDIO_USE_MIKMOD
#ifndef __AUDIO_MIKMOD_TRAITS_H__
#define __AUDIO_MIKMOD_TRAITS_H__

#include <pandabase.h>
#include <filename.h>
#include <mikmod.h>

class MikModSamplePlaying;

class EXPCL_PANDA MikModSample : public AudioTraits::SoundClass {
private:
  SAMPLE* _sample;

  MikModSample(SAMPLE*);
public:
  virtual ~MikModSample(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;
public:
  // used by the readers
  static MikModSample* load_wav(Filename);
  // used by the players
  virtual SAMPLE* get_sample(void);
  virtual int get_freq(void);
};

class EXPCL_PANDA MikModMusic : public AudioTraits::SoundClass {
private:
  MODULE* _music;
public:
  MikModMusic(void);
  virtual ~MikModMusic(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;
};

class EXPCL_PANDA MikModMidi : public AudioTraits::SoundClass {
private:
public:
  MikModMidi(void);
  virtual ~MikModMidi(void);

  virtual float length(void) const;
  virtual AudioTraits::PlayingClass* get_state(void) const;
  virtual AudioTraits::PlayerClass* get_player(void) const;
  virtual AudioTraits::DeletePlayingFunc* get_delstate(void) const;
public:
  // used by the readers
  static MikModMidi* load_midi(Filename);
};

class EXPCL_PANDA MikModSamplePlaying : public AudioTraits::PlayingClass {
private:
  int _voice;
public:
  MikModSamplePlaying(AudioTraits::SoundClass*);
  ~MikModSamplePlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
public:
  virtual void set_voice(int);
  virtual int get_voice(void) const;
};

class EXPCL_PANDA MikModMusicPlaying : public AudioTraits::PlayingClass {
public:
  MikModMusicPlaying(AudioTraits::SoundClass*);
  ~MikModMusicPlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA MikModMidiPlaying : public AudioTraits::PlayingClass {
public:
  MikModMidiPlaying(AudioTraits::SoundClass*);
  ~MikModMidiPlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
  static void destroy(AudioTraits::PlayingClass*);
};

class EXPCL_PANDA MikModSamplePlayer : public AudioTraits::PlayerClass {
public:
  MikModSamplePlayer(void);
  virtual ~MikModSamplePlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static MikModSamplePlayer* get_instance(void);
private:
  static MikModSamplePlayer* _global_instance;
};

class EXPCL_PANDA MikModFmsynthPlayer : public AudioTraits::PlayerClass {
public:
  MikModFmsynthPlayer(void);
  virtual ~MikModFmsynthPlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static MikModFmsynthPlayer* get_instance(void);
private:
  static MikModFmsynthPlayer* _global_instance;
};

class EXPCL_PANDA MikModMidiPlayer : public AudioTraits::PlayerClass {
public:
  MikModMidiPlayer(void);
  virtual ~MikModMidiPlayer(void);

  virtual void play_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*, float);
  virtual void stop_sound(AudioTraits::SoundClass*,
			  AudioTraits::PlayingClass*);
  virtual void set_volume(AudioTraits::PlayingClass*, float);
  virtual bool adjust_volume(AudioTraits::PlayingClass*);
public:
  // used by the readers
  static MikModMidiPlayer* get_instance(void);
private:
  static MikModMidiPlayer* _global_instance;
};

#endif /* __AUDIO_MIKMOD_TRAITS_H__ */
#endif /* AUDIO_USE_MIKMOD */
