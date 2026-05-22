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
 * @author blucheeze
 * @date 2026-05-26
 */

#include "pandabase.h"
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "dcast.h"

// Panda headers.
#include "config_audio.h"
#include "config_putil.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "filename.h"
#include "virtualFileSystem.h"
#include "reMutexHolder.h"

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

#define FMOD_MIN_SAMPLE_RATE 80000
#define FMOD_MAX_SAMPLE_RATE 192000
#define USER_DSP_MAGIC ((void*)0x7012AB35)

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
  _cache_limit = audio_cache_limit;

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

    if (version < FMOD_VERSION) {
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

  // Release all of our sounds.  This drives all FMODAudioSound destructors which
  // call _release_sound_data, decrementing cache refcounts to 0 and releasing
  // individual sounds.
  _sounds_playing.clear();
  _all_sounds.clear();

  // Release all sounds sitting in the expiration queue.
  _discard_excess_cache(0);
  // _sound_data_cache and _sound_ref_counts are now empty for zero-refcount
  // entries; clear any remaining state.
  _sound_data_cache.clear();
  _sound_ref_counts.clear();

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
 * Configure the global DSP filter chain.
 *
 * FMOD has a relatively powerful DSP implementation.  It is likely that most
 * configurations will be supported.
 */
bool FMODAudioManager::
configure_filters(FilterProperties *config) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  FMOD::DSP *head;
  // FMOD Core API: Use getDSP(0) to get the head DSP instead of getDSPHead
  result = _channelgroup->getDSP(0, &head);
  if (result != 0) {
    audio_error("Getting DSP head: " << FMOD_ErrorString(result) );
    return false;
  }
  update_dsp_chain(head, config);
  return true;
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

  // Update the FMOD system
  _system->update();
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
 * Immediately evicts a sound from the expiration queue and releases it.
 * Only affects entries currently in the queue (refcount == 0).
 * Checks both the 2D and 3D variants of the given filename.
 */
void FMODAudioManager::
uncache_sound(const Filename &file_name) {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::uncache_sound(\"" << file_name << "\")");

  Filename path = file_name;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());
  path.set_binary();

  for (int p = 0; p <= 1; ++p) {
    bool positional = (p == 1);
    SoundCacheKey key{path, positional};
    SoundDataCache::iterator ci = _sound_data_cache.find(key);
    if (ci == _sound_data_cache.end()) continue;

    FMOD::Sound *sound = ci->second;
    SoundRefCounts::iterator ri = _sound_ref_counts.find(sound);
    if (ri == _sound_ref_counts.end() || ri->second != 0) continue;

    // Remove from expiration queue, then release.
    ExpirationQueue::iterator ei = _expiring_sounds.begin();
    while (ei != _expiring_sounds.end()) {
      if (ei->_path == path && ei->_positional == positional) {
        _expiring_sounds.erase(ei);
        break;
      }
      ++ei;
    }
    sound->release();
    _sound_ref_counts.erase(ri);
    _sound_data_cache.erase(ci);
    audio_debug("FMODAudioManager: uncache_sound evicted " << path
                << " positional=" << positional);
  }
}

/**
 * Discards all zero-reference-count entries from the sound cache immediately,
 * regardless of the cache limit.
 */
void FMODAudioManager::
clear_cache() {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::clear_cache()");
  _discard_excess_cache(0);
}

/**
 * Sets the maximum number of zero-refcount sounds to keep resident in the
 * expiration queue.  Sounds beyond this limit are released immediately.
 * 0 keeps nothing (every sound is freed as soon as its last user releases it).
 */
void FMODAudioManager::
set_cache_limit(unsigned int count) {
  ReMutexHolder holder(_lock);
  audio_debug("FMODAudioManager::set_cache_limit(count=" << count << ")");
  _cache_limit = count;
  _discard_excess_cache(count);
}

/**
 * Returns the maximum number of zero-refcount sounds kept in the cache.
 */
