// Filename: fmodAudioSound.cxx
// Created by:  cort (January 22, 2003)
// Extended by: ben  (October 22, 2003)
// Prior system by: cary
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
#ifdef HAVE_FMOD //[

#include "throw_event.h"
#include "fmodAudioSound.h"
#include "fmodAudioManager.h"

#include <cmath>

#ifndef NDEBUG //[
  #define fmod_audio_debug(x) \
      audio_debug("FmodAudioSound "<<" \""<<get_name()<<"\" "<< x )
#else //][
  #define fmod_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: pandaFmodFinishedCallback_Stream
//       Access: file scope
//  Description: What happens when a sound ends (not reaches the end
//               of a loop, but really ends).
////////////////////////////////////////////////////////////////////
signed char
pandaFmodFinishedCallback_Stream( FSOUND_STREAM *audio, void *buff, int len, void *p_sound ) {
    FmodAudioSound* sound = (FmodAudioSound*)p_sound;
    assert(sound); //sanity test
    sound->finished();
    return true; //make signed char happy
}

////////////////////////////////////////////////////////////////////
//     Function: panda_Fmod_finished_callback
//       Access: file scope
//  Description: Sets up a finish callback for a sound. 
////////////////////////////////////////////////////////////////////
void 
panda_Fmod_finished_callback( FSOUND_STREAM *audio, FmodAudioSound* sound ) {
    if ( !audio || !sound ) {//sanity test
        return;
    }
    audio_debug("panda_Fmod_finished_callback(audio="<<((void*)audio)
        <<", sound="<<((void*)sound)<<")");
    FSOUND_STREAMCALLBACK callback = pandaFmodFinishedCallback_Stream;
                                //actual stream, callback func, pointer to FmodAudioSound
    FSOUND_Stream_SetEndCallback( audio, callback, sound );
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::FmodAudioSound
//       Access: 
//  Description: constructor
////////////////////////////////////////////////////////////////////
FmodAudioSound::
FmodAudioSound(FmodAudioManager* manager, FSOUND_STREAM *audio_data,
         string file_name, float length)
  : _manager(manager), _audio(audio_data), _file_name(file_name),
    _volume(1.0f), _balance(0), _loop_count(1), _length(length),
    _active(true), _paused(false), _bExclusive(false),_channel(-1) {
  _pos[0] = 0.0f; _pos[1] = 0.0f; _pos[2] = 0.0f;
  _vel[0] = 0.0f; _vel[1] = 0.0f; _vel[2] = 0.0f;
  nassertv(!file_name.empty());
  nassertv(audio_data != NULL);
  fmod_audio_debug("FmodAudioSound(manager="<<(void*)&manager
        <<", file_name="<<file_name<<")");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::~FmodAudioSound
//       Access: public
//  Description: destructor
////////////////////////////////////////////////////////////////////
FmodAudioSound::
~FmodAudioSound() {
  fmod_audio_debug("~FmodAudioSound()");
  this->stop();
  _manager->release_sound(this);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound:: play
//       Access: public
//  Description: Play a sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
play() {
  if (!_active) {
    return;
  }
  // If the sound is already playing, stop it.
  if (this->status() == AudioSound::PLAYING) {
    this->stop();
  }
if (_bExclusive) {
    // stop another sound that parent mgr is playing
    _manager->stop_all_sounds();
  }

  panda_Fmod_finished_callback( _audio, this );
  // Play the stream, but start it paused so we can set the volume and
  // panning first.
  assert(_audio != NULL);
  _channel = FSOUND_Stream_PlayEx(FSOUND_FREE, _audio, NULL, 1);
  if (_channel == -1) {
    fmod_audio_debug("play() failed");
    return;
  }
  
  // Set volume.
  unsigned char new_volume = (unsigned char)(_volume*255.0f);
  FSOUND_SetVolume(_channel, new_volume);
  
  // Set panning.
  unsigned char new_balance = (unsigned char)( (_balance+1.0f)*0.5f*255.0f);
  FSOUND_SetPan(_channel, new_balance);

  // Set 3d attributes, if needed
  if (FSOUND_Stream_GetMode(_audio) & FSOUND_HW3D) {
      // Convert from Panda coordinates to Fmod coordinates
      float fmod_pos [] = {_pos[0], _pos[2], _pos[1]};
      float fmod_vel [] = {_vel[0], _vel[2], _vel[1]};
      if(!FSOUND_3D_SetAttributes(_channel, fmod_pos, fmod_vel)) {
          audio_error("Unable to set 3d attributes for "<<_file_name<<"!");
      }
  }
  
  // Set looping -- unimplemented
  
  // Unpause and set status to playing
  FSOUND_SetPaused(_channel, 0);

  // Delay until the channel is actually playing.  This shouldn't
  // result in a noticeable delay, and allows you to immediately test
  // the status of the sound after initiating playback.
  while (!FSOUND_IsPlaying(_channel)) {
    // intentinaly empty loop.
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::stop
//       Access: public
//  Description: Stop a sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::stop() {
    if(!FSOUND_Stream_Stop(_audio)) {
        audio_error("Stop failed!");
    }
    _channel = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::finished
//       Access: public
//  Description: called by finishedCallback function when a sound
//               terminates (but doesn't loop).
////////////////////////////////////////////////////////////////////
void FmodAudioSound::finished() {
    fmod_audio_debug("finished()");
    stop();
    if (!_finished_event.empty()) {
        throw_event(_finished_event);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop
//       Access: public
//  Description: Turns looping on and off
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_loop(bool loop) {
  fmod_audio_debug("set_loop() set to "<<loop);
  unsigned int mode = FSOUND_Stream_GetMode(_audio);
  if (loop) {
      // turn looping on
      FSOUND_Stream_SetMode(_audio, mode | FSOUND_LOOP_NORMAL);
  } else {
      // turn looping off if and only if it is on
      if (FSOUND_LOOP_NORMAL == (mode & FSOUND_LOOP_NORMAL)) {
          FSOUND_Stream_SetMode(_audio, mode ^ FSOUND_LOOP_NORMAL);
      }
  }
  // default to loop infinitely
  _loop_count = loop ? 0 : 1;
  FSOUND_Stream_SetLoopCount(_audio, _loop_count - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop
//       Access: public
//  Description: Returns whether looping is on or off
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::get_loop() const {
  // 0 means loop forever,
  // >1 means loop that many times
  // So _loop_count != 1 means we're looping
  fmod_audio_debug("get_loop() returning "<<(_loop_count != 1));
  return (_loop_count != 1);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop_count
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_loop_count(unsigned long loop_count) {
  // Panda uses 0 to mean loop forever.
  // Fmod uses negative numbers to mean loop forever.
  // (0 means don't loop, 1 means play twice, etc.)
  // We must convert!
  
  fmod_audio_debug("set_loop_count() set to "<<loop_count);
  if (loop_count < 0) {
      fmod_audio_debug("Value out of bounds. Default to loop infinitely.");
      loop_count = 0;
  }
  _loop_count = loop_count;
  loop_count -= 1; 
  FSOUND_Stream_SetLoopCount(_audio, loop_count);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop_count
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned long FmodAudioSound::get_loop_count() const {
  fmod_audio_debug("get_loop_count() returning "<<_loop_count);
  return _loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_time
//       Access: public
//  Description: Sets the play position within the sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_time(float start_time) {
  if (start_time < 0.0f) {
    fmod_audio_debug("set_time(): param "<<start_time<<" out of range.");
    fmod_audio_debug("set_time(): clamping to zero.");
    start_time = 0.0f;
  } else if (start_time > _length) {
    fmod_audio_debug("set_time(): param "<<start_time<<" out of range.");
    fmod_audio_debug("set_time(): clamping to length ("<<_length<<".");
    start_time = _length - 0.01;
  }
  // FMOD measures time in milliseconds, so scale up by 1000.
  FSOUND_Stream_SetTime(_audio, (int)(start_time * 1000.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_time
//       Access: public
//  Description: Gets the play position within the sound
////////////////////////////////////////////////////////////////////
float FmodAudioSound::get_time() const {
  // A bug in stream WAV files causes FSOUND_Stream_GetTime() to
  // divide-by-zero somewhere if the stream isn't currently playing.
  // In this case, we should just return zero.
  if (!FSOUND_IsPlaying(_channel)) {
    return 0.0f;
  }

  // FMOD measures time in milliseconds, so scale down by 1000.
  float current_time = FSOUND_Stream_GetTime(_audio) * 0.001f;
  //fmod_audio_debug("get_time() returning "<<current_time);
  return current_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_volume
//       Access: public
//  Description: 0.0 to 1.0 scale of volume converted to Fmod's
//               internal 0.0 to 255.0 scale.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_volume(float vol) {
  if (vol < 0.0f) {
    fmod_audio_debug("set_volume(): param "<<vol<<" out of range.");
    fmod_audio_debug("set_volume(): clamping to zero.");
    vol = 0.0f;
  } else if (vol > 1.0f) {
    fmod_audio_debug("set_volume(): param "<<vol<<" out of range.");
    fmod_audio_debug("set_volume(): clamping to 1.0");
    vol = 1.0f;
  }
  _volume = vol;
  unsigned char new_volume = (unsigned char)(_volume*255.0f);
  FSOUND_SetVolume(_channel, new_volume);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_volume
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioSound::get_volume() const {
  fmod_audio_debug("get_volume() returning "<<_volume);
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_balance
//       Access: public
//  Description: -1.0 to 1.0 scale
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_balance(float bal) {
  if (bal < -1.0f) {
    fmod_audio_debug("set_balance(): param "<<bal<<" out of range.");
    fmod_audio_debug("set_balance(): clamping to -1.0.");
    bal = -1.0f;
  } else if (bal > 1.0f) {
    fmod_audio_debug("set_balance(): param "<<bal<<" out of range.");
    fmod_audio_debug("set_balance(): clamping to 1.0");
    bal = 1.0f;
  }
  _balance = bal;
  unsigned char new_balance = (unsigned char)( (_balance+1.0f)*0.5f*255.0f);
  FSOUND_SetPan(_channel, new_balance);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_balance
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioSound::get_balance() const {
  fmod_audio_debug("get_balance() returning "<<_balance);
  return _balance;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_active
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioSound::set_active(bool active) {
  fmod_audio_debug("set_active(active="<<active<<")");
  if (!active) {
    // Once looping works, a looping sound should be paused, not
    // stopped.  When the sound is activated again, it is unpaused.
    this->stop();
  }
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_active
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::get_active() const {
  fmod_audio_debug("get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_finished_event
//       Access: public
//  Description: Assign a string for the finished event to be referenced 
//               by in python by an accept method
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_finished_event(const string& event) {
  fmod_audio_debug("set_finished_event(event="<<event<<")");
  _finished_event = event;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_finished_event
//       Access: public
//  Description: Return the string the finished event is referenced by
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::
get_finished_event() const {
  fmod_audio_debug("get_finished_event() returning "<<_finished_event);
  return _finished_event;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_name
//       Access: public
//  Description: Get name of sound file
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::get_name() const {
  return _file_name;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::length
//       Access: public
//  Description: Get length
////////////////////////////////////////////////////////////////////
float FmodAudioSound::length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
    fmod_audio_debug("Set 3d position and velocity (px="<<px<<", py="<<py<<", pz="<<pz<<", vx="<<vx<<", vy="<<vy<<", vz="<<vz<<")");
    _pos[0] = px; _pos[1] = py; _pos[2] = pz;
    _vel[0] = vx; _vel[1] = vy; _vel[2] = vz;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
    audio_error("get3dAttributes: Currently unimplemented. Get the attributes of the attached object.");
    // NOTE: swap the +y with the +z axis to convert between FMOD
    //       coordinates and Panda3D coordinates
    //float temp;
    //temp = py;
    //py = pz;
    //pz = temp;

    //temp = vy;
    //vy = vz;
    //vz = temp;

    //float pos [] = {px, py, pz};
    //float vel [] = {vx, vy, vz};
    //FSOUND_3D_GetAttributes(_channel, pos, vel);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::status
//       Access: public
//  Description: Get status of the sound.
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus FmodAudioSound::status() const {
  // If the stream's channel isn't playing anything, then the stream
  // definitely isn't playing.
  if (!FSOUND_IsPlaying(_channel)) {
    return AudioSound::READY;
  }
  
  // If the channel is playing, see whether the current time is at the
  // end of the file.  If not, the stream is playing.
  float current_time = this->get_time();
  if (current_time >= _length - 0.01f) {
    // FMOD MIDI files don't stop automatically when they hit the end of the
    // file.  Their channel isn't released unless the stream is stopped
    // explicitly.
    FSOUND_Stream_Stop(_audio);
    return AudioSound::READY;
  } else {
    return AudioSound::PLAYING;
  }
}

#endif //]
