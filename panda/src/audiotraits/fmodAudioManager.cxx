/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioManager.cxx
 * @author cort
 * @date 2003-01-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 */

#include "pandabase.h"
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "dcast.h"

// Panda headers.
#include "clockObject.h"
#include "config_audio.h"
#include "config_putil.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "filename.h"
#include "virtualFileSystem.h"
#include "reMutexHolder.h"

// Panda DSP types.
#include "chorusDSP.h"
#include "compressorDSP.h"
#include "distortionDSP.h"
#include "echoDSP.h"
#include "faderDSP.h"
#include "flangeDSP.h"
#include "highpassDSP.h"
#include "limiterDSP.h"
#include "lowpassDSP.h"
#include "normalizeDSP.h"
#include "oscillatorDSP.h"
#include "sfxReverbDSP.h"

// FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>

TypeHandle FMODAudioManager::_type_handle;

ReMutex FMODAudioManager::_lock;
FMOD::System *FMODAudioManager::_system;

FMODAudioManager::ManagerList FMODAudioManager::_all_managers;

bool FMODAudioManager::_system_is_valid = false;

PN_stdfloat FMODAudioManager::_doppler_factor = 1;
PN_stdfloat FMODAudioManager::_distance_factor = 1;
PN_stdfloat FMODAudioManager::_drop_off_factor = 1;

FMODAudioManager::DSPManagers FMODAudioManager::_dsp_managers;

int FMODAudioManager::_last_update_frame = -1;

#define FMOD_MIN_SAMPLE_RATE 80000
#define FMOD_MAX_SAMPLE_RATE 192000

// Central dispatcher for audio errors.

void _fmod_audio_errcheck(const char *context, FMOD_RESULT result) {
  if (result != FMOD_OK) {
    audio_error(context << ": " << FMOD_ErrorString(result) );
  }
}

/**
 * Factory Function
 */
AudioManager *Create_FmodAudioManager() {
  audio_debug("Create_FmodAudioManager()");
  return new FMODAudioManager;
}


/**
 *
 */
