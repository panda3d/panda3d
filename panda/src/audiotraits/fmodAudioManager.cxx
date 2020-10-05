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
 * @author Brian Lach
 * @date 2020-10-04
 * Updated to FMOD Core.
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
#include "oscillatorDSP.h"

// FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>

#define USER_DSP_MAGIC ((void*)0x7012AB35)

TypeHandle FmodAudioManager::_type_handle;

ReMutex FmodAudioManager::_lock;
FMOD::System *FmodAudioManager::_system;

pset<FmodAudioManager *> FmodAudioManager::_all_managers;

bool FmodAudioManager::_system_is_valid = false;

PN_stdfloat FmodAudioManager::_doppler_factor = 1;
PN_stdfloat FmodAudioManager::_distance_factor = 1;
PN_stdfloat FmodAudioManager::_drop_off_factor = 1;

#define FMOD_MIN_SAMPLE_RATE 80000
#define FMOD_MAX_SAMPLE_RATE 192000

// Central dispatcher for audio errors.

void fmod_audio_errcheck(const char *context, FMOD_RESULT result) {
  if (result != 0) {
    audio_error(context << ": " << FMOD_ErrorString(result) );
  }
}

/**
 * Factory Function
 */
AudioManager *Create_FmodAudioManager() {
  audio_debug("Create_FmodAudioManager()");
  return new FmodAudioManager;
}


/**
 *
 */
