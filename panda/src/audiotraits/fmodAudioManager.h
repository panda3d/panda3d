/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioManager.h
 * @author cort
 * @date 2003-01-22
 * @author ben
 * @date 2003-10-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 *
 * Hello, all future Panda audio code people! This is my errata
 * documentation to help any future programmer maintain FMOD and PANDA.
 *
 * This documentation more then that is needed, but I wanted to go all
 * out, with the documentation. Because I was a totally newbie at
 * programming [especially with C/C++] this semester I want to make
 * sure future code maintainers have that insight I did not when
 * starting on the PANDA project here at the ETC/CMU.
 *
 * As of Spring 2006, Panda's FMOD audio support has been pretty much
 * completely rewritten. This has been done so PANDA can use FMOD-EX
 * [or AKA FMOD 4] and some of its new features.
 *
 * First, the FMOD-EX API itself has been completely rewritten compared
 * to previous versions. FMOD now handles any type of audio files, wave
 * audio [WAV, AIF, MP3, OGG, etc...] or musical file [MID, TRACKERS]
 * as the same type of an object. The API has also been structured more
 * like a sound studio, with 'sounds' and 'channels'. This will be
 * covered more in the FmodAudioSound.h/.cxx sources.
 *
 * Second, FMOD now offers virtually unlimited sounds to be played at
 * once via their virtual channels system. Actually the theoretical
 * limit is around 4000, but that is still a lot. What you need to know
 * about this, is that even thought you might only hear 32 sound being
 * played at once, FMOD will keep playing any additional sounds, and
 * swap those on virtual channels in and out with those on real
 * channels depending on priority, or distance [if you are dealing with
 * 3D audio].
 *
 * Third, FMOD's DSP support has been added. So you can now add GLOBAL
 * or SOUND specific DSP effects. Right not you can only use FMOD's
 * built in DSP effects.  But adding support for FMOD's support of VST
 * effects shouldn't be that hard.
 *
 * As for the FmodManager itself, it is pretty straight forward, I
 * hope. As a manager class, it will create the FMOD system with the
 * "_system" variable which is an instance of FMOD::SYSTEM. (Actually,
 * we create only one global _system variable now, and share it with
 * all outstanding FmodManager objects--this appears to be the way FMOD
 * wants to work.)  The FmodManager class is also the one responsible
 * for creation of Sounds, DSP, and maintaining the GLOBAL DSP chains
 * [The GLOBAL DSP chain is the DSP Chain which affects ALL the
 * sounds].
 *
 * Any way that is it for an intro, lets move on to looking at the rest
 * of the code.
 */

#ifndef __FMOD_AUDIO_MANAGER_H__
#define __FMOD_AUDIO_MANAGER_H__

// First the includes.
#include "pandabase.h"
#include "pset.h"

#include "audioManager.h"

// The includes needed for FMOD
#include <fmod.hpp>
#include <fmod_errors.h>

class FmodAudioSound;

extern void fmod_audio_errcheck(const char *context, FMOD_RESULT n);

class EXPCL_FMOD_AUDIO FmodAudioManager : public AudioManager {
  friend class FmodAudioSound;

public:
  FmodAudioManager();
  virtual ~FmodAudioManager();

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename &, bool positional = false, int mode=SM_heuristic);
  virtual PT(AudioSound) get_sound(MovieAudio *,  bool positional = false, int mode=SM_heuristic);

  virtual int get_speaker_setup();
  virtual void set_speaker_setup(SpeakerModeCategory cat);

  virtual void set_volume(PN_stdfloat);
  virtual PN_stdfloat get_volume() const;

  virtual void set_wavwriter(bool);

  virtual void set_active(bool);
  virtual bool get_active() const;

  virtual void stop_all_sounds();

  virtual void update();

  // This controls the "set of ears" that listens to 3D spacialized sound px,
  // py, pz are position coordinates.  Can be 0.0f to ignore.  vx, vy, vz are
  // a velocity vector in UNITS PER SECOND (default: meters). fx, fy and fz
  // are the respective components of a unit forward-vector ux, uy and uz are
  // the respective components of a unit up-vector These changes will NOT be
  // invoked until audio_3d_update() is called.
  virtual void audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
                                                PN_stdfloat vx, PN_stdfloat xy, PN_stdfloat xz,
                                                PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz,
                                                PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz);

  // REMOVE THIS ONE
  virtual void audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz,
                                                PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz,
                                                PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz,
                                                PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz);

  // Control the "relative scale that sets the distance factor" units for 3D
  // spacialized audio. This is a float in units-per-meter. Default value is
  // 1.0, which means that Panda units are understood as meters; for e.g.
  // feet, set 3.28. This factor is applied only to Fmod and OpenAL at the
  // moment.
  virtual void audio_3d_set_distance_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_distance_factor() const;

  // Control the presence of the Doppler effect.  Default is 1.0 Exaggerated
  // Doppler, use >1.0 Diminshed Doppler, use <1.0
  virtual void audio_3d_set_doppler_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_doppler_factor() const;

  // Exaggerate or diminish the effect of distance on sound.  Default is 1.0
  // Faster drop off, use >1.0 Slower drop off, use <1.0
  virtual void audio_3d_set_drop_off_factor(PN_stdfloat factor);
  virtual PN_stdfloat audio_3d_get_drop_off_factor() const;

  // THESE ARE NOT USED ANYMORE. THEY ARE ONLY HERE BECAUSE THEY are still
  // needed by Miles.  THESE are stubs in FMOD-EX version
  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);
  virtual void uncache_sound(const Filename &);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

private:
  FMOD::DSP *make_dsp(const FilterProperties::FilterConfig &conf);
  void update_dsp_chain(FMOD::DSP *head, FilterProperties *config);
  virtual bool configure_filters(FilterProperties *config);

 private:
  // This global lock protects all access to FMod library interfaces.
  static ReMutex _lock;

  static FMOD::System *_system;
  static pset<FmodAudioManager *> _all_managers;

  static bool _system_is_valid;

  static PN_stdfloat _distance_factor;
  static PN_stdfloat _doppler_factor;
  static PN_stdfloat _drop_off_factor;

  FMOD::ChannelGroup *_channelgroup;

  FMOD_VECTOR _position;
  FMOD_VECTOR _velocity;
  FMOD_VECTOR _forward;
  FMOD_VECTOR _up;

  // DLS info for MIDI files
  std::string _dlsname;
  FMOD_CREATESOUNDEXINFO _midi_info;

  bool _is_valid;
  bool _active;

  // The set of all sounds.  Needed only to implement stop_all_sounds.
  typedef pset<FmodAudioSound *> SoundSet;
  SoundSet _all_sounds;

  FMOD_OUTPUTTYPE _saved_outputtype;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "FmodAudioManager", AudioManager::get_class_type());
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

EXPCL_FMOD_AUDIO AudioManager *Create_FmodAudioManager();


#endif /* __FMOD_AUDIO_MANAGER_H__ */