FMODAudioManager::
FMODAudioManager() {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;

  // We need a temporary variable to check the FMOD version.
  unsigned int      version;

  _all_managers.insert(this);

  _concurrent_sound_limit = 0;

  ////////////////////////////////////////////////////////////
  // Initialize the 3D listener (camera) attributes.
  //
  _position.x = 0;
  _position.y = 0;
  _position.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  _forward.x = 0;
  _forward.y = 0;
  _forward.z = 0;

  _up.x = 0;
  _up.y = 0;
  _up.z = 0;

  ////////////////////////////////////////////////////////////

  _active = true;

  _saved_outputtype = FMOD_OUTPUTTYPE_AUTODETECT;

  if (!_system) {
    // Create the global FMOD System object.  This one object must be shared
    // by all FmodAudioManagers (this is particularly true on OSX, but the
    // FMOD documentation is unclear as to whether this is the intended design
    // on all systems).

    result = FMOD::System_Create(&_system);
    fmod_audio_errcheck("FMOD::System_Create()", result);

    // Lets check the version of FMOD to make sure the headers and libraries
    // are correct.
    result = _system->getVersion(&version);
    fmod_audio_errcheck("_system->getVersion()", result);

    if (version < FMOD_VERSION){
      audio_error("You are using an old version of FMOD.  This program requires: " << FMOD_VERSION);
    }

    // Determine the sample rate and speaker mode for the system.  We will use
    // the default configuration that FMOD chooses unless the user specifies
    // custom values via config variables.

    int sample_rate;
    FMOD_SPEAKERMODE speaker_mode;
    int num_raw_speakers;
    _system->getSoftwareFormat(&sample_rate,
                               &speaker_mode,
                               &num_raw_speakers);

    audio_debug("fmod-mixer-sample-rate: " << fmod_mixer_sample_rate);
    if (fmod_mixer_sample_rate.get_value() != -1) {
      if (fmod_mixer_sample_rate.get_value() >= FMOD_MIN_SAMPLE_RATE &&
          fmod_mixer_sample_rate.get_value() <= FMOD_MAX_SAMPLE_RATE) {
            sample_rate = fmod_mixer_sample_rate;
            audio_debug("Using user specified sample rate");
      } else {
        fmodAudio_cat.warning()
          << "fmod-mixer-sample-rate had an out-of-range value: "
          << fmod_mixer_sample_rate
          << ". Valid range is [" << FMOD_MIN_SAMPLE_RATE << ", "
          << FMOD_MAX_SAMPLE_RATE << "]\n";
      }
    }

    if (fmod_speaker_mode == FSM_unspecified) {
      if (fmod_use_surround_sound) {
        // fmod-use-surround-sound is the old variable, now replaced by fmod-
        // speaker-mode.  This is for backward compatibility.
        speaker_mode = FMOD_SPEAKERMODE_5POINT1;
      }
    } else {
      speaker_mode = (FMOD_SPEAKERMODE)fmod_speaker_mode.get_value();
    }

    // Set the mixer and speaker format.
    result = _system->setSoftwareFormat(sample_rate, speaker_mode,
                                        num_raw_speakers);
    fmod_audio_errcheck("_system->setSoftwareFormat()", result);

    // Now initialize the system.
    int nchan = fmod_number_of_sound_channels;
    int flags = FMOD_INIT_NORMAL;

    result = _system->init(nchan, flags, 0);
    if (result == FMOD_ERR_TOOMANYCHANNELS) {
      fmodAudio_cat.error()
        << "Value too large for fmod-number-of-sound-channels: " << nchan
        << "\n";
    } else {
      fmod_audio_errcheck("_system->init()", result);
    }

    _system_is_valid = (result == FMOD_OK);

    if (_system_is_valid) {
      result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
      fmod_audio_errcheck("_system->set3DSettings()", result);
    }
  }

  _is_valid = _system_is_valid;

  memset(&_midi_info, 0, sizeof(_midi_info));
  _midi_info.cbsize = sizeof(_midi_info);

  Filename dls_pathname = get_dls_pathname();

#ifdef IS_OSX
  // Here's a big kludge.  Don't ever let FMOD try to load this OSX-provided
  // file; it crashes messily if you do.
  // FIXME: Is this still true on FMOD Core?
  if (dls_pathname == "/System/Library/Components/CoreAudio.component/Contents/Resources/gs_instruments.dls") {
    dls_pathname = "";
  }
#endif  // IS_OSX

  if (!dls_pathname.empty()) {
    _dlsname = dls_pathname.to_os_specific();
    _midi_info.dlsname = _dlsname.c_str();
  }

  if (_is_valid) {
    result = _system->createChannelGroup("UserGroup", &_channelgroup);
    fmod_audio_errcheck("_system->createChannelGroup()", result);
  }
}

/**
 *
 */