FmodAudioManager::
FmodAudioManager() {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;

  // We need a temporary variable to check the FMOD version.
  unsigned int      version;

  _all_managers.insert(this);

  // Init 3D attributes
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

  _active = true;

  _saved_outputtype = FMOD_OUTPUTTYPE_AUTODETECT;

  if (_system == nullptr) {
    // Create the global FMOD System object.  This one object must be shared
    // by all FmodAudioManagers (this is particularly true on OSX, but the
    // FMOD documentation is unclear as to whether this is the intended design
    // on all systems).

    result = FMOD::System_Create(&_system);
    fmod_audio_errcheck("FMOD::System_Create()", result);

    // Let check the version of FMOD to make sure the headers and libraries
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

    fmodAudio_cat.debug()
      << "fmod-mixer-sample-rate: " << fmod_mixer_sample_rate << "\n";
    if (fmod_mixer_sample_rate.get_value() != -1) {
      if (fmod_mixer_sample_rate.get_value() >= FMOD_MIN_SAMPLE_RATE &&
          fmod_mixer_sample_rate.get_value() <= FMOD_MAX_SAMPLE_RATE) {
            sample_rate = fmod_mixer_sample_rate;
            fmodAudio_cat.debug()
              << "Using user specified sample rate\n";
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
FmodAudioManager::
~FmodAudioManager() {
  ReMutexHolder holder(_lock);
  // Be sure to delete associated sounds before deleting the manager!
  FMOD_RESULT result;

  // Release Sounds Next
  _all_sounds.clear();

  // Release all DSPs
  _dsps.clear();

  // Remove me from the managers list.
  _all_managers.erase(this);

  if (_all_managers.empty()) {
    result = _system->release();
    fmod_audio_errcheck("_system->release()", result);
    _system = nullptr;
    _system_is_valid = false;
  }
}

/**
 * Returns the FMOD DSP type from the Panda DSP type.
 */
FMOD_DSP_TYPE FmodAudioManager::
get_fmod_dsp_type(DSP::DSPType panda_type) const {
  switch (panda_type) {
  case DSP::DT_chorus:
    return FMOD_DSP_TYPE_CHORUS;
  case DSP::DT_compressor:
    return FMOD_DSP_TYPE_COMPRESSOR;
  case DSP::DT_delay:
    return FMOD_DSP_TYPE_DELAY;
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
  default:
    return FMOD_DSP_TYPE_UNKNOWN;
  }
}

/**
 * Configures the FMOD DSP based on the parameters in the Panda DSP.
 */
void FmodAudioManager::
configure_dsp(DSP *dsp_conf, FMOD::DSP *dsp) {
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
      dsp->setParameterBool(FMOD_DSP_COMPRESSOR_LINKED, comp_conf->get_linked());
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
      dsp->setParameterBool(FMOD_DSP_LIMITER_MODE, lim_conf->get_linked());
    }
    break;
  case DSP::DT_lowpass:
    {
      LowpassDSP *lp_conf = DCAST(LowpassDSP, dsp_conf);
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, lp_conf->get_cutoff());
      dsp->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, lp_conf->get_resonance());
    }
    break;
  case DSP::DT_oscillator:
    {
      OscillatorDSP *osc_conf = DCAST(OscillatorDSP, dsp_conf);
      dsp->setParameterInt(FMOD_DSP_OSCILLATOR_TYPE, osc_conf->get_oscillator_type());
      dsp->setParameterFloat(FMOD_DSP_OSCILLATOR_RATE, osc_conf->get_rate());
    }
    break;
  }
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool FmodAudioManager::
insert_dsp(int index, DSP *panda_dsp) {
  auto itr = _dsps.find(panda_dsp);
  if (itr != _dsps.end()) {
    // DSP already in chain.
    return false;
  }

  FMOD_DSP_TYPE fmod_type = get_fmod_dsp_type(panda_dsp->get_dsp_type());
  if (fmod_type == FMOD_DSP_TYPE_UNKNOWN) {
    fmodAudio_cat.warning()
      << panda_dsp->get_type().get_name()
      << " unsupported by FMOD audio implementation.\n";
    return false;
  }

  FMOD_RESULT ret;
  FMOD::DSP *dsp;
  ret = _system->createDSPByType(fmod_type, &dsp);
  fmod_audio_errcheck("_system->createDSPByType()", ret);

  ret = dsp->setUserData(USER_DSP_MAGIC);
  fmod_audio_errcheck("dsp->setUserData()", ret);

  ret = _channelgroup->addDSP(index, dsp);
  fmod_audio_errcheck("_channelgroup->addDSP()", ret);

  configure_dsp(panda_dsp, dsp);

  // Keep a mapping between the FMOD DSP and the Panda DSP.
  _dsps[panda_dsp] = dsp;

  return true;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool FmodAudioManager::
remove_dsp(DSP *panda_dsp) {
  auto itr = _dsps.find(panda_dsp);
  if (itr == _dsps.end()) {
    // DSP was not in chain.
    return false;
  }

  FMOD::DSP *dsp = itr->second;
  FMOD_RESULT ret;
  ret = _channelgroup->removeDSP(dsp);
  fmod_audio_errcheck("_channelGroup->removeDSP()", ret);
  ret = dsp->release();
  fmod_audio_errcheck("dsp->release()", ret);

  _dsps.erase(itr);

  return true;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void FmodAudioManager::
remove_all_dsps() {
  FMOD_RESULT ret;
  FMOD::DSP *dsp;
  int num_dsps = get_num_dsps();
  for (int i = 0; i < num_dsps; i++) {
    ret = _channelgroup->getDSP(i, &dsp);
    fmod_audio_errcheck("_channelgroup->getDSP()", ret);

    if (dsp) {
      ret = _channelgroup->removeDSP(dsp);
      fmod_audio_errcheck("_channelgroup->removeDSP()", ret);
      ret = dsp->release();
      fmod_audio_errcheck("dsp->release()", ret);
    }
  }

  _dsps.clear();
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int FmodAudioManager::
get_num_dsps() const {
  int count;
  FMOD_RESULT ret = _channelgroup->getNumDSPs(&count);
  fmod_audio_errcheck("_channelgroup->getNumDSPs()", ret);
  return count;
}

/**
 * This just check to make sure the FMOD System is up and running correctly.
 */
bool FmodAudioManager::
is_valid() {
  return _is_valid;
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) FmodAudioManager::
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
    PT(FmodAudioSound) sound = new FmodAudioSound(this, file, positional);

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
PT(AudioSound) FmodAudioManager::
get_sound(MovieAudio *source, bool positional, int) {
  nassert_raise("FMOD audio manager does not support MovieAudio sources");
  return nullptr;
}

/**
 * This is to query if you are using a MultiChannel Setup.
 */
int FmodAudioManager::
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
    return 0;
  case FMOD_SPEAKERMODE_MONO:
    return 1;
  case FMOD_SPEAKERMODE_STEREO:
    return 2;
  case FMOD_SPEAKERMODE_QUAD:
    return 3;
  case FMOD_SPEAKERMODE_SURROUND:
    return 4;
  case FMOD_SPEAKERMODE_5POINT1:
    return 5;
  case FMOD_SPEAKERMODE_7POINT1:
    return 6;
  case FMOD_SPEAKERMODE_MAX:
    return 7;
  default:
    return 8;
  }
}

/**
 * This is to set up FMOD to use a MultiChannel Setup.  This method is pretty
 * much useless.  To set a speaker setup in FMOD for Surround Sound, stereo,
 * or whatever you have to set the SpeakerMode BEFORE you Initialize FMOD.
 * Since Panda Inits the FmodAudioManager right when you Start it up, you are
 * never given an oppertunity to call this function.  That is why I stuck a
 * BOOL in the CONFIG.PRC file, whichs lets you flag if you want to use a
 * Multichannel or not.  That will set the speaker setup when an instance of
 * this class is constructed.  Still I put this here as a measure of good
 * faith, since you can query the speaker setup after everything in Init.
 * Also, maybe someone will completely hack Panda someday, in which one can
 * init or re-init the AudioManagers after Panda is running.
 */
void FmodAudioManager::
set_speaker_setup(AudioManager::SpeakerModeCategory cat) {
  ///ReMutexHolder holder(_lock);
  //FMOD_RESULT result;
  //FMOD_SPEAKERMODE speakerModeType = (FMOD_SPEAKERMODE)cat;
  //result = _system->setSpeakerMode( speakerModeType);
  //fmod_audio_errcheck("_system->setSpeakerMode()", result);
  fmodAudio_cat.warning()
    << "FmodAudioManager::set_speaker_setup() doesn't do anything\n";
}

/**
 * Sets the volume of the AudioManager.  It is not an override, but a
 * multiplier.
 */
void FmodAudioManager::
set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  result = _channelgroup->setVolume(volume);
  fmod_audio_errcheck("_channelgroup->setVolume()", result);
}

/**
 * Returns the AudioManager's volume.
 */
PN_stdfloat FmodAudioManager::
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
void FmodAudioManager::
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
 * Turn on/off Warning: not implemented.
 */
void FmodAudioManager::
set_active(bool active) {
  ReMutexHolder holder(_lock);
  if (_active != active) {
    _active = active;

    // Tell our AudioSounds to adjust:
    for (SoundSet::iterator i = _all_sounds.begin();
         i != _all_sounds.end();
         ++i) {
      (*i)->set_active(_active);
    }
  }
}

/**
 *
 */
bool FmodAudioManager::
get_active() const {
  return _active;
}

/**
 * Stop playback on all sounds managed by this manager.
 */
void FmodAudioManager::
stop_all_sounds() {
  ReMutexHolder holder(_lock);
  // We have to walk through this list with some care, since stopping a sound
  // may also remove it from the set (if there are no other references to the
  // sound).
  SoundSet::iterator i;
  i = _all_sounds.begin();
  while (i != _all_sounds.end()) {
    SoundSet::iterator next = i;
    ++next;

    (*i)->stop();
    i = next;
  }
}

/**
 * Perform all per-frame update functions.
 */
void FmodAudioManager::
update() {
  ReMutexHolder holder(_lock);
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
void FmodAudioManager::
audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_set_listener_attributes()");

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
void FmodAudioManager::
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  audio_error("audio3dGetListenerAttributes: currently unimplemented. Get the attributes of the attached object");

}


/**
 * Set units per meter (Fmod uses meters internally for its sound-
 * spacialization calculations)
 */
void FmodAudioManager::
audio_3d_set_distance_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug( "FmodAudioManager::audio_3d_set_distance_factor( factor= " << factor << ")" );

  FMOD_RESULT result;

  _distance_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);


}

