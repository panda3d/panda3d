// Filename: fmodAudioManager.cxx
// Created by:  cort (January 22, 2003)
// Prior system by: cary
// Rewrite [for new Version of FMOD-EX] by: Stan Rosenbaum "Staque" - Spring 2006
//
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "config_audio.h"
#include "dcast.h"

#ifdef HAVE_FMODEX //[

//Panda headers.
#include "config_audio.h"
#include "config_util.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "fmodAudioDSP.h"
//Needed so People use Panda's Generic UNIX Style Paths for Filename.
#include "filename.h"
#include "virtualFileSystem.h"

//FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>



TypeHandle FmodAudioManager::_type_handle;

pset<FmodAudioManager *> FmodAudioManager::_all_managers;

////////////////////////////////////////////////////////////////////
// Central dispatcher for audio errors.
////////////////////////////////////////////////////////////////////

static void fmod_audio_errcheck(FMOD_RESULT result) {
  if (result != 0) {
    audio_error("FMOD Error: "<< FMOD_ErrorString(result) );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Create_AudioManager
//       Access: Private
//  Description: Factory Function
////////////////////////////////////////////////////////////////////
PT(AudioManager) Create_AudioManager() {
  return new FmodAudioManager;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::FmodAudioManager()
//       Access: Public
//  Description: Constructor
////////////////////////////////////////////////////////////////////
FmodAudioManager::
FmodAudioManager() {
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
    
  result = FMOD::System_Create(&_system);
  fmod_audio_errcheck(result);

  //  Let check the Version of FMOD to make sure the Headers and Libraries are correct.
  result = _system->getVersion(&version);
  fmod_audio_errcheck(result);
  
  if (version < FMOD_VERSION){
    audio_error("You are using an old version of FMOD.  This program requires:" << FMOD_VERSION);
  }

  //Stick Surround Sound 5.1 thing Here.
  if (fmod_use_surround_sound) {
    audio_debug("Setting FMOD to use 5.1 Surround Sound.");
    result = _system->setSpeakerMode( FMOD_SPEAKERMODE_5POINT1 );
    fmod_audio_errcheck(result);
  }

  //Now we Initialize the System.
  result = _system->init(fmod_number_of_sound_channels, FMOD_INIT_NORMAL, 0);
  fmod_audio_errcheck(result);

  if (result == FMOD_OK){
    _is_valid = true;
  } else {
    _is_valid = false;
  }

  //  This sets the distance factor for 3D audio to use feet. 
  //  FMOD uses meters by default.
  //  Since Panda use feet we need to compensate for that with a factor of 3.28
  //
  //  This can be over written.  You just need to call
  //  audio_3d_set_distance_factor(float factor) and set you new factor.
  
  _doppler_factor = 1;
  _distance_factor = 3.28;
  _drop_off_factor = 1;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck( result );
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::~FmodAudioManager
//       Access: Public
//  Description: DESTRCUTOR !!!
////////////////////////////////////////////////////////////////////
FmodAudioManager::
~FmodAudioManager() {
  // Be sure to delete associated sounds before deleting the manager!
  FMOD_RESULT result;

  //Release DSPs First
  _system_dsp.clear();

  //Release Sounds Next
  _all_sounds.clear();

  // Remove me from the managers list.
  _all_managers.erase(this);

    result = _system->release();
  fmod_audio_errcheck(result);
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
//     Function: FmodAudioManager::get_sound()
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(const string &file_name, bool positional) {
  //Needed so People use Panda's Generic UNIX Style Paths for Filename.
  //path.to_os_specific() converts it back to the proper OS version later on.
  
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_sound_path());

  // Build a new AudioSound from the audio data.
  PT(AudioSound) audioSound = 0;
  PT(FmodAudioSound) fmodAudioSound = new FmodAudioSound(this, path, positional );

  _all_sounds.insert(fmodAudioSound);

  audioSound = fmodAudioSound;

  return audioSound;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::create_dsp()
//       Access: Published
//  Description: This is what creates a DSP instance.
////////////////////////////////////////////////////////////////////
PT(AudioDSP) FmodAudioManager::
create_dsp(DSP_category index) {
  // Build a new AudioSound from the audio data.
  PT(FmodAudioDSP) fmodAudioDSP = new FmodAudioDSP(this, index);

  return (AudioDSP*)fmodAudioDSP;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::add_dsp()
//       Access: Published
//  Description: This stick the DSP in the Global Effect Chain
//        DSP effects here, affect all the SOUNDS being played
//        in panda.
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
add_dsp( PT(AudioDSP) x) {
  FMOD_RESULT result;

  FmodAudioDSP *fdsp;

  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {
    return false;
  } else {
    result = _system->addDSP( fdsp->_dsp );
    fmod_audio_errcheck( result );
    _system_dsp.insert(fdsp);
    fdsp->set_in_chain(true);
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::remove_dsp()
//       Access: Published
//  Description: This removes the DSP from the Global Effect chain but does not destroy it.
//               Just remember a "Removed"  DSP is still availible for use 
//         in another chain Global or a specific sound.
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
remove_dsp(PT(AudioDSP) x) {
  FMOD_RESULT result;

  FmodAudioDSP *fdsp;
  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {
    result = fdsp->_dsp->remove();
    fmod_audio_errcheck( result );

    _system_dsp.erase(fdsp);

    fdsp->set_in_chain(false);

    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::getSpeakerSetup()
//       Access: Published
//  Description: This is to query if you are using a MultiChannel Setup.
////////////////////////////////////////////////////////////////////
int FmodAudioManager::
getSpeakerSetup() {
  FMOD_RESULT result;
  FMOD_SPEAKERMODE speakerMode;
  int returnMode;

  result = _system->getSpeakerMode( &speakerMode );
  fmod_audio_errcheck( result );

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
    case  FMOD_SPEAKERMODE_PROLOGIC:
      returnMode = 7;
      break;
    case  FMOD_SPEAKERMODE_MAX:
      returnMode = 8;
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
  FMOD_RESULT result;
  FMOD_SPEAKERMODE speakerModeType = (FMOD_SPEAKERMODE)cat;
  result = _system->setSpeakerMode( speakerModeType);
  fmod_audio_errcheck(result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_volume(float volume)
//       Access: Public
//  Description: 
//        There isn't a specific system volume function in FMOD-EX,
//        so this function is moot now.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::set_volume(float volume) {
  audio_warning("FmodAudioManager::set_volume has no effect." );
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_volume()
//       Access: Public
//  Description: 
//        There isn't a specific system volume function in FMOD-EX,
//        so this function is moot now.
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
get_volume() const {
  audio_warning("FmodAudioManager::get_volume has no effect." );
  return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_active(bool active)
//       Access: Public
//  Description: Turn on/off
//               Warning: not implemented.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_active(bool active) {
  _active = active;
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
  for (SoundSet::iterator i = _all_sounds.begin(); i != _all_sounds.end(); ++i) {
    (*i)->stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::update
//       Access: Public
//  Description: Perform all per-frame update functions.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
update() {
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
audio_3d_set_listener_attributes(float px, float py, float pz, float vx, float vy, float vz, float fx, float fy, float fz, float ux, float uy, float uz) {
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
  fmod_audio_errcheck( result );

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: Get position of the "ear" that picks up 3d sounds
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_get_listener_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz, float *fx, float *fy, float *fz, float *ux, float *uy, float *uz) {
  audio_error("audio3dGetListenerAttributes: currently unimplemented. Get the attributes of the attached object");

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: Set units per meter (Fmod uses meters internally for
//               its sound-spacialization calculations)
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_set_distance_factor(float factor) {
  audio_debug( "FmodAudioManager::audio_3d_set_distance_factor( factor= " << factor << ")" );
  
  FMOD_RESULT result;

  _distance_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck( result );


}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: Gets units per meter (Fmod uses meters internally for
//               its sound-spacialization calculations)
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
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
audio_3d_set_doppler_factor(float factor) {
  audio_debug("FmodAudioManager::audio_3d_set_doppler_factor(factor="<<factor<<")");

  FMOD_RESULT result;

  _doppler_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck( result );

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
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
audio_3d_set_drop_off_factor(float factor) {
  audio_debug("FmodAudioManager::audio_3d_set_drop_off_factor("<<factor<<")");

  FMOD_RESULT result;

  _drop_off_factor = factor;

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  fmod_audio_errcheck( result );

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
audio_3d_get_drop_off_factor() const {
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



#endif //]