FMODAudioManager::
~FMODAudioManager() {
  ReMutexHolder holder(_lock);

  // Be sure to delete associated sounds before deleting the manager!
  FMOD_RESULT result;

  // Release all of our sounds
  _sounds_playing.clear();
  _all_sounds.clear();

  // Release all DSPs
  remove_all_dsps();

  // Remove me from the managers list.
  _all_managers.erase(this);

  if (_channelgroup) {
    _channelgroup->release();
    _channelgroup = nullptr;
  }

  if (_all_managers.empty()) {
    result = _system->release();
    fmod_audio_errcheck("_system->release()", result);
    _system = nullptr;
    _system_is_valid = false;
  }
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool FMODAudioManager::
insert_dsp(int index, DSP *panda_dsp) {
  ReMutexHolder holder(_lock);

  // If it's already in there, take it out and put it in the new spot.
  remove_dsp(panda_dsp);

  FMOD::DSP *dsp = create_fmod_dsp(panda_dsp);
  if (!dsp) {
    fmodAudio_cat.warning()
      << panda_dsp->get_type().get_name()
      << " unsupported by FMOD audio implementation.\n";
    return false;
  }

  FMOD_RESULT ret;
  ret = _channelgroup->addDSP(index, dsp);
  fmod_audio_errcheck("_channelgroup->addDSP()", ret);

  // Keep track of our DSPs.
  add_manager_to_dsp(panda_dsp, this);
  _dsps[panda_dsp] = dsp;

  return true;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool FMODAudioManager::
remove_dsp(DSP *panda_dsp) {
  ReMutexHolder holder(_lock);

  auto itr = _dsps.find(panda_dsp);
  if (itr == _dsps.end()) {
    return false;
  }

  FMOD::DSP *dsp = itr->second;

  FMOD_RESULT ret;
  ret = _channelgroup->removeDSP(dsp);
  fmod_audio_errcheck("_channelGroup->removeDSP()", ret);

  ret = dsp->release();
  fmod_audio_errcheck("dsp->release()", ret);

  remove_manager_from_dsp(panda_dsp, this);
  _dsps.erase(itr);

  return true;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void FMODAudioManager::
remove_all_dsps() {
  ReMutexHolder holder(_lock);

  for (auto itr = _dsps.begin(); itr != _dsps.end(); itr++) {
    DSP *panda_dsp = itr->first;
    FMOD::DSP *fmod_dsp = itr->second;
    FMOD_RESULT ret;

    ret = _channelgroup->removeDSP(fmod_dsp);
    fmod_audio_errcheck("_channelgroup->removeDSP()", ret);

    ret = fmod_dsp->release();
    fmod_audio_errcheck("fmod_dsp->release()", ret);

    remove_manager_from_dsp(panda_dsp, this);
  }

  _dsps.clear();
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int FMODAudioManager::
get_num_dsps() const {
  // Can't use _channelgroup->getNumDSPs() because that includes DSPs that are
  // created internally by FMOD.  We want to return the number of user-created
  // DSPs.

  return (int)_dsps.size();
}

/**
 * This just check to make sure the FMOD System is up and running correctly.
 */
bool FMODAudioManager::
is_valid() {
  return _is_valid;
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) FMODAudioManager::
get_sound(const Filename &file_name, bool positional, int) {
  ReMutexHolder holder(_lock);
  // Needed so People use Panda's Generic UNIX Style Paths for Filename.
  // path.to_os_specific() converts it back to the proper OS version later on.

  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  // Locate the file on disk.
  path.set_binary();
  PT(VirtualFile) file = vfs->get_file(path);
  if (file != nullptr) {
    // Build a new AudioSound from the audio data.
    PT(FMODAudioSound) sound = new FMODAudioSound(this, file, positional);

    _all_sounds.insert(sound);
    return sound;
  } else {
    audio_error("createSound(" << path << "): File not found.");
    return get_null_sound();
  }
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) FMODAudioManager::
get_sound(MovieAudio *source, bool positional, int) {
  nassert_raise("FMOD audio manager does not support MovieAudio sources");
  return nullptr;
}

/**
 * This is to query if you are using a multichannel setup.
 */
int FMODAudioManager::
get_speaker_setup() {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;

  int ret;

  int sample_rate;
  FMOD_SPEAKERMODE speaker_mode;
  int num_raw_speakers;
  result = _system->getSoftwareFormat(&sample_rate,
                                      &speaker_mode,
                                      &num_raw_speakers);
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  switch (speaker_mode) {
  case FMOD_SPEAKERMODE_RAW:
    return AudioManager::SPEAKERMODE_raw;
  case FMOD_SPEAKERMODE_MONO:
    return AudioManager::SPEAKERMODE_mono;
  case FMOD_SPEAKERMODE_STEREO:
    return AudioManager::SPEAKERMODE_stereo;
  case FMOD_SPEAKERMODE_QUAD:
    return AudioManager::SPEAKERMODE_quad;
  case FMOD_SPEAKERMODE_SURROUND:
    return AudioManager::SPEAKERMODE_surround;
  case FMOD_SPEAKERMODE_5POINT1:
    return AudioManager::SPEAKERMODE_5point1;
  case FMOD_SPEAKERMODE_7POINT1:
    return AudioManager::SPEAKERMODE_7point1;
  case FMOD_SPEAKERMODE_7POINT1POINT4:
    return AudioManager::SPEAKERMODE_7point1point4;
  case FMOD_SPEAKERMODE_MAX:
    return AudioManager::SPEAKERMODE_max;
  default:
    return AudioManager::SPEAKERMODE_COUNT;
  }
}

/**
 * This is to set up FMOD to use a MultiChannel Setup.  This method is pretty
 * much useless.  To set a speaker setup in FMOD for Surround Sound, stereo,
 * or whatever you have to set the SpeakerMode BEFORE you Initialize FMOD.
 * Since Panda Inits the FMODAudioManager right when you Start it up, you are
 * never given an oppertunity to call this function.  That is why I stuck a
 * BOOL in the CONFIG.PRC file, whichs lets you flag if you want to use a
 * Multichannel or not.  That will set the speaker setup when an instance of
 * this class is constructed.  Still I put this here as a measure of good
 * faith, since you can query the speaker setup after everything in Init.
 * Also, maybe someone will completely hack Panda someday, in which one can
 * init or re-init the AudioManagers after Panda is running.
 */
void FMODAudioManager::
set_speaker_setup(AudioManager::SpeakerModeCategory cat) {
  ///ReMutexHolder holder(_lock);
  //FMOD_RESULT result;
  //FMOD_SPEAKERMODE speakerModeType = (FMOD_SPEAKERMODE)cat;
  //result = _system->setSpeakerMode( speakerModeType);
  //fmod_audio_errcheck("_system->setSpeakerMode()", result);
  fmodAudio_cat.warning()
    << "FMODAudioManager::set_speaker_setup() doesn't do anything\n";
}

/**
 * Sets the volume of the AudioManager.  It is not an override, but a
 * multiplier.
 */
void FMODAudioManager::
set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  result = _channelgroup->setVolume(volume);
  fmod_audio_errcheck("_channelgroup->setVolume()", result);
}

/**
 * Returns the AudioManager's volume.
 */
PN_stdfloat FMODAudioManager::
get_volume() const {
  ReMutexHolder holder(_lock);
  float volume;
  FMOD_RESULT result;
  result = _channelgroup->getVolume(&volume);
  fmod_audio_errcheck("_channelgroup->getVolume()", result);
  return (PN_stdfloat)volume;
}

/**
 * Changes output mode to write all audio to a wav file.
 */
void FMODAudioManager::
set_wavwriter(bool outputwav) {
  ReMutexHolder holder(_lock);
  if (outputwav) {
    _system->getOutput(&_saved_outputtype);
    _system->setOutput(FMOD_OUTPUTTYPE_WAVWRITER);
  }
  else {
    _system->setOutput(_saved_outputtype);
  }
}

/**
 * Turn on/off.
 */
void FMODAudioManager::
set_active(bool active) {
  ReMutexHolder holder(_lock);
  if (_active != active) {
    _active = active;

    // Tell our AudioSounds to adjust:
    for (AllSounds::iterator i = _all_sounds.begin();
         i != _all_sounds.end();
         ++i) {
      (*i)->set_active(_active);
    }
  }
}

/**
 *
 */
bool FMODAudioManager::
get_active() const {
  return _active;
}

/**
 * Stop playback on all sounds managed by this manager.
 */
void FMODAudioManager::
stop_all_sounds() {
  ReMutexHolder holder(_lock);
  // We have to walk through this list with some care, since stopping a sound
  // may also remove it from the set (if there are no other references to the
  // sound).
  AllSounds::iterator i;
  i = _all_sounds.begin();
  while (i != _all_sounds.end()) {
    AllSounds::iterator next = i;
    ++next;

    (*i)->stop();
    i = next;
  }
}

/**
 * Perform all per-frame update functions.
 */
void FMODAudioManager::
update() {
  ReMutexHolder holder(_lock);

  // Call finished() and release our reference to sounds that have finished
  // playing.
  update_sounds();

  // Update the FMOD system, but make sure we only do it once per frame.
  ClockObject *clock = ClockObject::get_global_clock();
  int current_frame = clock->get_frame_count();
  if (current_frame != _last_update_frame) {
    update_dirty_dsps();
    _system->update();
    _last_update_frame = current_frame;
  }
}

/**
 * Set position of the "ear" that picks up 3d sounds NOW LISTEN UP!!! THIS IS
 * IMPORTANT! Both Panda3D and FMOD use a left handed coordinate system.  But
 * there is a major difference!  In Panda3D the Y-Axis is going into the
 * Screen and the Z-Axis is going up.  In FMOD the Y-Axis is going up and the
 * Z-Axis is going into the screen.  The solution is simple, we just flip the
 * Y and Z axis, as we move coordinates from Panda to FMOD and back.  What
 * does did mean to average Panda user?  Nothing, they shouldn't notice
 * anyway.  But if you decide to do any 3D audio work in here you have to keep
 * it in mind.  I told you, so you can't say I didn't.
 */
void FMODAudioManager::
audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::audio_3d_set_listener_attributes()");

  FMOD_RESULT result;

  _position.x = px;
  _position.y = pz;
  _position.z = py;

  _velocity.x = vx;
  _velocity.y = vz;
  _velocity.z = vy;

  _forward.x = fx;
  _forward.y = fz;
  _forward.z = fy;

  _up.x = ux;
  _up.y = uz;
  _up.z = uy;

  result = _system->set3DListenerAttributes( 0, &_position, &_velocity, &_forward, &_up);
  fmod_audio_errcheck("_system->set3DListenerAttributes()", result);

}

/**
 * Get position of the "ear" that picks up 3d sounds
 */
void FMODAudioManager::
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  audio_error("audio3dGetListenerAttributes: currently unimplemented. Get the attributes of the attached object");

}

/**
 * Set units per meter (Fmod uses meters internally for its sound-
 * spacialization calculations)
 */
void FMODAudioManager::
audio_3d_set_distance_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug( "FMODAudioManager::audio_3d_set_distance_factor( factor= " << factor << ")" );

  FMOD_RESULT result;

  _distance_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);
}

