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

class MikModPlaying;

class EXPCL_PANDA MikModSample : public AudioTraits::SampleClass {
private:
  SAMPLE* _sample;
  int _voice;

  MikModSample(SAMPLE*);
public:
  virtual ~MikModSample(void);

  virtual float length(void);
  virtual AudioTraits::SampleClass::SampleStatus status(void);
public:
  // used by the readers
  static MikModSample* load_wav(Filename);
  virtual MikModPlaying* get_state(void);
  static void destroy(AudioTraits::SampleClass*);
  // used by the players
  virtual void set_voice(int);
  virtual int get_voice(void);
  virtual SAMPLE* get_sample(void);
  virtual int get_freq(void);
};

class EXPCL_PANDA MikModMusic : public AudioTraits::MusicClass {
private:
  MODULE* _music;
public:
  MikModMusic(void);
  virtual ~MikModMusic(void);

  virtual AudioTraits::MusicClass::MusicStatus status(void);
};

class EXPCL_PANDA MikModMidi : public AudioTraits::MusicClass {
private:
public:
  MikModMidi(void);
  virtual ~MikModMidi(void);

  virtual AudioTraits::MusicClass::MusicStatus status(void);
public:
  // used by the readers
  static MikModMidi* load_midi(Filename);
  static void destroy(AudioTraits::MusicClass*);
  virtual MikModPlaying* get_state(void);
};

class EXPCL_PANDA MikModPlaying : public AudioTraits::PlayingClass {
public:
  MikModPlaying(void);
  ~MikModPlaying(void);

  virtual AudioTraits::PlayingClass::PlayingStatus status(void);
};

class EXPCL_PANDA MikModSamplePlayer : public AudioTraits::PlayerClass {
public:
  MikModSamplePlayer(void);
  virtual ~MikModSamplePlayer(void);

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
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

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
};

class EXPCL_PANDA MikModMidiPlayer : public AudioTraits::PlayerClass {
public:
  MikModMidiPlayer(void);
  virtual ~MikModMidiPlayer(void);

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
public:
  // used by the readers
  static MikModMidiPlayer* get_instance(void);
private:
  static MikModMidiPlayer* _global_instance;
};

#endif /* __AUDIO_MIKMOD_TRAITS_H__ */
#endif /* AUDIO_USE_MIKMOD */