unsigned int FMODAudioManager::
get_cache_limit() const {
  return _cache_limit;
}

FMOD_RESULT FMODAudioManager::
get_speaker_mode(FMOD_SPEAKERMODE &mode) const {
  int num_samples;
  int num_raw_speakers;

  return _system->getSoftwareFormat(&num_samples, &mode,
                                    &num_raw_speakers);
}

/**
 * Converts a FilterConfig to an FMOD_DSP
 */
FMOD::DSP *FMODAudioManager::
make_dsp(const FilterProperties::FilterConfig &conf) {
  ReMutexHolder holder(_lock);
  FMOD_DSP_TYPE dsptype;
  FMOD_RESULT result;
  FMOD::DSP *dsp;
  switch (conf._type) {
  case FilterProperties::FT_lowpass:    dsptype = FMOD_DSP_TYPE_LOWPASS;     break;
  case FilterProperties::FT_highpass:   dsptype = FMOD_DSP_TYPE_HIGHPASS;    break;
  case FilterProperties::FT_echo:       dsptype = FMOD_DSP_TYPE_ECHO;        break;
  case FilterProperties::FT_flange:     dsptype = FMOD_DSP_TYPE_FLANGE;      break;
  case FilterProperties::FT_distort:    dsptype = FMOD_DSP_TYPE_DISTORTION;  break;
  case FilterProperties::FT_normalize:  dsptype = FMOD_DSP_TYPE_NORMALIZE;   break;
  case FilterProperties::FT_parameq:    dsptype = FMOD_DSP_TYPE_PARAMEQ;     break;
  case FilterProperties::FT_pitchshift: dsptype = FMOD_DSP_TYPE_PITCHSHIFT;  break;
  case FilterProperties::FT_chorus:     dsptype = FMOD_DSP_TYPE_CHORUS;      break;
  case FilterProperties::FT_sfxreverb:  dsptype = FMOD_DSP_TYPE_SFXREVERB;   break;
  case FilterProperties::FT_compress:   dsptype = FMOD_DSP_TYPE_COMPRESSOR;  break;
  case FilterProperties::FT_fader:      dsptype = FMOD_DSP_TYPE_FADER;       break;
  case FilterProperties::FT_limiter:    dsptype = FMOD_DSP_TYPE_LIMITER;     break;
  case FilterProperties::FT_pan:        dsptype = FMOD_DSP_TYPE_PAN;         break;
  case FilterProperties::FT_tremolo:    dsptype = FMOD_DSP_TYPE_TREMOLO;     break;
  case FilterProperties::FT_delay:      dsptype = FMOD_DSP_TYPE_DELAY;       break;
  default:
    audio_error("Garbage in DSP configuration data");
    return nullptr;
  }

  result = _system->createDSPByType( dsptype, &dsp);
  if (result != 0) {
    audio_error("Could not create DSP object");
    return nullptr;
  }

  FMOD_RESULT res1 = FMOD_OK;
  FMOD_RESULT res2 = FMOD_OK;
  FMOD_RESULT res3 = FMOD_OK;
  FMOD_RESULT res4 = FMOD_OK;
  FMOD_RESULT res5 = FMOD_OK;
  FMOD_RESULT res6 = FMOD_OK;
  FMOD_RESULT res7 = FMOD_OK;
  FMOD_RESULT res8 = FMOD_OK;
  FMOD_RESULT res9 = FMOD_OK;
  FMOD_RESULT res10 = FMOD_OK;
  FMOD_RESULT res11 = FMOD_OK;
  FMOD_RESULT res12 = FMOD_OK;
  FMOD_RESULT res13 = FMOD_OK;
  FMOD_RESULT res14 = FMOD_OK;
  FMOD_RESULT res15 = FMOD_OK;
  FMOD_RESULT res16 = FMOD_OK;
  FMOD_RESULT res17 = FMOD_OK;
  FMOD_RESULT res18 = FMOD_OK;
  FMOD_RESULT res19 = FMOD_OK;
  FMOD_RESULT res20 = FMOD_OK;
  FMOD_RESULT res21 = FMOD_OK;
  FMOD_RESULT res22 = FMOD_OK;

  switch (conf._type) {
  case FilterProperties::FT_lowpass:
    res1 = dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF,     conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE,  conf._b);
    break;
  case FilterProperties::FT_highpass:
    res1 = dsp->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF,    conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_HIGHPASS_RESONANCE, conf._b);
    break;
  case FilterProperties::FT_echo:
    res1 = dsp->setParameterFloat(FMOD_DSP_ECHO_DELAY,         conf._c);
    res2 = dsp->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK,      conf._d);
    res3 = dsp->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL,      conf._a);
    res4 = dsp->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL,      conf._b);
    break;
  case FilterProperties::FT_flange:
    res1 = dsp->setParameterFloat(FMOD_DSP_FLANGE_MIX,         conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_FLANGE_DEPTH,       conf._c);
    res3 = dsp->setParameterFloat(FMOD_DSP_FLANGE_RATE,        conf._d);
    break;
  case FilterProperties::FT_distort:
    res1 = dsp->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL,   conf._a);
    break;
  case FilterProperties::FT_normalize:
    res1 = dsp->setParameterFloat(FMOD_DSP_NORMALIZE_FADETIME,  conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_NORMALIZE_THRESHOLD,conf._b);
    res3 = dsp->setParameterFloat(FMOD_DSP_NORMALIZE_MAXAMP,    conf._c);
    break;
  case FilterProperties::FT_parameq:
    res1 = dsp->setParameterFloat(FMOD_DSP_PARAMEQ_CENTER,     conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_PARAMEQ_BANDWIDTH,  conf._b);
    res3 = dsp->setParameterFloat(FMOD_DSP_PARAMEQ_GAIN,       conf._c);
    break;
  case FilterProperties::FT_pitchshift:
    res1 = dsp->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH,   conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, conf._b);
    break;
  case FilterProperties::FT_chorus:
    res1 = dsp->setParameterFloat(FMOD_DSP_CHORUS_MIX,         conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_CHORUS_RATE,        conf._f);
    res3 = dsp->setParameterFloat(FMOD_DSP_CHORUS_DEPTH,       conf._g);
    break;
  case FilterProperties::FT_sfxreverb:
    res1 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, conf._d);
    res2 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, conf._g);
    res3 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, conf._i);
    res4 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, conf._l);
    res5 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HFDECAYRATIO, conf._e);
    res6 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, conf._j);
    res7 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, conf._k);
    res8 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFFREQUENCY, conf._n);
    res9 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_LOWSHELFGAIN, conf._m);
    res10 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_HIGHCUT, conf._c);
    res11 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYLATEMIX, conf._f);
    res12 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, conf._b);
    res13 = dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, conf._a);
    break;
  case FilterProperties::FT_compress:
    res1 = dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_THRESHOLD, conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_ATTACK,    conf._b);
    res3 = dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_RELEASE,   conf._c);
    res4 = dsp->setParameterFloat(FMOD_DSP_COMPRESSOR_GAINMAKEUP,conf._d);
    break;
  case FilterProperties::FT_fader:
    res1 = dsp->setParameterFloat(FMOD_DSP_FADER_GAIN, conf._a);
    break;
  case FilterProperties::FT_limiter:
    res1 = dsp->setParameterFloat(FMOD_DSP_LIMITER_RELEASETIME,   conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_LIMITER_CEILING,       conf._b);
    res3 = dsp->setParameterFloat(FMOD_DSP_LIMITER_MAXIMIZERGAIN, conf._c);
    res4 = dsp->setParameterBool  (FMOD_DSP_LIMITER_MODE, (bool)conf._d);
    break;
  case FilterProperties::FT_pan:
    // 22 of 24 FMOD_DSP_PAN parameters are set here.
    // FMOD_DSP_PAN_OVERALL_GAIN is internal/read-only and intentionally skipped.
    // FMOD_DSP_PAN_ATTENUATION_RANGE is read-only (FMOD-managed) and intentionally skipped.
    {
      // 2D parameters
      res1  = dsp->setParameterInt  (FMOD_DSP_PAN_MODE,                  (int)conf._a);
      res2  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_STEREO_POSITION,    conf._b);
      res3  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_DIRECTION,          conf._c);
      res4  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_EXTENT,             conf._d);
      res5  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_ROTATION,           conf._e);
      res6  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_LFE_LEVEL,          conf._f);
      res7  = dsp->setParameterInt  (FMOD_DSP_PAN_2D_STEREO_MODE,        (int)conf._g);
      res8  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_STEREO_SEPARATION,  conf._h);
      res9  = dsp->setParameterFloat(FMOD_DSP_PAN_2D_STEREO_AXIS,        conf._i);
      res10 = dsp->setParameterInt  (FMOD_DSP_PAN_ENABLED_SPEAKERS,      (int)conf._j);
      // 3D position uses FMOD_DSP_PARAMETER_3DATTRIBUTES_MULTI (multi-listener
      // variant).  numlisteners=1 covers the typical single-player case; the
      // multi-listener case (split-screen/VR) would need a richer API.
      // weight[0]=1.0 ensures this listener is fully active (zero-init gives 0).
      // velocity/forward/up are hardcoded to sensible point-source defaults
      // (no Doppler, standard FMOD orientation); expose via a future API if needed.
      FMOD_DSP_PARAMETER_3DATTRIBUTES_MULTI attrs = {};
      attrs.numlisteners = 1;
      attrs.relative[0].position = {conf._k, conf._l, conf._m};
      attrs.relative[0].velocity = {0, 0, 0};
      attrs.relative[0].forward  = {0, 0, 1};
      attrs.relative[0].up       = {0, 1, 0};
      attrs.weight[0] = 1.0f;
      res11 = dsp->setParameterData(FMOD_DSP_PAN_3D_POSITION, &attrs, sizeof(attrs));
      // 3D distance / extent parameters
      res12 = dsp->setParameterInt  (FMOD_DSP_PAN_3D_ROLLOFF,            (int)conf._n);
      res13 = dsp->setParameterFloat(FMOD_DSP_PAN_3D_MIN_DISTANCE,       conf._o);
      res14 = dsp->setParameterFloat(FMOD_DSP_PAN_3D_MAX_DISTANCE,       conf._p);
      res15 = dsp->setParameterInt  (FMOD_DSP_PAN_3D_EXTENT_MODE,        (int)conf._q);
      res16 = dsp->setParameterFloat(FMOD_DSP_PAN_3D_SOUND_SIZE,         conf._r);
      res17 = dsp->setParameterFloat(FMOD_DSP_PAN_3D_MIN_EXTENT,         conf._s);
      res18 = dsp->setParameterFloat(FMOD_DSP_PAN_3D_PAN_BLEND,          conf._t);
      // LFE, speaker mode, height, and range override
      res19 = dsp->setParameterInt  (FMOD_DSP_PAN_LFE_UPMIX_ENABLED,    (int)conf._u);
      res20 = dsp->setParameterInt  (FMOD_DSP_PAN_SURROUND_SPEAKER_MODE, (int)conf._v);
      res21 = dsp->setParameterFloat(FMOD_DSP_PAN_2D_HEIGHT_BLEND,       conf._w);
      res22 = dsp->setParameterBool (FMOD_DSP_PAN_OVERRIDE_RANGE,        (bool)conf._x);
    }
    break;
  case FilterProperties::FT_tremolo:
    res1 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_FREQUENCY, conf._a);
    res2 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_DEPTH,     conf._b);
    res3 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_SHAPE,     conf._c);
    res4 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_SKEW,      conf._d);
    res5 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_DUTY,      conf._e);
    res6 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_SQUARE,    conf._f);
    res7 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_PHASE,     conf._g);
    res8 = dsp->setParameterFloat(FMOD_DSP_TREMOLO_SPREAD,    conf._h);
    break;
  case FilterProperties::FT_delay:
    // FMOD requires maxdelay to be set before the channel delays.
    res1 = dsp->setParameterFloat(FMOD_DSP_DELAY_MAXDELAY, conf._c);
    res2 = dsp->setParameterFloat(FMOD_DSP_DELAY_CH0,      conf._a);
    res3 = dsp->setParameterFloat(FMOD_DSP_DELAY_CH1,      conf._b);
    break;
  }

  if ((res1!=FMOD_OK)||(res2!=FMOD_OK)||(res3!=FMOD_OK)||(res4!=FMOD_OK)||
      (res5!=FMOD_OK)||(res6!=FMOD_OK)||(res7!=FMOD_OK)||(res8!=FMOD_OK)||
      (res9!=FMOD_OK)||(res10!=FMOD_OK)||(res11!=FMOD_OK)||(res12!=FMOD_OK)||
      (res13!=FMOD_OK)||(res14!=FMOD_OK)||(res15!=FMOD_OK)||(res16!=FMOD_OK)||
      (res17!=FMOD_OK)||(res18!=FMOD_OK)||(res19!=FMOD_OK)||(res20!=FMOD_OK)||
      (res21!=FMOD_OK)||(res22!=FMOD_OK)) {
    audio_error("Could not configure DSP");
    dsp->release();
    return nullptr;
  }

  dsp->setUserData(USER_DSP_MAGIC);

  return dsp;
}