/**
 * Gets units per meter (Fmod uses meters internally for its sound-
 * spacialization calculations)
 */
PN_stdfloat FMODAudioManager::
audio_3d_get_distance_factor() const {
  audio_debug("FMODAudioManager::audio_3d_get_distance_factor()");

  return _distance_factor;
}

/**
 * Exaggerates or diminishes the Doppler effect.  Defaults to 1.0
 */
void FMODAudioManager::
audio_3d_set_doppler_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::audio_3d_set_doppler_factor(factor="<<factor<<")");

  FMOD_RESULT result;

  _doppler_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);
}

/**
 *
 */
PN_stdfloat FMODAudioManager::
audio_3d_get_doppler_factor() const {
  audio_debug("FMODAudioManager::audio_3d_get_doppler_factor()");

  return _doppler_factor;
}

/**
 * Control the effect distance has on audability.  Defaults to 1.0
 */
void FMODAudioManager::
audio_3d_set_drop_off_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::audio_3d_set_drop_off_factor("<<factor<<")");

  FMOD_RESULT result;

  _drop_off_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);

}

/**
 *
 */
PN_stdfloat FMODAudioManager::
audio_3d_get_drop_off_factor() const {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::audio_3d_get_drop_off_factor()");

  return _drop_off_factor;
}

