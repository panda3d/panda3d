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
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "fmodAudioDSP.h"
//Needed so People use Panda's Generic UNIX Style Paths for Filename.
#include "filename.h"


//FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>


////////////////////////////////////////////////////////////////////
//  This in needed for Panda's Pointer System
//  DO NOT ERASE!
////////////////////////////////////////////////////////////////////

TypeHandle FmodAudioManager::_type_handle;

////////////////////////////////////////////////////////////////////
//  END OF POINTER THING
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//  OK I am not sure if there is the best place for this one,  
//  but it seems to work.  IE, the FmodSound and FmodDSP classes can find it.
//  All FMOD API calls return a success or failure error.
//  [I am sure there is a name for this type of programming but I don't know it.]
//  Anyway, by adding the line "notify-level-audio debug" to the config.prc file 
//  of Panda, of the config.in file of MAKEPANDA, you can see the Debugs printed out at the
//  Python Prompt
/////////////////////////////////////////////////////////////////////
void ERRCHECK(FMOD_RESULT result){
  audio_debug("FMOD State: "<< result <<" "<< FMOD_ErrorString(result) );
}


PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() Fmod.");
  return new FmodAudioManager;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::FmodAudioManager()
//       Access: Public
//  Description: Constructor
////////////////////////////////////////////////////////////////////
FmodAudioManager::
FmodAudioManager() {

  //OK Lets create the FMOD Audio Manager.
  audio_debug("FmodAudioManager::FmodAudioManager()");

  FMOD_RESULT result;

  //We need a varible temporary to check the FMOD Version.
  unsigned int      version;

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
    


  audio_debug("FMOD::System_Create()");
  result = FMOD::System_Create(&_system);
  ERRCHECK(result);

  //  Let check the Version of FMOD to make sure the Headers and Libraries are correct.
  audio_debug("FMOD::System_Create()");
  result = _system->getVersion(&version);
  ERRCHECK(result);

  audio_debug("FMOD VERSION:" << hex << version );
  audio_debug("FMOD - Getting Version");

  if (version < FMOD_VERSION){
    audio_debug("Error!  You are using an old version of FMOD.  This program requires:" << FMOD_VERSION);
  }

  //Stick Surround Sound 5.1 thing Here.

  audio_debug("Checking for Surround Sound Flag.");

  if (fmod_use_surround_sound) {
    audio_debug("Setting FMOD to use 5.1 Surround Sound.");
    result = _system->setSpeakerMode( FMOD_SPEAKERMODE_5POINT1 );
    ERRCHECK(result);
  }

  //Now we Initialize the System.

  audio_debug("FMOD::System_Init");
  result = _system->init(fmod_number_of_sound_channels, FMOD_INIT_NORMAL, 0);
  ERRCHECK(result);

  if (result == FMOD_OK){
    audio_debug("FMOD Intialized OK, We are good to go Houston!");
    _is_valid = true;
  } else {
    audio_debug("Something is still wrong with FMOD!  Check source.");
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

  audio_debug("Setting 3D Audio settings: Doppler Factor, Distance Factor, Drop Off Factor");

  result = _system->set3DSettings( _doppler_factor, _distance_factor, _drop_off_factor);
  ERRCHECK( result );

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::~FmodAudioManager
//       Access: Public
//  Description: DESTRCUTOR !!!
////////////////////////////////////////////////////////////////////
FmodAudioManager::
~FmodAudioManager() {
  // Be sure to delete associated sounds before deleting the manager!
  audio_debug("~FmodAudioManager(): Closing Down");

  FMOD_RESULT result;

  //Release DSPs First
  _system_dsp.clear();

  //Release Sounds Next
  _all_sounds.clear();

  //result = _system->close();
  //ERRCHECK(result);

  result = _system->release();
  ERRCHECK(result);

  audio_debug("~FmodAudioManager(): System Down.");

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::is_valid
//       Access: Public
//  Description: This just check to make sure the FMOD System is 
//         up and running correctly.
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
is_valid() {
  audio_debug("FmodAudioManager::is_valid() = " << _is_valid );
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_sound()
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(const string &file_name, bool positional) {

  audio_debug("FmodAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  //Needed so People use Panda's Generic UNIX Style Paths for Filename.
  //path.to_os_specific() converts it back to the proper OS version later on.
  Filename path = file_name;

  // Build a new AudioSound from the audio data.
  PT(AudioSound) audioSound = 0;
  PT(FmodAudioSound) fmodAudioSound = new FmodAudioSound(this, path.to_os_specific(), positional );

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
  audio_debug("FmodAudioManager()::create_dsp");
  
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
  // intentionally blank

  FMOD_RESULT result;

  FmodAudioDSP *fdsp;

  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {

    audio_debug("FmodAudioManager()::add_dsp");
    audio_debug("This DSP has already been assigned to the system or a sound.");

    return false;

  } else
  {

    result = _system->addDSP( fdsp->_dsp );
    ERRCHECK( result );

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
  // intentionally blank

  FMOD_RESULT result;

  FmodAudioDSP *fdsp;
  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {

    result = fdsp->_dsp->remove();
    ERRCHECK( result );

    _system_dsp.erase(fdsp);

    fdsp->set_in_chain(false);

    return true;

  } else
  {

    audio_debug("FmodAudioManager()::remove_dsp()");
    audio_debug("This DSP doesn't exist in this chain.");

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
  // intentionally blank

  FMOD_RESULT result;
  FMOD_SPEAKERMODE speakerMode;
  int returnMode;

  result = _system->getSpeakerMode( &speakerMode );
  ERRCHECK( result );

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
setSpeakerSetup(AudioManager::SPEAKERMODE_category cat) {
  // intentionally blank

  audio_debug("FmodAudioSound::setSpeakerSetup() " );

  //Local Variables that are needed.
  FMOD_RESULT result;

  FMOD_SPEAKERMODE speakerModeType = (FMOD_SPEAKERMODE)cat;

  result = _system->setSpeakerMode( speakerModeType);
  ERRCHECK(result);

  audio_debug("Speaker Mode Set");

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_volume(float volume)
//       Access: Public
//  Description: 
//        There isn't a specific system volume function in FMOD-EX,
//        so this function is moot now.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::set_volume(float volume) {
  audio_debug("FmodAudioManager::set_volume()" );
  audio_debug("This function has no effect in this version." );
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
  audio_debug("FmodAudioManager::get_volume() returning ");
  audio_debug("This function has no effect in this version." );
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_active(bool active)
//       Access: Public
//  Description: Turn on/off
//         Again, this function is pretty much moot in this version now.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_active(bool active) {
  audio_debug("FmodAudioManager::set_active(flag="<<active<<")");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_active()
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
get_active() const {
  audio_debug("FmodAudioManager::get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::stop_all_sounds()
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
stop_all_sounds() {
  audio_debug("FmodAudioManager::stop_all_sounds()" );

  for (SoundSet::iterator i = _all_sounds.begin(); i != _all_sounds.end(); ++i) {
    (*i)->stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::audio_3d_update
//       Access: Public
//  Description: Commit position changes to listener and all
//               positioned sounds. Normally, you'd want to call this
//               once per iteration of your main loop.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
audio_3d_update() {
  audio_debug("FmodAudioManager::audio_3d_update()");
  audio_debug("Calling FMOD's update function");

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
  ERRCHECK( result );

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
  ERRCHECK( result );


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
  ERRCHECK( result );

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
  ERRCHECK( result );

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