/**
 * Alters a DSP chain to make it match the specified configuration.
 */
void FMODAudioManager::
update_dsp_chain(FMOD::DSP *head, FilterProperties *config) {
  ReMutexHolder holder(_lock);
  const FilterProperties::ConfigVector &conf = config->get_config();
  FMOD_RESULT result;

  // FMOD Core API: Use ChannelGroup DSP list instead of traversing DSP graph
  // Get the number of DSPs in the channel group
  int numdsps = 0;
  result = _channelgroup->getNumDSPs(&numdsps);
  fmod_audio_errcheck("_channelgroup->getNumDSPs()", result);

  // Remove user DSPs by iterating backwards (to avoid index shifting)
  for (int i = numdsps - 1; i >= 0; i--) {
    FMOD::DSP *dsp;
    result = _channelgroup->getDSP(i, &dsp);
    fmod_audio_errcheck("_channelgroup->getDSP()", result);

    void *userdata;
    result = dsp->getUserData(&userdata);
    fmod_audio_errcheck("dsp->getUserData()", result);

    if (userdata == USER_DSP_MAGIC) {
      // This is a user DSP, remove it
      result = _channelgroup->removeDSP(dsp);
      fmod_audio_errcheck("_channelgroup->removeDSP()", result);
      result = dsp->release();
      fmod_audio_errcheck("dsp->release()", result);
    }
  }

  // Add new DSPs from the config
  for (int i=0; i<(int)(conf.size()); i++) {
    FMOD::DSP *dsp = make_dsp(conf[i]);
    result = _channelgroup->addDSP(i, dsp);
    fmod_audio_errcheck("_channelgroup->addDSP()", result);
  }
}