/**
 *
 */
void FMODAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  ReMutexHolder holder(_lock);
  _concurrent_sound_limit = limit;
  reduce_sounds_playing_to(_concurrent_sound_limit);
}

/**
 *
 */
unsigned int FMODAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

/**
 *
 */
void FMODAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  ReMutexHolder holder(_lock);

  // first give all sounds that have finished a chance to stop, so that these
  // get stopped first
  update_sounds();

  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    nassertv(sound != _sounds_playing.end());
    // When the user stops a sound, there is still a PT in the user's hand.
    // When we stop a sound here, however, this can remove the last PT.  This
    // can cause an ugly recursion where stop calls the destructor, and the
    // destructor calls stop.  To avoid this, we create a temporary PT, stop
    // the sound, and then release the PT.
    PT(FMODAudioSound) s = (*sound);
    s->stop();
  }
}

/**
 * NOT USED FOR FMOD!!! Clears a sound out of the sound cache.
 */
void FMODAudioManager::
uncache_sound(const Filename &file_name) {
  audio_debug("FMODAudioManager::uncache_sound(\""<<file_name<<"\")");
}

/**
 * NOT USED FOR FMOD!!! Clear out the sound cache.
 */
void FMODAudioManager::
clear_cache() {
  audio_debug("FMODAudioManager::clear_cache()");
}

