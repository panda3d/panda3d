// Filename: fmodAudioManager.cxx
// Created by:  cort (January 22, 2003)
// Prior system by: cary
// Rewrite [for new Version of FMOD-EX] by: Stan Rosenbaum "Staque" - Spring 2006
//
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "dcast.h"

//Panda headers.
#include "config_audio.h"
#include "config_util.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "filename.h"
#include "virtualFileSystem.h"
#include "reMutexHolder.h"

//FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>

#define USER_DSP_MAGIC ((void*)0x7012AB35)

TypeHandle FmodAudioManager::_type_handle;

ReMutex FmodAudioManager::_lock;
FMOD::System *FmodAudioManager::_system; 

pset<FmodAudioManager *> FmodAudioManager::_all_managers;

bool FmodAudioManager::_system_is_valid = false;


//  This sets the distance factor for 3D audio to use feet. 
//  FMOD uses meters by default.
//  Since Panda use feet we need to compensate for that with a factor of 3.28
//
//  This can be overwritten.  You just need to call
//  audio_3d_set_distance_factor(PN_stdfloat factor) and set your new factor.

PN_stdfloat FmodAudioManager::_doppler_factor = 1;
PN_stdfloat FmodAudioManager::_distance_factor = 3.28;
PN_stdfloat FmodAudioManager::_drop_off_factor = 1;


////////////////////////////////////////////////////////////////////
// Central dispatcher for audio errors.
////////////////////////////////////////////////////////////////////