/**
 * Gets units per meter (Fmod uses meters internally for its sound-
 * spacialization calculations)
 */
PN_stdfloat FmodAudioManager::
audio_3d_get_distance_factor() const {
  audio_debug("FmodAudioManager::audio_3d_get_distance_factor()");

  return _distance_factor;
}

/**
 * Exaggerates or diminishes the Doppler effect.  Defaults to 1.0
 */
void FmodAudioManager::
audio_3d_set_doppler_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_set_doppler_factor(factor="<<factor<<")");

  FMOD_RESULT result;

  _doppler_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);

}

/**
 *
 */
PN_stdfloat FmodAudioManager::
audio_3d_get_doppler_factor() const {
  audio_debug("FmodAudioManager::audio_3d_get_doppler_factor()");

  return _doppler_factor;
}

/**
 * Control the effect distance has on audability.  Defaults to 1.0
 */
void FmodAudioManager::
audio_3d_set_drop_off_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_set_drop_off_factor("<<factor<<")");

  FMOD_RESULT result;

  _drop_off_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);

}

/**
 *
 */
PN_stdfloat FmodAudioManager::
audio_3d_get_drop_off_factor() const {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_get_drop_off_factor()");

  return _drop_off_factor;

}



/**
 * NOT USED FOR FMOD-EX!!!
 */
void FmodAudioManager::
set_concurrent_sound_limit(unsigned int limit) {

}

/**
 * NOT USED FOR FMOD-EX!!!
 */
unsigned int FmodAudioManager::
get_concurrent_sound_limit() const {
  return 1000000;
}

/**
 * NOT USED FOR FMOD-EX!!!
 */
void FmodAudioManager::
reduce_sounds_playing_to(unsigned int count) {

}


/**
 * NOT USED FOR FMOD-EX!!! Clears a sound out of the sound cache.
 */
void FmodAudioManager::
uncache_sound(const Filename &file_name) {
  audio_debug("FmodAudioManager::uncache_sound(\""<<file_name<<"\")");

}


/**
 * NOT USED FOR FMOD-EX!!! Clear out the sound cache.
 */
void FmodAudioManager::
clear_cache() {
  audio_debug("FmodAudioManager::clear_cache()");

}

/**
 * NOT USED FOR FMOD-EX!!! Set the number of sounds that the cache can hold.
 */
void FmodAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("FmodAudioManager::set_cache_limit(count="<<count<<")");

}

/**
 * NOT USED FOR FMOD-EX!!! Gets the number of sounds that the cache can hold.
 */
unsigned int FmodAudioManager::
get_cache_limit() const {
  audio_debug("FmodAudioManager::get_cache_limit() returning ");
  // return _cache_limit;
  return 0;
}

FMOD_RESULT FmodAudioManager::
get_speaker_mode(FMOD_SPEAKERMODE &mode) const {
  int num_samples;
  int num_raw_speakers;

  return _system->getSoftwareFormat(&num_samples, &mode,
                                    &num_raw_speakers);
}