/**
 * NOT USED FOR FMOD!!! Set the number of sounds that the cache can hold.
 */
void FMODAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("FMODAudioManager::set_cache_limit(count="<<count<<")");
}

/**
 * NOT USED FOR FMOD!!! Gets the number of sounds that the cache can hold.
 */
unsigned int FMODAudioManager::
get_cache_limit() const {
  audio_debug("FMODAudioManager::get_cache_limit() returning ");
  return 0;
}

FMOD_RESULT FMODAudioManager::
get_speaker_mode(FMOD_SPEAKERMODE &mode) const {
  int num_samples;
  int num_raw_speakers;

  return _system->getSoftwareFormat(&num_samples, &mode,
                                    &num_raw_speakers);
}

/**
 * Returns the FMOD DSP type from the Panda DSP type.
 */
FMOD_DSP_TYPE FMODAudioManager::
get_fmod_dsp_type(DSP::DSPType panda_type) {
  switch (panda_type) {
  case DSP::DT_chorus:
    return FMOD_DSP_TYPE_CHORUS;
  case DSP::DT_compressor:
    return FMOD_DSP_TYPE_COMPRESSOR;
  case DSP::DT_distortion:
    return FMOD_DSP_TYPE_DISTORTION;
  case DSP::DT_echo:
    return FMOD_DSP_TYPE_ECHO;
  case DSP::DT_fader:
    return FMOD_DSP_TYPE_FADER;
  case DSP::DT_flange:
    return FMOD_DSP_TYPE_FLANGE;
  case DSP::DT_highpass:
    return FMOD_DSP_TYPE_HIGHPASS;
  case DSP::DT_lowpass:
    return FMOD_DSP_TYPE_LOWPASS;
  case DSP::DT_limiter:
    return FMOD_DSP_TYPE_LIMITER;
  case DSP::DT_oscillator:
    return FMOD_DSP_TYPE_OSCILLATOR;
  case DSP::DT_sfxreverb:
    return FMOD_DSP_TYPE_SFXREVERB;
  case DSP::DT_normalize:
    return FMOD_DSP_TYPE_NORMALIZE;
  default:
    return FMOD_DSP_TYPE_UNKNOWN;
  }
}

/**
 * Creates and returns a new FMOD DSP instance for the provided Panda DSP,
 * or returns nullptr if the DSP type is unsupported.
 */
FMOD::DSP *FMODAudioManager::
create_fmod_dsp(DSP *panda_dsp) {
  ReMutexHolder holder(_lock);

  FMOD_DSP_TYPE type = get_fmod_dsp_type(panda_dsp->get_dsp_type());
  if (type == FMOD_DSP_TYPE_UNKNOWN) {
    // We don't support this DSP type.
    return nullptr;
  }

  audio_debug("Creating new DSP instance of type "
              << panda_dsp->get_type().get_name());

  FMOD_RESULT result;
  FMOD::DSP *dsp;
  result = _system->createDSPByType(type, &dsp);
  fmod_audio_errcheck("_sytem->createDSPByType()", result);

  // Apply the current parameters while we're at it.
  configure_dsp(panda_dsp, dsp);
  panda_dsp->clear_dirty();

  return dsp;
}

/**
 * Returns the FMOD DSP associated with the Panda DSP, or nullptr if it doesn't
 * exist.
 */
FMOD::DSP *FMODAudioManager::
get_fmod_dsp(DSP *panda_dsp) const {
  auto itr = _dsps.find(panda_dsp);
  if (itr == _dsps.end()) {
    return nullptr;
  }

  return itr->second;
}

/**
 * Configures the FMOD DSP based on the parameters in the Panda DSP.
 */
