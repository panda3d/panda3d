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
 * @author lachbr
 * @date 2020-10-04
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
 * covered more in the FMODAudioSound.h/.cxx sources.
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

#ifndef FMODAUDIOMANAGER_H
#define FMODAUDIOMANAGER_H

// First the includes.
#include "pandabase.h"
#include "pset.h"
#include "pdeque.h"

#include "audioManager.h"
#include "dsp.h"

// The includes needed for FMOD
#include <fmod.hpp>
#include <fmod_errors.h>

class FMODAudioSound;

extern void _fmod_audio_errcheck(const char *context, FMOD_RESULT n);

#ifdef NDEBUG
#define fmod_audio_errcheck(context, n)
#else
#define fmod_audio_errcheck(context, n) _fmod_audio_errcheck(context, n)
#endif // NDEBUG

class EXPCL_FMOD_AUDIO FMODAudioManager : public AudioManager {
  friend class FMODAudioSound;

public:
  FMODAudioManager();
  virtual ~FMODAudioManager();

  virtual bool configure_filters(FilterProperties *config);

  virtual bool insert_dsp(int index, DSP *dsp);
  virtual bool remove_dsp(DSP *dsp);
  virtual void remove_all_dsps();
  virtual int get_num_dsps() const;

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

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);

  // These are currently unused by the FMOD implementation.  Could possibly
  // implement them, though.
  virtual void uncache_sound(const Filename &);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  FMOD_RESULT get_speaker_mode(FMOD_SPEAKERMODE &mode) const;

private:
  FMOD_DSP_TYPE get_fmod_dsp_type(DSP::DSPType panda_type);
  FMOD::DSP *create_fmod_dsp(DSP *panda_dsp);
  FMOD::DSP *get_fmod_dsp(DSP *panda_dsp) const;
  void configure_dsp(DSP *panda_dsp, FMOD::DSP *dsp);

  static void add_manager_to_dsp(DSP *dsp, FMODAudioManager *mgr);
  static void remove_manager_from_dsp(DSP *dsp, FMODAudioManager *mgr);
  static void update_dirty_dsps();

  void starting_sound(FMODAudioSound *sound);
  void stopping_sound(FMODAudioSound *sound);
  // Tell the manager that the sound dtor was called.
  void release_sound(FMODAudioSound* sound);

  void update_sounds();

private:
  // This global lock protects all access to FMod library interfaces.
  static ReMutex _lock;

  static FMOD::System *_system;

  typedef pset<FMODAudioManager *> ManagerList;
  static ManagerList _all_managers;

  static bool _system_is_valid;

  static PN_stdfloat _distance_factor;
  static PN_stdfloat _doppler_factor;
  static PN_stdfloat _drop_off_factor;

  // We need this to support applying the same DSP onto multiple audio
  // managers.  We run a once-per-frame update method that iterates over all
  // the DSPs, and for each one, checks if the dirty flag is set.  If it is,
  // we configure the DSP on all audio managers that it has been applied to.
  typedef pmap<DSP *, ManagerList> DSPManagers;
  static DSPManagers _dsp_managers;

  static int _last_update_frame;

private:
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

  typedef phash_set<PT(FMODAudioSound)> SoundsPlaying;
  SoundsPlaying _sounds_playing;

  typedef phash_set<FMODAudioSound *> AllSounds;
  AllSounds _all_sounds;

  // Mapping of Panda DSP instance to FMOD DSP instance.
  typedef pmap<PT(DSP), FMOD::DSP *> FMODDSPs;
  FMODDSPs _dsps;

  FMOD_OUTPUTTYPE _saved_outputtype;

  unsigned int _concurrent_sound_limit;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "FMODAudioManager", AudioManager::get_class_type());
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


#endif /* FMODAUDIOMANAGER_H */
