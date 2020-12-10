/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioSound.h
 * @author cort
 * @date 2003-01-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 *
 * [FIRST READ FMODAudioManager for an Introduction if you haven't
 * already].
 *
 * Hello, all future Panda audio code people! This is my errata
 * documentation to Help any future programmer maintain FMOD and PANDA.
 *
 * Well, if you reading this you probably want to know how PANDA deals
 * with sounds directly using FMOD-EX. Well I am going to tell you.
 *
 * The first thing, you as the programmer have to understand,
 * especially if you never have done sound programming before, is how
 * the FMOD-EX API works.
 *
 * With FMOD-EX the guys at Firelight, adopted a model of managing
 * sounds with FMOD similar to how a Sound Designer creates sound in a
 * sound studio using SOUNDS and CHANNELS. Although this may seem
 * strange at first, if you are not familiar with sound programming,
 * there is a very good metaphor you are probably already familiar with
 * to explain how FMOD-EX works.
 *
 * Think of you standard GUI API. Usually a GUI API is made up of two
 * things: Windows and Widgets. These correspond to CHANNELS and
 * SOUNDS, where a Channel is a Window and a Sound is Widget. Sounds
 * are played within channels, and channels don't exist unless they
 * have something to display.
 *
 * Now why am I explaining all of this? When PANDA was created they set
 * up the basic audio classes to handle only the idea of a SOUND. The
 * idea of a Channel really wasn't prevalent as in more modern Audio
 * APIs. With this rewrite of PANDA to use the FMOD-EX API, the PANDA
 * FMODAudioSound Class, now has to handle two different parts of the
 * FMOD-EX API in order to play a sound.
 *
 * SOUND: The object the handles the audio data in form of WAV, AIF,
 * OGG, MID, IT, MP3, etc... And CHANNEL: The object that actually
 * plays the sound and manipulates it in real time.
 *
 * Ultimately this isn't a problem expect for a couple situations when
 * you go to play a sound, which I will explain in more detail in that
 * part of the code. All that you have to know right now is that
 * Channels in FMOD do not exist unless they are playing a sound. And
 * in the PANDA FMODAudioSound API class there is only ONE dedicated
 * channel per sound.  Otherwise there is really nothing to worry
 * about.
 */

#ifndef FMODAUDIOSOUND_H
#define FMODAUDIOSOUND_H

#include "pandabase.h"

#include "audioSound.h"
#include "reMutex.h"
#include "fmodAudioManager.h"

#include <fmod.hpp>
#include <fmod_errors.h>

class VirtualFile;

class EXPCL_FMOD_AUDIO FMODAudioSound : public AudioSound {
public:
  FMODAudioSound(AudioManager *manager, VirtualFile *file, bool positional);
  virtual ~FMODAudioSound();

  // For best compatibility, set the loop_count, start_time, volume, and
  // balance, prior to calling play().  You may set them while they're
  // playing, but it's implementation specific whether you get the results.
  void play();
  void stop();

  // loop: false = play once; true = play forever.  inits to false.
  void set_loop(bool loop=true);
  bool get_loop() const;

  // loop_count: 0 = forever; 1 = play once; n = play n times.  inits to 1.
  void set_loop_count(unsigned long loop_count=1);
  unsigned long get_loop_count() const;

  // 0 = beginning; length() = end.  inits to 0.0.
  void set_time(PN_stdfloat start_time=0.0);
  PN_stdfloat get_time() const;

  // 0 = minimum; 1.0 = maximum.  inits to 1.0.
  void set_volume(PN_stdfloat volume=1.0);
  PN_stdfloat get_volume() const;

  // -1.0 is hard left 0.0 is centered 1.0 is hard right inits to 0.0.
  void set_balance(PN_stdfloat balance_right=0.0);
  PN_stdfloat get_balance() const;

  // play_rate is any positive float value.  inits to 1.0.
  void set_play_rate(PN_stdfloat play_rate=1.0f);
  PN_stdfloat get_play_rate() const;

  const std::string &get_name() const;

  // return: playing time in seconds.
  PN_stdfloat length() const;

  // Controls the position of this sound's emitter.  pos is a pointer to an
  // xyz triplet of the emitter's position.  vel is a pointer to an xyz
  // triplet of the emitter's velocity.
  void set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz);
  void get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz);

  void set_3d_min_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_min_distance() const;

  void set_3d_max_distance(PN_stdfloat dist);
  PN_stdfloat get_3d_max_distance() const;

  AudioSound::SoundStatus status() const;

  virtual PN_stdfloat get_speaker_mix(int speaker);
  virtual void set_speaker_mix(int speaker, PN_stdfloat mix);
  virtual void set_speaker_mix(PN_stdfloat frontleft, PN_stdfloat frontright,
                               PN_stdfloat center, PN_stdfloat sub,
                               PN_stdfloat backleft, PN_stdfloat backright,
                               PN_stdfloat sideleft, PN_stdfloat sideright);

  void set_active(bool active=true);
  bool get_active() const;

  void finished();
  void set_finished_event(const std::string& event);
  const std::string& get_finished_event() const;

private:
  PT(FMODAudioManager) _manager;
  FMOD::Sound      *_sound;
  FMOD::Channel    *_channel;

  Filename _file_name;

  float _volume;
  float _balance;
  float _playrate;
  int   _priority;
  float _mix[AudioManager::SPK_COUNT];

  float _sample_frequency;
  unsigned int _length;

  FMOD_SPEAKERMODE  _speakermode;

  FMOD_VECTOR _location;
  FMOD_VECTOR _velocity;

  PN_stdfloat _min_dist;
  PN_stdfloat _max_dist;

  void start_playing();
  void set_volume_on_channel();
  void set_balance_on_channel();
  void set_play_rate_on_channel();
  void set_3d_attributes_on_channel();
  // void add_dsp_on_channel();
  void set_speaker_mix_or_balance_on_channel();

  virtual int get_priority();
  virtual void set_priority(int priority);

  bool _active;
  bool _paused;
  PN_stdfloat _start_time;

  bool _is_midi;

  std::string _finished_event;

  static FMOD_RESULT F_CALLBACK
  open_callback(const char *name, unsigned int *file_size,
                void **handle, void *user_data);

  static FMOD_RESULT F_CALLBACK
  close_callback(void *handle, void *user_data);

  static FMOD_RESULT F_CALLBACK
  read_callback(void *handle, void *buffer, unsigned int size_bytes,
                unsigned int *bytes_read, void *user_data);

  static FMOD_RESULT F_CALLBACK
  seek_callback(void *handle, unsigned int pos, void *user_data);

 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioSound::init_type();
    register_type(_type_handle, "FMODAudioSound", AudioSound::get_class_type());
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

#include "fmodAudioSound.I"

#endif // FMODAUDIOSOUND_H