void FMODAudioManager::
configure_dsp(DSP *dsp_conf, FMOD::DSP *dsp) {
  ReMutexHolder holder(_lock);

  switch(dsp_conf->get_dsp_type()) {
  default:
    fmodAudio_cat.warning()
      << "Don't know how to configure "
      << dsp_conf->get_type().get_name() << "\n";
    break;
  case DSP::DT_chorus:
    {
      ChorusDSP *chorus_conf = DCAST(ChorusDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_CHORUS_MIX, chorus_conf->get_mix());
      dsp->setParameterFloat(FMOD_DSP_CHORUS_RATE, chorus_conf->get_rate());
      dsp->setParameterFloat(FMOD_DSP_CHORUS_DEPTH, chorus_conf->get_depth());
    }
    break;
  case DSP::DT_compressor:
    {
      CompressorDSP *comp_conf = DCAST(CompressorDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, comp_conf->get_threshold());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_RATIO, comp_conf->get_ratio());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK, comp_conf->get_attack());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_RELEASE, comp_conf->get_release());
      dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_GAINMAKEUP, comp_conf->get_gainmakeup());
    }
    break;
  case DSP::DT_distortion:
    {
      DistortionDSP *dist_conf = DCAST(DistortionDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL, dist_conf->get_level());
    }
    break;
  case DSP::DT_echo:
    {
      EchoDSP *echo_conf = DCAST(EchoDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_ECHO_DELAY, echo_conf->get_delay());
      dsp->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK, echo_conf->get_feedback());
      dsp->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL, echo_conf->get_drylevel());
      dsp->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL, echo_conf->get_wetlevel());
    }
    break;
  case DSP::DT_fader:
    {
      FaderDSP *fader_conf = DCAST(FaderDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_FADER_GAIN, fader_conf->get_gain());
    }
    break;
  case DSP::DT_flange:
    {
      FlangeDSP *flange_conf = DCAST(FlangeDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_FLANGE_MIX, flange_conf->get_mix());
      dsp->setParameterFloat(FMOD_DSP_FLANGE_DEPTH, flange_conf->get_depth());
      dsp->setParameterFloat(FMOD_DSP_FLANGE_RATE, flange_conf->get_rate());
    }
    break;
  case DSP::DT_highpass:
    {
      HighpassDSP *hp_conf = DCAST(HighpassDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, hp_conf->get_cutoff());
      dsp->setParameterFloat(FMOD_DSP_HIGHPASS_RESONANCE, hp_conf->get_resonance());
    }
    break;
  case DSP::DT_limiter:
    {
      LimiterDSP *lim_conf = DCAST(LimiterDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_LIMITER_RELEASETIME, lim_conf->get_release_time());
      dsp->setParameterFloat(FMOD_DSP_LIMITER_CEILING, lim_conf->get_ceiling());
      dsp->setParameterFloat(FMOD_DSP_LIMITER_MAXIMIZERGAIN, lim_conf->get_maximizer_gain());
    }
    break;
  case DSP::DT_lowpass:
    {
      LowpassDSP *lp_conf = DCAST(LowpassDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lp_conf->get_cutoff());
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, lp_conf->get_resonance());
    }
    break;
  case DSP::DT_normalize:
    {
      NormalizeDSP *norm_conf = DCAST(NormalizeDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_FADETIME, norm_conf->get_fade_time());
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_THRESHHOLD, norm_conf->get_threshold());
      dsp->setParameterFloat(FMOD_DSP_NORMALIZE_MAXAMP, norm_conf->get_max_amp());
    }
    break;
  case DSP::DT_oscillator:
    {
      OscillatorDSP *osc_conf = DCAST(OscillatorDSP, dsp_conf);
      dsp->setParameterInt(FMOD_DSP_OSCILLATOR_TYPE, osc_conf->get_oscillator_type());
      dsp->setParameterFloat(FMOD_DSP_OSCILLATOR_RATE, osc_conf->get_rate());
    }
    break;
  case DSP::DT_sfxreverb:
    {
      SFXReverbDSP *sfx_conf = DCAST(SFXReverbDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, sfx_conf->get_decay_time());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, sfx_conf->get_early_delay());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, sfx_conf->get_late_delay());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, sfx_conf->get_hf_reference());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, sfx_conf->get_hf_decay_ratio());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, sfx_conf->get_diffusion());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, sfx_conf->get_density());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFFREQUENCY, sfx_conf->get_low_shelf_frequency());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFGAIN, sfx_conf->get_low_shelf_gain());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, sfx_conf->get_highcut());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, sfx_conf->get_early_late_mix());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, sfx_conf->get_wetlevel());
      dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, sfx_conf->get_drylevel());
    }
    break;
  }
}