/**
 * Inform the manager that a sound is about to play.
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
 * Removes the indicated sound from the manager's list of sounds.
 */
void FMODAudioManager::
release_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);
  AllSounds::iterator ai = _all_sounds.find(sound);
  if (ai != _all_sounds.end()) {
    _all_sounds.erase(ai);
  }
}

/**
 * Returns a cached FMOD::Sound* for the given resolved path and positional flag,
 * incrementing its reference count.  Returns nullptr on cache miss.
 * Must be called under _lock.
 */
FMOD::Sound *FMODAudioManager::
_acquire_sound_data(const Filename &path, bool positional) {
  SoundCacheKey key{path, positional};
  SoundDataCache::iterator it = _sound_data_cache.find(key);
  if (it == _sound_data_cache.end()) {
    return nullptr;
  }
  FMOD::Sound *sound = it->second;
  int &refs = _sound_ref_counts[sound];
  if (refs == 0) {
    // Sound was in the expiration queue — pull it back out.
    ExpirationQueue::iterator ei = _expiring_sounds.begin();
    while (ei != _expiring_sounds.end()) {
      if (ei->_path == path && ei->_positional == positional) {
        _expiring_sounds.erase(ei);
        break;
      }
      ++ei;
    }
  }
  refs++;
  audio_debug("FMODAudioManager: cache HIT " << path
              << " positional=" << positional << " refs=" << refs);
  return sound;
}