void fmod_audio_errcheck(const char *context, FMOD_RESULT result) {
  if (result != 0) {
    audio_error(context << ": " << FMOD_ErrorString(result) );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Create_FmodAudioManager
//       Access: Private
//  Description: Factory Function
////////////////////////////////////////////////////////////////////
AudioManager *Create_FmodAudioManager() {
  audio_debug("Create_FmodAudioManager()");
  return new FmodAudioManager;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
FmodAudioManager() {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;

  //We need a varible temporary to check the FMOD Version.
  unsigned int      version;

  _all_managers.insert(this);
  
  //Init 3D attributes
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

  _saved_outputtype = FMOD_OUTPUTTYPE_AUTODETECT;

  if (_system == (FMOD::System *)NULL) {
    // Create the global FMOD System object.  This one object must be
    // shared by all FmodAudioManagers (this is particularly true on
    // OSX, but the FMOD documentation is unclear as to whether this
    // is the intended design on all systems).

    result = FMOD::System_Create(&_system);
    fmod_audio_errcheck("FMOD::System_Create()", result);

    //  Let check the Version of FMOD to make sure the Headers and Libraries are correct.
    result = _system->getVersion(&version);
    fmod_audio_errcheck("_system->getVersion()", result);
  
    if (version < FMOD_VERSION){
      audio_error("You are using an old version of FMOD.  This program requires:" << FMOD_VERSION);
    }

    // Set speaker mode.
    if (fmod_speaker_mode.get_value() == FSM_unspecified) {
      if (fmod_use_surround_sound) {
        // fmod-use-surround-sound is the old variable, now replaced
        // by fmod-speaker-mode.  This is for backward compatibility.
        result = _system->setSpeakerMode(FMOD_SPEAKERMODE_5POINT1);
        fmod_audio_errcheck("_system->setSpeakerMode()", result);
      }
    } else {
      FMOD_SPEAKERMODE speakerMode;
      speakerMode = (FMOD_SPEAKERMODE) fmod_speaker_mode.get_value();
      result = _system->setSpeakerMode(speakerMode);
      fmod_audio_errcheck("_system->setSpeakerMode()", result);
    }

    //Now we Initialize the System.
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
  // Here's a big kludge.  Don't ever let FMOD try to load this
  // OSX-provided file; it crashes messily if you do.
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

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
~FmodAudioManager() {
  ReMutexHolder holder(_lock);
  // Be sure to delete associated sounds before deleting the manager!
  FMOD_RESULT result;

  //Release Sounds Next
  _all_sounds.clear();

  // Remove me from the managers list.
  _all_managers.erase(this);

  if (_all_managers.empty()) {
    result = _system->release();
    fmod_audio_errcheck("_system->release()", result);
    _system = NULL;
    _system_is_valid = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::is_valid
//       Access: Public
//  Description: This just check to make sure the FMOD System is 
//         up and running correctly.
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
is_valid() {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::make_dsp
//       Access: Private
//  Description: Converts a FilterConfig to an FMOD_DSP
////////////////////////////////////////////////////////////////////
FMOD::DSP *FmodAudioManager::
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
  default:
    audio_error("Garbage in DSP configuration data");
    return NULL;
  }

  result = _system->createDSPByType( dsptype, &dsp);
  if (result != 0) {
    audio_error("Could not create DSP object");
    return NULL;
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

  switch (conf._type) {
  case FilterProperties::FT_lowpass:
    res1 = dsp->setParameter(FMOD_DSP_LOWPASS_CUTOFF,     conf._a);
    res2 = dsp->setParameter(FMOD_DSP_LOWPASS_RESONANCE,  conf._b);
    break;
  case FilterProperties::FT_highpass:
    res1 = dsp->setParameter(FMOD_DSP_HIGHPASS_CUTOFF,    conf._a);
    res2 = dsp->setParameter(FMOD_DSP_HIGHPASS_RESONANCE, conf._b);
    break;
  case FilterProperties::FT_echo:
    res1 = dsp->setParameter(FMOD_DSP_ECHO_DRYMIX,        conf._a);
    res2 = dsp->setParameter(FMOD_DSP_ECHO_WETMIX,        conf._b);
    res3 = dsp->setParameter(FMOD_DSP_ECHO_DELAY,         conf._c);
    res4 = dsp->setParameter(FMOD_DSP_ECHO_DECAYRATIO,    conf._d);
    break;
  case FilterProperties::FT_flange:
    res1 = dsp->setParameter(FMOD_DSP_FLANGE_DRYMIX,      conf._a);
    res2 = dsp->setParameter(FMOD_DSP_FLANGE_WETMIX,      conf._b);
    res3 = dsp->setParameter(FMOD_DSP_FLANGE_DEPTH,       conf._c);
    res4 = dsp->setParameter(FMOD_DSP_FLANGE_RATE,        conf._d);
    break;
  case FilterProperties::FT_distort:
    res1 = dsp->setParameter(FMOD_DSP_DISTORTION_LEVEL,   conf._a);
    break;
  case FilterProperties::FT_normalize:
    res1 = dsp->setParameter(FMOD_DSP_NORMALIZE_FADETIME,  conf._a);
    res2 = dsp->setParameter(FMOD_DSP_NORMALIZE_THRESHHOLD,conf._b);
    res3 = dsp->setParameter(FMOD_DSP_NORMALIZE_MAXAMP,    conf._c);
    break;
  case FilterProperties::FT_parameq:
    res1 = dsp->setParameter(FMOD_DSP_PARAMEQ_CENTER,     conf._a);
    res2 = dsp->setParameter(FMOD_DSP_PARAMEQ_BANDWIDTH,  conf._b);
    res3 = dsp->setParameter(FMOD_DSP_PARAMEQ_GAIN,       conf._c);
    break;
  case FilterProperties::FT_pitchshift:
    res1 = dsp->setParameter(FMOD_DSP_PITCHSHIFT_PITCH,   conf._a);
    res2 = dsp->setParameter(FMOD_DSP_PITCHSHIFT_FFTSIZE, conf._b);
    res3 = dsp->setParameter(FMOD_DSP_PITCHSHIFT_OVERLAP, conf._c);
    break;
  case FilterProperties::FT_chorus:
    res1 = dsp->setParameter(FMOD_DSP_CHORUS_DRYMIX,      conf._a);
    res2 = dsp->setParameter(FMOD_DSP_CHORUS_WETMIX1,     conf._b);
    res3 = dsp->setParameter(FMOD_DSP_CHORUS_WETMIX2,     conf._c);
    res4 = dsp->setParameter(FMOD_DSP_CHORUS_WETMIX3,     conf._d);
    res5 = dsp->setParameter(FMOD_DSP_CHORUS_DELAY,       conf._e);
    res6 = dsp->setParameter(FMOD_DSP_CHORUS_RATE,        conf._f);
    res7 = dsp->setParameter(FMOD_DSP_CHORUS_DEPTH,       conf._g);
    break;
  case FilterProperties::FT_sfxreverb:
    res1 = dsp->setParameter(FMOD_DSP_SFXREVERB_DRYLEVEL, conf._a);
    res2 = dsp->setParameter(FMOD_DSP_SFXREVERB_ROOM,     conf._b);
    res3 = dsp->setParameter(FMOD_DSP_SFXREVERB_ROOMHF,   conf._c);
    res4 = dsp->setParameter(FMOD_DSP_SFXREVERB_DECAYTIME,conf._d);
    res5 = dsp->setParameter(FMOD_DSP_SFXREVERB_DECAYHFRATIO,    conf._e);
    res6 = dsp->setParameter(FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL,conf._f);
    res7 = dsp->setParameter(FMOD_DSP_SFXREVERB_REFLECTIONSDELAY,conf._g);
    res8 = dsp->setParameter(FMOD_DSP_SFXREVERB_REVERBLEVEL,     conf._h);
    res9 = dsp->setParameter(FMOD_DSP_SFXREVERB_REVERBDELAY,     conf._i);
    res10 = dsp->setParameter(FMOD_DSP_SFXREVERB_DIFFUSION,      conf._j);
    res11 = dsp->setParameter(FMOD_DSP_SFXREVERB_DENSITY,        conf._k);
    res12 = dsp->setParameter(FMOD_DSP_SFXREVERB_HFREFERENCE,    conf._l);
    res13 = dsp->setParameter(FMOD_DSP_SFXREVERB_ROOMLF,         conf._m);
    res14 = dsp->setParameter(FMOD_DSP_SFXREVERB_LFREFERENCE,    conf._n);
    break;
  case FilterProperties::FT_compress:
    res1 = dsp->setParameter(FMOD_DSP_COMPRESSOR_THRESHOLD, conf._a);
    res2 = dsp->setParameter(FMOD_DSP_COMPRESSOR_ATTACK,    conf._b);
    res3 = dsp->setParameter(FMOD_DSP_COMPRESSOR_RELEASE,   conf._c);
    res4 = dsp->setParameter(FMOD_DSP_COMPRESSOR_GAINMAKEUP,conf._d);
    break;
  }

  if ((res1!=FMOD_OK)||(res2!=FMOD_OK)||(res3!=FMOD_OK)||(res4!=FMOD_OK)||
      (res5!=FMOD_OK)||(res6!=FMOD_OK)||(res7!=FMOD_OK)||(res8!=FMOD_OK)||
      (res9!=FMOD_OK)||(res10!=FMOD_OK)||(res11!=FMOD_OK)||(res12!=FMOD_OK)||
      (res13!=FMOD_OK)||(res14!=FMOD_OK)) {
    audio_error("Could not configure DSP");
    dsp->release();
    return NULL;
  }

  dsp->setUserData(USER_DSP_MAGIC);
  
  return dsp;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::update_dsp_chain
//       Access: Public
//  Description: Alters a DSP chain to make it match the specified
//               configuration.
//
//               This is an inadequate implementation - it just
//               clears the whole DSP chain and rebuilds it from
//               scratch.  A better implementation would compare
//               the existing DSP chain to the desired one, and
//               make incremental changes.  This would prevent
//               a "pop" sound when the changes are made.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
update_dsp_chain(FMOD::DSP *head, FilterProperties *config) {
  ReMutexHolder holder(_lock);
  const FilterProperties::ConfigVector &conf = config->get_config();
  FMOD_RESULT result;

  while (1) {
    int numinputs;
    result = head->getNumInputs(&numinputs);
    fmod_audio_errcheck("head->getNumInputs()", result);
    if (numinputs != 1) {
      break;
    }
    FMOD::DSP *prev;
    result = head->getInput(0, &prev, NULL);
    fmod_audio_errcheck("head->getInput()", result);
    void *userdata;
    result = prev->getUserData(&userdata);
    fmod_audio_errcheck("prev->getUserData()", result);
    if (userdata != USER_DSP_MAGIC) {
      break;
    }
    result = prev->remove();
    fmod_audio_errcheck("prev->remove()", result);
    result = prev->release();
    fmod_audio_errcheck("prev->release()", result);
  }

  for (int i=0; i<(int)(conf.size()); i++) {
    FMOD::DSP *dsp = make_dsp(conf[i]);
    result = _channelgroup->addDSP(dsp, NULL);
    fmod_audio_errcheck("_channelgroup->addDSP()", result);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::configure_filters
//       Access: Public
//  Description: Configure the global DSP filter chain.
//
//               FMOD has a relatively powerful DSP 
//               implementation.  It is likely that most 
//               configurations will be supported.
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
configure_filters(FilterProperties *config) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  FMOD::DSP *head;
  result = _channelgroup->getDSPHead(&head);
  if (result != 0) {
    audio_error("Getting DSP head: " << FMOD_ErrorString(result) );
    return false;
  }
  update_dsp_chain(head, config);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_sound
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(const string &file_name, bool positional, int) {
  ReMutexHolder holder(_lock);
  //Needed so People use Panda's Generic UNIX Style Paths for Filename.
  //path.to_os_specific() converts it back to the proper OS version later on.
  
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  // Build a new AudioSound from the audio data.
  PT(AudioSound) audioSound = 0;
  PT(FmodAudioSound) fmodAudioSound = new FmodAudioSound(this, path, positional );

  _all_sounds.insert(fmodAudioSound);

  audioSound = fmodAudioSound;

  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_sound
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(MovieAudio *source, bool positional, int) {
  nassert_raise("FMOD audio manager does not support MovieAudio sources");
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::getSpeakerSetup()
//       Access: Published
//  Description: This is to query if you are using a MultiChannel Setup.
////////////////////////////////////////////////////////////////////
int FmodAudioManager::
getSpeakerSetup() {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  FMOD_SPEAKERMODE speakerMode;
  int returnMode;

  result = _system->getSpeakerMode( &speakerMode );
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  switch (speakerMode) {
    case  FMOD_SPEAKERMODE_RAW:
      returnMode = 0;
      break;
    case  FMOD_SPEAKERMODE_MONO:
      returnMode = 1;
      break;
    case  FMOD_SPEAKERMODE_STEREO:
      returnMode = 2;
      break;
    case  FMOD_SPEAKERMODE_QUAD:
      returnMode = 3;
      break;
    case  FMOD_SPEAKERMODE_SURROUND:
      returnMode = 4;
      break;
    case  FMOD_SPEAKERMODE_5POINT1:
      returnMode = 5;
      break;
    case  FMOD_SPEAKERMODE_7POINT1:
      returnMode = 6;
      break;
    case  FMOD_SPEAKERMODE_MAX:
      returnMode = 7;
      break;
    default:
      returnMode = -1;
    }

  return returnMode;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::setSpeakerSetup()
//       Access: Published
//  Description: This is to set up FMOD to use a MultiChannel Setup.
//         This method is pretty much useless.
//         To set a speaker setup in FMOD for Surround Sound, 
//         stereo, or whatever you have to set the SpeakerMode
//         BEFORE you Initialize FMOD.
//         Since Panda Inits the FmodAudioManager right when you
//         Start it up, you are never given an oppertunity to call
//         this function.
//         That is why I stuck a BOOL in the CONFIG.PRC file, whichs
//         lets you flag if you want to use a Multichannel or not.
//         That will set the speaker setup when an instance of this
//         class is constructed.
//         Still I put this here as a measure of good faith, since you
//         can query the speaker setup after everything in Init.
//         Also, maybe someone will completely hack Panda someday, in which
//         one can init or re-init the AudioManagers after Panda is running.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
setSpeakerSetup(AudioManager::SpeakerModeCategory cat) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  FMOD_SPEAKERMODE speakerModeType = (FMOD_SPEAKERMODE)cat;
  result = _system->setSpeakerMode( speakerModeType);
  fmod_audio_errcheck("_system->setSpeakerMode()", result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_volume(PN_stdfloat volume)
//       Access: Public
//  Description: Sets the volume of the AudioManager.
//               It is not an override, but a multiplier.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  result = _channelgroup->setVolume(volume);
  fmod_audio_errcheck("_channelgroup->setVolume()", result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_volume()
//       Access: Public
//  Description: Returns the AudioManager's volume.
////////////////////////////////////////////////////////////////////
PN_stdfloat FmodAudioManager::
get_volume() const {
  ReMutexHolder holder(_lock);
  float volume;
  FMOD_RESULT result;
  result = _channelgroup->getVolume(&volume);
  fmod_audio_errcheck("_channelgroup->getVolume()", result);
  return (PN_stdfloat)volume;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_wavwriter()
//       Access: Public
//  Description: Changes output mode to write all audio to a wav file.
////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_active(bool active)
//       Access: Public
//  Description: Turn on/off
//               Warning: not implemented.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_active()
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::stop_all_sounds()
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
stop_all_sounds() {
  ReMutexHolder holder(_lock);
  // We have to walk through this list with some care, since stopping
  // a sound may also remove it from the set (if there are no other
  // references to the sound).
  SoundSet::iterator i;
  i = _all_sounds.begin();
  while (i != _all_sounds.end()) {
    SoundSet::iterator next = i;
    ++next;

    (*i)->stop();
    i = next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::update
//       Access: Public
//  Description: Perform all per-frame update functions.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
update() {
  ReMutexHolder holder(_lock);
  _system->update();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_set_listener_attributes
//       Access: Public
//  Description: Set position of the "ear" that picks up 3d sounds
//        NOW LISTEN UP!!! THIS IS IMPORTANT!
//        Both Panda3D and FMOD use a left handed coordinate system.
//        But there is a major difference!
//        In Panda3D the Y-Axis is going into the Screen and the Z-Axis is going up.
//        In FMOD the Y-Axis is going up and the Z-Axis is going into the screen.
//        The solution is simple, we just flip the Y and Z axis, as we move coordinates
//        from Panda to FMOD and back.
//        What does did mean to average Panda user?  Nothing, they shouldn't notice anyway.
//        But if you decide to do any 3D audio work in here you have to keep it in mind.
//        I told you, so you can't say I didn't.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: Get position of the "ear" that picks up 3d sounds
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  audio_error("audio3dGetListenerAttributes: currently unimplemented. Get the attributes of the attached object");

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: Set units per meter (Fmod uses meters internally for
//               its sound-spacialization calculations)
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_set_distance_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug( "FmodAudioManager::audio_3d_set_distance_factor( factor= " << factor << ")" );
  
  FMOD_RESULT result;

  _distance_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);


}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: Gets units per meter (Fmod uses meters internally for
//               its sound-spacialization calculations)
////////////////////////////////////////////////////////////////////
PN_stdfloat FmodAudioManager::
audio_3d_get_distance_factor() const {
  audio_debug("FmodAudioManager::audio_3d_get_distance_factor()");

  return _distance_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_set_doppler_factor
//       Access: Public
//  Description: Exaggerates or diminishes the Doppler effect. 
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_set_doppler_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_set_doppler_factor(factor="<<factor<<")");

  FMOD_RESULT result;

  _doppler_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat FmodAudioManager::
audio_3d_get_doppler_factor() const {
  audio_debug("FmodAudioManager::audio_3d_get_doppler_factor()");

  return _doppler_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_set_drop_off_factor
//       Access: Public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_set_drop_off_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_set_drop_off_factor("<<factor<<")");

  FMOD_RESULT result;

  _drop_off_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck("_system->set3DSettings()", result);

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat FmodAudioManager::
audio_3d_get_drop_off_factor() const {
  ReMutexHolder holder(_lock);
  audio_debug("FmodAudioManager::audio_3d_get_drop_off_factor()");

  return _drop_off_factor;

}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description:  NOT USED FOR FMOD-EX!!!
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_concurrent_sound_limit(unsigned int limit) {

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_concurrent_sound_limit
//       Access: Public
//  Description: NOT USED FOR FMOD-EX!!!
////////////////////////////////////////////////////////////////////
unsigned int FmodAudioManager::
get_concurrent_sound_limit() const {
  return 1000000;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::reduce_sounds_playing_to
//       Access: Private
//  Description: NOT USED FOR FMOD-EX!!!
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
reduce_sounds_playing_to(unsigned int count) {

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::uncache_sound
//       Access: Public
//  Description: NOT USED FOR FMOD-EX!!!
//         Clears a sound out of the sound cache.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
uncache_sound(const string& file_name) {
  audio_debug("FmodAudioManager::uncache_sound(\""<<file_name<<"\")");

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::clear_cache
//       Access: Public
//  Description: NOT USED FOR FMOD-EX!!!
//         Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
clear_cache() {
  audio_debug("FmodAudioManager::clear_cache()");
  
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_cache_limit
//       Access: Public
//  Description: NOT USED FOR FMOD-EX!!!
//         Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("FmodAudioManager::set_cache_limit(count="<<count<<")");

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_cache_limit
//       Access: Public
//  Description: NOT USED FOR FMOD-EX!!!
//         Gets the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
unsigned int FmodAudioManager::
get_cache_limit() const {
  audio_debug("FmodAudioManager::get_cache_limit() returning ");
  //return _cache_limit;
  return 0;
}