/**
 * Adds the specified audio manager as having the specified DSP applied to it.
 */
void FMODAudioManager::
add_manager_to_dsp(DSP *dsp, FMODAudioManager *mgr) {
  auto itr = _dsp_managers.find(dsp);
  if (itr == _dsp_managers.end()) {
    audio_debug("Adding first manager to DSP");
    _dsp_managers[dsp] = { mgr };
    return;
  }

  audio_debug("Adding new manager to DSP");

  itr->second.insert(mgr);
}

/**
 * Removes the specified audio manager from the list of audio managers with
 * this DSP applied to it.  Destructs the DSP if there are no more managers
 * with this DSP.
 */
void FMODAudioManager::
remove_manager_from_dsp(DSP *dsp, FMODAudioManager *mgr) {
  auto itr = _dsp_managers.find(dsp);
  if (itr == _dsp_managers.end()) {
    return;
  }

  itr->second.erase(mgr);

  audio_debug("Removed manager from DSP");

  if (itr->second.size() == 0) {
    audio_debug("DSP has no more managers");
    _dsp_managers.erase(itr);
  }
}

/**
 * Updates all DSPs with the dirty flag set.
 */
void FMODAudioManager::
update_dirty_dsps() {
  for (auto itr = _dsp_managers.begin(); itr != _dsp_managers.end(); itr++) {
    DSP *panda_dsp = itr->first;
    const ManagerList &managers = itr->second;

    if (panda_dsp->is_dirty()) {
      audio_debug("Updating dirty " << panda_dsp->get_type().get_name());

      for (auto mitr = managers.begin(); mitr != managers.end(); mitr++) {
        FMODAudioManager *manager = *mitr;
        FMOD::DSP *fmod_dsp = manager->get_fmod_dsp(panda_dsp);
        if (!fmod_dsp) {
          continue;
        }

        manager->configure_dsp(panda_dsp, fmod_dsp);
      }

      panda_dsp->clear_dirty();
    }
  }
}

/**
 * Inform the manager that a sound is about to play.  The manager will add
 * this sound to the table of sounds that are playing.
 */
void FMODAudioManager::
starting_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);

  // If the sound is already in there, don't do anything.
  if (_sounds_playing.find(sound) != _sounds_playing.end()) {
    return;
  }

  // first give all sounds that have finished a chance to stop, so that these
  // get stopped first
  update_sounds();

  if (_concurrent_sound_limit) {
    reduce_sounds_playing_to(_concurrent_sound_limit-1); // because we're about to add one
  }

  _sounds_playing.insert(sound);
}

/**
 * Inform the manager that a sound is finished or someone called stop on the
 * sound (this should not be called if a sound is only paused).
 */
void FMODAudioManager::
stopping_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);

  _sounds_playing.erase(sound); // This could case the sound to destruct.
}

/**
 * Calls finished() on any sounds that have finished playing.
 */
void FMODAudioManager::
update_sounds() {
  ReMutexHolder holder(_lock);

  // See if any of our playing sounds have ended we must first collect a
  // seperate list of finished sounds and then iterated over those again
  // calling their finished method.  We can't call finished() within a loop
  // iterating over _sounds_playing since finished() modifies _sounds_playing
  SoundsPlaying sounds_finished;

  SoundsPlaying::iterator i = _sounds_playing.begin();
  for (; i != _sounds_playing.end(); ++i) {
    FMODAudioSound *sound = (*i);
    if (sound->status() != AudioSound::PLAYING) {
      sounds_finished.insert(*i);
    }
  }

  i = sounds_finished.begin();
  for (; i != sounds_finished.end(); ++i) {
    (**i).finished();
  }
}
