// Filename: fmodAudioSound.cxx
// Created by:  cort (January 22, 2003)
// Extended by: ben  (October 22, 2003)
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
#include "dcast.h"

#ifdef HAVE_FMODEX //[

//Panda Headers
#include "config_audio.h"
#include "fmodAudioSound.h"
#include "string_utils.h"

TypeHandle FmodAudioSound::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::FmodAudioSound
//       Access: public
//  Description: Constructor
//               All sound will DEFAULT load as a 2D sound unless
//               otherwise specified.
////////////////////////////////////////////////////////////////////

FmodAudioSound::
FmodAudioSound(AudioManager *manager, Filename file_name, bool positional) { 
  audio_debug("FmodAudioSound::FmodAudioSound() Creating new sound, filename: " << file_name  );

  _active = manager->get_active();

  //Local Variables that are needed.
  FMOD_RESULT result;

  //Inits 3D Attributes
  _location.x = 0;
  _location.y = 0;
  _location.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  //Play Rate Variable
  _playrate = 1;

  // These set the Speaker Levels to a default if you are using a MultiChannel Setup.
  for (int i=0; i<AudioManager::SPK_COUNT; i++) {
    _mix[i] = 1.0;
  }

  //Assign the values we need
  FmodAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  _channel = 0;
  _file_name = file_name;

  FMOD_CREATESOUNDEXINFO *sound_info = NULL;

  //Get the Speaker Mode [Important for later on.]
  result = _manager->_system->getSpeakerMode( &_speakermode );
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  // Calculate the approximate uncompressed size of the sound.
  int size =  file_name.get_file_size();
  string ext = downcase(file_name.get_extension());
  if (ext != "wav") size *= 10;
  
  int flag = positional ? FMOD_3D : FMOD_2D;
  int streamflag = (size > 250000) ? FMOD_CREATESTREAM : FMOD_CREATESAMPLE;
  if (ext == "mid") {
    streamflag = FMOD_CREATESTREAM;
    sound_info = &_manager->_midi_info;
    
    if (sound_info->dlsname != NULL) {
      audio_debug("Using DLS file " << sound_info->dlsname);
    }
  }

  result = _manager->_system->createSound( file_name.c_str(), FMOD_SOFTWARE | streamflag | flag , 
                                           sound_info, &_sound);
  if (result != FMOD_OK) {
    audio_error("createSound(" << file_name << "): " << FMOD_ErrorString(result));

    // We couldn't load the sound file.  Create a blank sound record
    // instead.
    char blank_data[100];
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(exinfo));
    memset(blank_data, 0, sizeof(blank_data));
    exinfo.cbsize = sizeof(exinfo);
    exinfo.length = sizeof(blank_data);
    exinfo.numchannels = 1;
    exinfo.defaultfrequency = 8000;
    exinfo.format = FMOD_SOUND_FORMAT_PCM16;
    result = _manager->_system->createSound( blank_data, FMOD_SOFTWARE | flag | FMOD_OPENMEMORY | FMOD_OPENRAW, &exinfo, &_sound);
    fmod_audio_errcheck("createSound (blank)", result);
  }

  // Some WAV files contain a loop bit.  This is not handled
  // consistently.  Override it.
  _sound->setLoopCount(1);
  _sound->setMode(FMOD_LOOP_OFF);
  
  //This is just to collect the defaults of the sound, so we don't
  //Have to query FMOD everytime for the info.
  //It is also important we get the '_sampleFrequency' variable here, for the
  //'set_play_rate()' and 'get_play_rate()' methods later;
  
  result = _sound->getDefaults( &_sampleFrequency, &_volume , &_balance, &_priority);
  fmod_audio_errcheck("_sound->getDefaults()", result);
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::~FmodAudioSound
//       Access: public
//  Description: DESTRUCTOR!!!
////////////////////////////////////////////////////////////////////
FmodAudioSound::
~FmodAudioSound() {
  FMOD_RESULT result;

  //Remove me from table of all sounds.
  _manager->_all_sounds.erase(this);

  //The Release Sound
  result = _sound->release();
  fmod_audio_errcheck("_sound->release()", result);
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound:: play
//       Access: public
//  Description: Plays a sound.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
play() {
  set_time(0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: sound_end_callback
//       Access: Static
//  Description: When fmod finishes playing a sound, decrements the
//               reference count of the associated FmodAudioSound.
////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK sound_end_callback(FMOD_CHANNEL *  channel, 
					  FMOD_CHANNEL_CALLBACKTYPE  type, 
					  void *commanddata1, 
					  void *commanddata2) {
  if (type == FMOD_CHANNEL_CALLBACKTYPE_END) {
    FMOD::Channel *fc = (FMOD::Channel *)channel;
    void *userdata = NULL;
    FMOD_RESULT result = fc->getUserData(&userdata);
    fmod_audio_errcheck("channel->getUserData()", result);
    FmodAudioSound *fsound = (FmodAudioSound*)userdata;
    fsound->_self_ref = fsound;
  }
  return FMOD_OK;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::stop
//       Access: public
//  Description: Stop a sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
stop() {
  FMOD_RESULT result;

  if (_channel != 0) {
    result =_channel->stop();
    if (result == FMOD_OK) {
      _self_ref.clear();
    }
    fmod_audio_errcheck("_channel->stop()", result);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop
//       Access: public
//  Description: Turns looping on and off
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_loop(bool loop) {
  if (loop) {
    set_loop_count(0);
  } else {
    set_loop_count(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop
//       Access: public
//  Description: Returns whether looping is on or off
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
get_loop() const {
  if (get_loop_count() == 1) {
    return false;
  } else {
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop_count
//       Access: public
//  Description: 
//        Panda uses 0 to mean loop forever.
//        Fmod uses negative numbers to mean loop forever.
//        (0 means don't loop, 1 means play twice, etc.
//        We must convert!
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_loop_count(unsigned long loop_count) {

  audio_debug("FmodAudioSound::set_loop_count()   Setting the sound's loop count to: " << loop_count);

  //LOCALS
  FMOD_RESULT result;

  if (loop_count == 0) {
    result = _sound->setLoopCount( -1 );
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);    
    fmod_audio_errcheck("_sound->setMode()", result);
  } else if (loop_count == 1) {
    result = _sound->setLoopCount( 1 );
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_OFF);    
    fmod_audio_errcheck("_sound->setMode()", result);
  } else {
    result = _sound->setLoopCount( loop_count );
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);
    fmod_audio_errcheck("_sound->setMode()", result);
  }

  audio_debug("FmodAudioSound::set_loop_count()   Sound's loop count should be set to: " << loop_count);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop_count
//       Access: public
//  Description: Return how many times a sound will loop.
////////////////////////////////////////////////////////////////////
unsigned long FmodAudioSound::
get_loop_count() const {
  FMOD_RESULT result;
  int loop_count;

  result = _sound->getLoopCount( &loop_count );
  fmod_audio_errcheck("_sound->getLoopCount()", result);

  if (loop_count <= 0) {
    return 0;
  } else {
    return (unsigned long)loop_count;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_time
//       Access: public
//  Description: Starts playing from the specified location.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_time(float start_time) {
  FMOD_RESULT result;

  if (!_active) {
    _paused = true;
    return;
  }
  
  int startTime = (int)(start_time * 1000);
  
  if (_channel != 0) {
    // try backing up current sound.
    result = _channel->setPosition( startTime , FMOD_TIMEUNIT_MS );
    if (result == FMOD_ERR_INVALID_HANDLE ||
        result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = 0;

    } else {
      fmod_audio_errcheck("_channel->setPosition()", result);

      bool playing;
      result = _channel->isPlaying(&playing);
      fmod_audio_errcheck("_channel->isPlaying()", result);
      if (result != FMOD_OK || !playing) {
        _channel = 0;
      }
    }
  }
  
  if (_channel == 0) {
    result = _manager->_system->playSound(FMOD_CHANNEL_FREE, _sound, true, &_channel);
    fmod_audio_errcheck("_system->playSound()", result);
    result = _channel->setUserData(this);
    fmod_audio_errcheck("_channel->setUserData()", result);
    result = _channel->setCallback(sound_end_callback);
    fmod_audio_errcheck("_channel->setCallback()", result);
    result = _channel->setPosition( startTime , FMOD_TIMEUNIT_MS );
    fmod_audio_errcheck("_channel->setPosition()", result);

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();
    // add_dsp_on_channel();
    set_3d_attributes_on_channel();

    result = _channel->setPaused(false);
    fmod_audio_errcheck("_channel->setPaused()", result);
    
    _self_ref = this;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_time
//       Access: public
//  Description: Gets the play position within the sound
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_time() const {
  FMOD_RESULT result;
  unsigned int current_time;

  if (_channel == 0) {
    return 0.0f;
  }

  result = _channel->getPosition( &current_time , FMOD_TIMEUNIT_MS );
  if (result == FMOD_ERR_INVALID_HANDLE) {
    return 0.0f;
  }
  fmod_audio_errcheck("_channel->getPosition()", result);
  
  return ((double)current_time) / 1000.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_volume(float vol)
//       Access: public
//  Description: 0.0 to 1.0 scale of volume converted to Fmod's
//               internal 0.0 to 255.0 scale.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_volume(float vol) {
  _volume = vol;
  set_volume_on_channel();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_volume
//       Access: public
//  Description: Gets the current volume of a sound.  1 is Max. O is Min.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_volume() const {
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_volume_on_channel()
//       Access: Private
//  Description: Set the volume on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_volume_on_channel() {
  FMOD_RESULT result;

  if (_channel != 0) {
    result = _channel->setVolume( _volume );
    if (result == FMOD_ERR_INVALID_HANDLE) {
      _channel = 0;
    } else {
      fmod_audio_errcheck("_channel->setVolume()", result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_balance(float bal)
//       Access: public
//  Description: -1.0 to 1.0 scale
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_balance(float bal) {
  _balance = bal;
  set_speaker_mix_or_balance_on_channel();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_balance
//       Access: public
//  Description: -1.0 to 1.0 scale 
//        -1 should be all the way left.
//        1 is all the way to the right.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_balance() const {
  return _balance;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_play_rate(float rate)
//       Access: public
//  Description: Sets the speed at which a sound plays back.
//        The rate is a multiple of the sound, normal playback speed.
//        IE 2 would play back 2 times fast, 3 would play 3 times, and so on.
//        This can also be set to a negative number so a sound plays backwards.
//        But rememeber if the sound is not playing, you must set the 
//        sound's time to its end to hear a song play backwards.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_play_rate(float rate) {
  _playrate = rate;
  set_play_rate_on_channel();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_play_rate
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_play_rate() const {
  return _playrate;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_play_rate_on_channel()
//       Access: public
//  Description: Set the play rate on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_play_rate_on_channel() {
  FMOD_RESULT result;
  float frequency = _sampleFrequency * _playrate;
  
  if (_channel != 0) {
    result = _channel->setFrequency( frequency );
    if (result == FMOD_ERR_INVALID_HANDLE) {
      _channel = 0;
    } else {
      fmod_audio_errcheck("_channel->setFrequency()", result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_name
//       Access: public
//  Description: Get name of sound file
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::
get_name() const {
  return _file_name;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::length
//       Access: public
//  Description: Get length
//        FMOD returns the time in MS  so we have to convert to seconds.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
length() const {
  FMOD_RESULT result;
  unsigned int length;

  result = _sound->getLength( &length, FMOD_TIMEUNIT_MS );
  fmod_audio_errcheck("_sound->getLength()", result);

  return ((double)length) / 1000.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound
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
void FmodAudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
  _location.x = px;
  _location.y = pz;
  _location.z = py;
  
  _velocity.x = vx;
  _velocity.y = vz;
  _velocity.z = vy;

  set_3d_attributes_on_channel();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_attributes_on_channel
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_attributes_on_channel() {
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);
  
  if ((_channel != 0) && (soundMode & FMOD_3D)) {
    result = _channel->set3DAttributes( &_location, &_velocity );
    if (result == FMOD_ERR_INVALID_HANDLE) {
      _channel = 0;
    } else {
      fmod_audio_errcheck("_channel->set3DAttributes()", result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound
//         Currently unimplemented. Get the attributes of the attached object.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
  audio_error("get3dAttributes: Currently unimplemented. Get the attributes of the attached object.");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_min_distance
//       Access: public
//  Description: Set the distance that this sound begins to fall off. Also
//               affects the rate it falls off.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_min_distance(float dist) {
  FMOD_RESULT result;

  _min_dist = dist;

  result = _sound->set3DMinMaxDistance( dist, _max_dist );
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_min_distance
//       Access: public
//  Description: Get the distance that this sound begins to fall off
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_3d_min_distance() const {
  return _min_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_max_distance
//       Access: public
//  Description: Set the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_max_distance(float dist) {
  FMOD_RESULT result;

  _max_dist = dist;

  result = _sound->set3DMinMaxDistance( _min_dist, dist );
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_max_distance
//       Access: public
//  Description: Get the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_speaker_mix
//       Access: Published
//  Description: In Multichannel Speaker systems [like Surround].
//
//        Speakers which don't exist in some systems will simply be ignored.
//        But I haven't been able to test this yet, so I am jsut letting you know.
//
//        BTW This will also work in Stereo speaker systems, but since
//        PANDA/FMOD has a balance [pan] function what is the point?
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_speaker_mix(AudioManager::SpeakerId speaker) {
  if (_channel == 0) {
    return 0.0;
  }

  FMOD_RESULT result;
  float frontleft;
  float frontright;
  float center;
  float sub;
  float backleft;
  float backright; 
  float sideleft;
  float sideright;

  result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
  fmod_audio_errcheck("_channel->getSpeakerMix()", result);

  switch(speaker) {
  case AudioManager::SPK_frontleft:  return frontleft;
  case AudioManager::SPK_frontright: return frontright;
  case AudioManager::SPK_center:     return center;
  case AudioManager::SPK_sub:        return sub;
  case AudioManager::SPK_backleft:   return backleft;
  case AudioManager::SPK_backright:  return backright;
  case AudioManager::SPK_sideleft:   return sideleft;
  case AudioManager::SPK_sideright:  return sideright;
  default: return 0.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_speaker_mix
//       Access: Published
//  Description: This sets the speaker mix for Surround Sound sytems.
//               It required 8 parameters which match up to the following:
//
//               * 1 = Front Left
//               * 2 = Front Right
//               * 3 = Center
//               * 4 = Subwoofer
//               * 5 = Back Left
//               * 6 = Back Right
//               * 7 = Side Left
//               * 8 = Side Right
//
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_speaker_mix(float frontleft, float frontright, float center, float sub, float backleft, float backright, float sideleft, float  sideright) {
  _mix[AudioManager::SPK_frontleft]  = frontleft;
  _mix[AudioManager::SPK_frontright] = frontright;
  _mix[AudioManager::SPK_center]     = center;
  _mix[AudioManager::SPK_sub]        = sub;
  _mix[AudioManager::SPK_backleft]   = backleft;
  _mix[AudioManager::SPK_backright]  = backright;
  _mix[AudioManager::SPK_sideleft]   = sideleft;
  _mix[AudioManager::SPK_sideright]  = sideright;

  set_speaker_mix_or_balance_on_channel();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_speaker_mix_or_balance_on_channel
//       Access: Private
//  Description: This is simply a safety catch.
//        If you are using a Stero speaker setup Panda will only pay attention
//        to 'set_balance()' command when setting speaker balances.
//        Other wise it will use 'set_speaker_mix'.
//        I put this in, because other wise you end up with a sitation,
//        where 'set_speaker_mix()' or 'set_balace()' will override any
//        previous speaker balance setups.  It all depends on which was called last.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_speaker_mix_or_balance_on_channel() {
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);

  if ((_channel != 0) && (( soundMode & FMOD_3D ) == 0)) {
    if ( _speakermode == FMOD_SPEAKERMODE_STEREO ) {
      result = _channel->setPan( _balance );
    } else {
      result = _channel->setSpeakerMix( _mix[AudioManager::SPK_frontleft],
                                        _mix[AudioManager::SPK_frontright],
                                        _mix[AudioManager::SPK_center],
                                        _mix[AudioManager::SPK_sub],
                                        _mix[AudioManager::SPK_backleft],
                                        _mix[AudioManager::SPK_backright],
                                        _mix[AudioManager::SPK_sideleft],
                                        _mix[AudioManager::SPK_sideright] 
                                        );
    }
    if (result == FMOD_ERR_INVALID_HANDLE) {
      _channel = 0;
    } else {
      fmod_audio_errcheck("_channel->setSpeakerMix()/setPan()", result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_priority
//       Access: Published
//  Description: Sets the priority of a sound.
//        This is what FMOD uses to determine is a sound will
//        play if all the other real channels have been used up.
////////////////////////////////////////////////////////////////////
int FmodAudioSound::
get_priority() {
  audio_debug("FmodAudioSound::get_priority()");
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_priority(int priority)
//       Access: Published
//  Description: Sets the Sound Priority [Whether is will be played
//        over other sound when real audio channels become short.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_priority(int priority) {
  // intentionally blank

  audio_debug("FmodAudioSound::set_priority()");

  FMOD_RESULT result;

  _priority = priority;

  result = _sound->setDefaults( _sampleFrequency, _volume , _balance, _priority);
  fmod_audio_errcheck("_sound->setDefaults()", result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::status
//       Access: public
//  Description: Get status of the sound.
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus FmodAudioSound::
status() const {
  FMOD_RESULT result;
  bool playingState;

  if ( _channel == 0 ) {
    return READY;
  }

  result = _channel->isPlaying( &playingState );
  if ((result == FMOD_OK) && (playingState == true)) {
    return PLAYING;
  } else {
    return READY;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_active
//       Access: public
//  Description: Sets whether the sound is marked "active".  By
//               default, the active flag true for all sounds.  If the
//               active flag is set to false for any particular sound,
//               the sound will not be heard.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_active(bool active) {
  if (_active != active) {
    _active = active;
    if (_active) {
      // ...activate the sound.
      if (_paused && get_loop_count()==0) {
        // ...this sound was looping when it was paused.
        _paused = false;
        play();
      }

    } else {
      // ...deactivate the sound.
      if (status() == PLAYING) {
        if (get_loop_count() == 0) {
          // ...we're pausing a looping sound.
          _paused = true;
        }
        stop();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_active 
//       Access: public
//  Description: Returns whether the sound has been marked "active".
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::finished
//       Access: public
//  Description: Not implemented.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
finished() {
  audio_error("finished: not implemented under FMOD-EX");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_finished_event
//       Access: public
//  Description: NOT USED ANYMORE!!!
//        Assign a string for the finished event to be referenced 
//              by in python by an accept method
//
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_finished_event(const string& event) {
  audio_error("set_finished_event: not implemented under FMOD-EX");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_finished_event
//       Access: public
//  Description:NOT USED ANYMORE!!!
//        Return the string the finished event is referenced by
//
//        
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::
get_finished_event() const {
  audio_error("get_finished_event: not implemented under FMOD-EX");
  return _finished_event;
}

#endif //]
