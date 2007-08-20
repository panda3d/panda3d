// Filename: openalAudioSound.cxx
// Created by:  Ben Buchwald <bb2@alumni.cmu.edu>
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

#ifdef HAVE_OPENAL //[

//Panda Headers
#include "throw_event.h"
#include "openalAudioSound.h"
#include "openalAudioManager.h"

TypeHandle OpenALAudioSound::_type_handle;


#ifndef NDEBUG //[
  #define openal_audio_debug(x) \
      audio_debug("OpenALAudioSound \""<<get_name() \
      <<"\" "<< x )
#else //][
#define openal_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::OpenALAudioSound
//       Access: public
//  Description: Constructor
//               All sound will DEFAULT load as a 2D sound unless
//               otherwise specified.
////////////////////////////////////////////////////////////////////

OpenALAudioSound::
OpenALAudioSound(OpenALAudioManager* manager,
    OpenALAudioManager::SoundData *sd, string file_name, bool positional)
    : _sd(sd), _source(0), _manager(manager), _file_name(file_name),
    _volume(1.0f), _balance(0), _play_rate(1.0),
    _loop_count(1), _pause_time(0.0),
    _active(true), _paused(false) {
  nassertv(sd != NULL);
  nassertv(!file_name.empty());
  audio_debug("OpenALAudioSound(manager=0x"<<(void*)&manager
      <<", sd=0x"<<(void*)sd<<", file_name="<<file_name<<")");

  //Inits 3D Attributes
  _location[0] = 0;
  _location[1] = 0;
  _location[2] = 0;

  _velocity[0] = 0;
  _velocity[1] = 0;
  _velocity[2] = 0;

  _min_dist = 3.28f; _max_dist = 1000000000.0f;
  _drop_off_factor = 1.0f;

  _buffer = _sd->_buffer;
  
  // don't assign source until we play since sources are limited
  /*
  // Create a source to play the buffer
  alGetError(); // clear errors
  alGenSources(1,&_source);
  al_audio_errcheck("alGenSources()");

  // Assign the buffer to the source
  alSourcei(_source,AL_BUFFER,_buffer);
  al_audio_errcheck("alSourcei(_source,AL_BUFFER,_buffer)");
  */

  // nonpositional sources are made relative to the listener so they don't move
  _positional = positional;
  if (_positional) {
    int nChannels=1;
    alGetBufferi(_buffer,AL_CHANNELS,&nChannels);
    al_audio_errcheck("alGetBufferi(_buffer,AL_CHANNELS)");
    if (nChannels>1)
      audio_warning("OpenALAudioSound: Stereo sounds won't be spacialized: "<<file_name);
  } else {
    /*alSourcei(_source,AL_SOURCE_RELATIVE,AL_TRUE);
    al_audio_errcheck("alSourcei(_source,AL_SOURCE_RELATIVE,AL_TRUE)");*/
  }

  /*
  // set initial values since they are manager-relative
  set_volume(_volume);
  //set_balance(_balance);
  set_play_rate(_play_rate);
  set_3d_min_distance(_min_dist);
  set_3d_max_distance(_max_dist);
  set_3d_drop_off_factor(_drop_off_factor);
  */
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::~OpenALAudioSound
//       Access: public
//  Description: DESTRUCTOR!!!
////////////////////////////////////////////////////////////////////
OpenALAudioSound::
~OpenALAudioSound() {
  openal_audio_debug("~OpenALAudioSound()");
  cleanup();
  _manager->release_sound(this);
  openal_audio_debug("~OpenALAudioSound() done");
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound:: play
//       Access: public
//  Description: Plays a sound.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
play() {
  float px,py,pz,vx,vy,vz;
      
  openal_audio_debug("play()");
  if (_active) {
    if (status() == AudioSound::PLAYING) {
      stop();
    }
    //nassertv(_source);
    _manager->starting_sound(this);

    if (_source) {
      // Setup source
      _manager->make_current();

      alGetError(); // clear errors
      
      // Assign the buffer to the source
      alSourcei(_source,AL_BUFFER,_buffer);
      ALenum result = alGetError();
      if (result!=AL_NO_ERROR) {
        audio_error("alSourcei(_source,AL_BUFFER,_buffer): " << alGetString(result) );
        stop();
        return;
      }

      // nonpositional sources are made relative to the listener so they don't move
      alSourcei(_source,AL_SOURCE_RELATIVE,_positional?AL_FALSE:AL_TRUE);
      al_audio_errcheck("alSourcei(_source,AL_SOURCE_RELATIVE)");

      // set source properties that we have stored
      set_volume(_volume);
      //set_balance(_balance);
      set_play_rate(_play_rate);
      set_3d_min_distance(_min_dist);
      set_3d_max_distance(_max_dist);
      set_3d_drop_off_factor(_drop_off_factor);
      
      get_3d_attributes(&px,&py,&pz,&vx,&vy,&vz);
      set_3d_attributes(px, py, pz, vx, vy, vz);

      set_loop_count(_loop_count);

      if (_pause_time) {
        set_time(_pause_time);
        _pause_time = 0.0;
      }

      // Start playing:
      alSourcePlay(_source);
      al_audio_errcheck("alSourcePlay(_source)");

      audio_debug("  started sound " << _file_name );
    }
  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused "<<_file_name );
    _paused=true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::stop
//       Access: public
//  Description: Stop a sound
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
stop() {
  openal_audio_debug("stop()");
  //nassertv(_source);
  
  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourceStop(_source);
    al_audio_errcheck("alSourceStop(_source)");
  }

  _pause_time = 0.0;

  _manager->stopping_sound(this);
  // The _paused flag should not be cleared here.  _paused is not like
  // the Pause button on a cd/dvd player.  It is used as a flag to say
  // that it was looping when it was set inactive.  There is no need to
  // make this symmetrical with play().  set_active() is the 'owner' of
  // _paused.  play() accesses _paused to help in the situation where
  // someone calls play on an inactive sound().
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::finished
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
finished() {
  openal_audio_debug("finished()");
  _manager->stopping_sound(this);
  if (!_finished_event.empty()) {
    throw_event(_finished_event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_loop
//       Access: public
//  Description: Turns looping on and off
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_loop(bool loop) {
  openal_audio_debug("set_loop(loop="<<loop<<")");
  // loop count of 0 means always loop
  set_loop_count((loop)?0:1);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_loop
//       Access: public
//  Description: Returns whether looping is on or off
////////////////////////////////////////////////////////////////////
bool OpenALAudioSound::
get_loop() const {
  openal_audio_debug("get_loop() returning "<<(_loop_count==0));
  return (_loop_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_loop_count
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_loop_count(unsigned long loop_count) {
  openal_audio_debug("set_loop_count(loop_count="<<loop_count<<")");

  if (loop_count>1) {
    audio_error("OpenALAudioSound::set_loop_count() doesn't support looping a finite number of times, 0 (infinite) or 1 only");
    loop_count = 1;
  }

  if (_loop_count!=loop_count) {
    _loop_count=loop_count;
  }

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcei(_source,AL_LOOPING,_loop_count==0?AL_TRUE:AL_FALSE);
    al_audio_errcheck("alSourcei(_source,AL_LOOPING)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_loop_count
//       Access: public
//  Description: Return how many times a sound will loop.
////////////////////////////////////////////////////////////////////
unsigned long OpenALAudioSound::
get_loop_count() const {
  openal_audio_debug("get_loop_count() returning "<<_loop_count);
  return _loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_time
//       Access: public
//  Description: Sets the play position within the sound
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_time(float time) {
  openal_audio_debug("set_time(time="<<time<<")");

  //nassertv(_source);
  
  // Ensure we don't inadvertently run off the end of the sound.
  float max_time = length();
  if (time > max_time) {
    openalAudio_cat.warning()
      << "set_time(" << time << ") requested for sound of length " 
      << max_time << "\n";
    time = max_time;
  }

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_SEC_OFFSET,time);
    al_audio_errcheck("alSourcef(_source,AL_SEC_OFFSET)");
  }

  if (status()==PLAYING) {
    _pause_time = 0.0;
  } else {
    _pause_time = time;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_time
//       Access: public
//  Description: Gets the play position within the sound
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_time() const {
  float time;

  //nassertv(_source);

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alGetSourcef(_source,AL_SEC_OFFSET,&time);
    al_audio_errcheck("alGetSourcef(_source,AL_SEC_OFFSET)");
  } else {
    time = _pause_time;
  }
  openal_audio_debug("get_time() returning "<<time);
  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_volume(float vol)
//       Access: public
//  Description: 0.0 to 1.0 scale of volume converted to Fmod's
//               internal 0.0 to 255.0 scale.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_volume(float volume) {
  openal_audio_debug("set_volume(volume="<<volume<<")");

  //nassertv(_source);
  
  _volume=volume;

  if (_source) {
    volume*=_manager->get_volume();
    
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_GAIN,volume);
    al_audio_errcheck("alSourcef(_source,AL_GAIN)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_volume
//       Access: public
//  Description: Gets the current volume of a sound.  1 is Max. O is Min.
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_volume() const {
  openal_audio_debug("get_volume() returning "<<_volume);
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_balance(float bal)
//       Access: public
//  Description: -1.0 to 1.0 scale
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_balance(float balance_right) {
  audio_debug("OpenALAudioSound::set_balance() not implemented");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_balance
//       Access: public
//  Description: -1.0 to 1.0 scale 
//        -1 should be all the way left.
//        1 is all the way to the right.
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_balance() const {
  audio_debug("OpenALAudioSound::get_balance() not implemented");
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_play_rate(float rate)
//       Access: public
//  Description: Sets the speed at which a sound plays back.
//        The rate is a multiple of the sound, normal playback speed.
//        IE 2 would play back 2 times fast, 3 would play 3 times, and so on.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_play_rate(float play_rate) {
  openal_audio_debug("set_play_rate(play_rate="<<play_rate<<")");

  //nassertv(_source);
  
  _play_rate=play_rate;

  if (_source) {
    play_rate*=_manager->get_play_rate();
    
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_PITCH,play_rate);
    al_audio_errcheck("alSourcef(_source,AL_PITCH)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_play_rate
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_play_rate() const {
  openal_audio_debug("get_play_rate() returning "<<_play_rate);
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::length
//       Access: public
//  Description: Get length
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
length() const {
  return _sd->get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound
//        NOW LISTEN UP!!! THIS IS IMPORTANT!
//        Both Panda3D and OpenAL use a right handed coordinate system.
//        But there is a major difference!
//        In Panda3D the Y-Axis is going into the Screen and the Z-Axis is going up.
//        In OpenAL the Y-Axis is going up and the Z-Axis is coming out of the screen.
//        The solution is simple, we just flip the Y and Z axis and negate the Z, as we move coordinates
//        from Panda to OpenAL and back.
//        What does did mean to average Panda user?  Nothing, they shouldn't notice anyway.
//        But if you decide to do any 3D audio work in here you have to keep it in mind.
//        I told you, so you can't say I didn't.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
  _location[0] = px;
  _location[1] = pz;
  _location[2] = -py; 

  _velocity[0] = vx;
  _velocity[1] = vz;
  _velocity[2] = -vy;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcefv(_source,AL_POSITION,_location);
    al_audio_errcheck("alSourcefv(_source,AL_POSITION)");
    alSourcefv(_source,AL_VELOCITY,_velocity);
    al_audio_errcheck("alSourcefv(_source,AL_VELOCITY)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound
//         Currently unimplemented. Get the attributes of the attached object.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
  *px = _location[0];
  *py = -_location[2];
  *pz = _location[1];

  *vx = _velocity[0];
  *vy = -_velocity[2];
  *vz = _velocity[1];
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_min_distance
//       Access: public
//  Description: Set the distance that this sound begins to fall off. Also
//               affects the rate it falls off.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_min_distance(float dist) {
  _min_dist = dist;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_REFERENCE_DISTANCE,_min_dist*_manager->audio_3d_get_distance_factor());
    al_audio_errcheck("alSourcefv(_source,AL_REFERENCE_DISTANCE)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_min_distance
//       Access: public
//  Description: Get the distance that this sound begins to fall off
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_3d_min_distance() const {
  return _min_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_max_distance
//       Access: public
//  Description: Set the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_max_distance(float dist) {
  _max_dist = dist;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_MAX_DISTANCE,_max_dist*_manager->audio_3d_get_distance_factor());
    al_audio_errcheck("alSourcefv(_source,AL_MAX_DISTANCE)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_max_distance
//       Access: public
//  Description: Get the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_drop_off_factor
//       Access: public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_drop_off_factor(float factor) {
  _drop_off_factor = factor;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_ROLLOFF_FACTOR,_drop_off_factor*_manager->audio_3d_get_drop_off_factor());
    al_audio_errcheck("alSourcefv(_source,AL_ROLLOFF_FACTOR)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_drop_off_factor
//       Access: public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
float OpenALAudioSound::
get_3d_drop_off_factor() const {
  return _drop_off_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_active
//       Access: public
//  Description: Sets whether the sound is marked "active".  By
//               default, the active flag true for all sounds.  If the
//               active flag is set to false for any particular sound,
//               the sound will not be heard.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_active(bool active) {
  openal_audio_debug("set_active(active="<<active<<")");
  if (_active!=active) {
    _active=active;
    if (_active) {
      // ...activate the sound.
      if (_paused
          &&
          _loop_count==0) {
        // ...this sound was looping when it was paused.
        _paused=false;
        play();
      }
    } else {
      // ...deactivate the sound.
      if (status()==PLAYING) {
        if (_loop_count==0) {
          // ...we're pausing a looping sound.
          _paused=true;
        }
        stop();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_active 
//       Access: public
//  Description: Returns whether the sound has been marked "active".
////////////////////////////////////////////////////////////////////
bool OpenALAudioSound::
get_active() const {
  openal_audio_debug("get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_finished_event
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_finished_event(const string& event) {
  openal_audio_debug("set_finished_event(event="<<event<<")");
  _finished_event = event;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_finished_event
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
const string& OpenALAudioSound::
get_finished_event() const {
  openal_audio_debug("get_finished_event() returning "<<_finished_event);
  return _finished_event;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_name
//       Access: public
//  Description: Get name of sound file
////////////////////////////////////////////////////////////////////
const string& OpenALAudioSound::
get_name() const {
  return _file_name;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::status
//       Access: public
//  Description: Get status of the sound.
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus OpenALAudioSound::
status() const {
  ALenum status;

  if (_source==0) {
    //return AudioSound::BAD;
    return AudioSound::READY;
  }

  _manager->make_current();

  alGetError(); // clear errors
  alGetSourcei(_source,AL_SOURCE_STATE,&status);
  al_audio_errcheck("alGetSourcei(_source,AL_SOURCE_STATE)");

  if (status == AL_PLAYING/* || status == AL_PAUSED*/) {
    return AudioSound::PLAYING;
  } else {
    return AudioSound::READY;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::cleanup
//       Access: Private
//  Description: Called to release any resources associated with the
//               sound.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
cleanup() {
  if (_source) {
    stop();
    /*
    if (OpenALAudioManager::_openal_active) {
      // delete the source
      _manager->make_current();
      alGetError(); // clear errors
      alDeleteSources(1,&_source);
      al_audio_errcheck("alDeleteSources()");
    }
    _source = 0;
    */
  }
}

#endif //]