/**
 * Registers a successfully preloaded FMOD::Sound* in the cache with refcount 1.
 * Only call for sounds where preload succeeded and the file is not MIDI.
 * Must be called under _lock.
 */
void FMODAudioManager::
_register_sound_data(const Filename &path, bool positional, FMOD::Sound *sound) {
  // Each instance sharing a cached sound applies its own loop/priority/distance
  // to its own FMOD::Channel* at play time.  The FMOD::Sound* itself is treated
  // as an immutable audio buffer after registration.
  _sound_data_cache[SoundCacheKey{path, positional}] = sound;
  _sound_ref_counts[sound] = 1;
  audio_debug("FMODAudioManager: cache STORE " << path
              << " positional=" << positional);
}

/**
 * Decrements the reference count for the given sound.  When the count reaches
 * zero the sound is moved into the expiration queue instead of being released
 * immediately.  discard_excess_cache() handles the actual FMOD release when
 * the queue exceeds _cache_limit.
 * Handles both cached and non-cached (streaming/blank) sounds.
 * Must be called under _lock.
 */
void FMODAudioManager::
_release_sound_data(FMOD::Sound *sound, const Filename &path, bool positional) {
  SoundRefCounts::iterator ri = _sound_ref_counts.find(sound);
  if (ri == _sound_ref_counts.end()) {
    // Not a cached sound (streaming or blank error fallback) — release directly.
    FMOD_RESULT result = sound->release();
    fmod_audio_errcheck("_sound->release() [uncached]", result);
    return;
  }

  ri->second--;
  audio_debug("FMODAudioManager: sound release " << path
              << " positional=" << positional << " refs=" << ri->second);
  if (ri->second > 0) {
    return; // still in use by other FMODAudioSound instances
  }

  // Refcount reached 0: move to expiration queue (LRU tail).
  _expiring_sounds.push_back(SoundCacheKey{path, positional});
  audio_debug("FMODAudioManager: cache EXPIRE (queued) " << path
              << " positional=" << positional
              << " queue_size=" << _expiring_sounds.size());

  _discard_excess_cache(_cache_limit);
}

/**
 * Frees zero-refcount cached sounds from the front of the expiration queue
 * until the queue size is at or below the given limit.
 * Must be called under _lock.
 */
void FMODAudioManager::
_discard_excess_cache(unsigned int limit) {
  while (_expiring_sounds.size() > limit) {
    const SoundCacheKey &key = _expiring_sounds.front();

    SoundDataCache::iterator ci = _sound_data_cache.find(key);
    if (ci != _sound_data_cache.end()) {
      FMOD::Sound *sound = ci->second;
      nassertv(_sound_ref_counts[sound] == 0);
      audio_debug("FMODAudioManager: cache RELEASE " << key._path
                  << " positional=" << key._positional);
      sound->release();
      _sound_ref_counts.erase(sound);
      _sound_data_cache.erase(ci);
    }
    _expiring_sounds.pop_front();
  }
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
