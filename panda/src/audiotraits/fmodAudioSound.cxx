// Filename: fmodAudioSound.cxx
// Created by:  cort (January 22, 2003)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#ifdef HAVE_FMOD //[

#include "fmodAudioSound.h"
#include "fmodAudioManager.h"

#include <cmath>

#ifndef NDEBUG //[
  #define fmod_audio_debug(x) \
      audio_debug("FmodAudioSound "<<" \""<<get_name()<<"\" "<< x )
#else //][
  #define fmod_audio_debug(x) ((void)0)
#endif //]

FmodAudioSound::
FmodAudioSound(FmodAudioManager* manager, FSOUND_STREAM *audio_data,
	       string file_name, float length)
  : _manager(manager), _audio(audio_data), _file_name(file_name),
    _volume(1.0f), _balance(0), _loop_count(1), _length(length),
    _active(true), _paused(false), _channel(-1) {
  nassertv(!file_name.empty());
  nassertv(audio_data != NULL);
  audio_debug("FmodAudioSound(manager=0x"<<(void*)&manager
	      <<", file_name="<<file_name<<")");
}

FmodAudioSound::
~FmodAudioSound() {
  fmod_audio_debug("~FmodAudioSound()");
  this->stop();
  _manager->release_sound(this);
}
  
void FmodAudioSound::
play() {
  if (!_active) {
    return;
  }

  if(_manager->_bExclusive) {
    // stop any other sound that parent mgr is playing
    _manager->stop_all_sounds();
  }

  // If the sound is already playing, stop it.
  if (this->status() == AudioSound::PLAYING) {
    this->stop();
  }

  // Play the stream, but start it paused so we can set the volume and
  // panning first.
  bool bStatusOK = false;
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
  
  // Set looping -- unimplemented
  
  // Unpause and set status to playing
  FSOUND_SetPaused(_channel, 0);

  // Delay until the channel is actually playing.  This shouldn't
  // result in a noticeable delay, and allows you to immediately test
  // the status of the sound after initiating playback.
  while (!FSOUND_IsPlaying(_channel));
}

void FmodAudioSound::stop() {
  FSOUND_Stream_Stop(_audio);
  _channel = -1;
}

void FmodAudioSound::set_loop(bool loop) {
  audio_error("FmodAudioSound::set_loop() -- not yet implemented");
  fmod_audio_debug("set_loop() set to "<<loop);
  _loop_count = loop ? 0 : 1;
}

bool FmodAudioSound::get_loop() const {
  fmod_audio_debug("get_loop() returning "<<(_loop_count==0));
  return (_loop_count == 0);
}
  
void FmodAudioSound::set_loop_count(unsigned long loop_count) {
  audio_error("FmodAudioSound::set_loop_count() -- not yet implemented");
  fmod_audio_debug("set_loop_count() set to "<<loop_count);
  _loop_count = loop_count;
}

unsigned long FmodAudioSound::get_loop_count() const {
  fmod_audio_debug("get_loop_count() returning "<<_loop_count);
  return _loop_count;
}
  
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
  FSOUND_Stream_SetTime(_audio, start_time * 1000.0f);
}

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

float FmodAudioSound::get_volume() const {
  fmod_audio_debug("get_volume() returning "<<_volume);
  return _volume;
}

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

float FmodAudioSound::get_balance() const {
  fmod_audio_debug("get_balance() returning "<<_balance);
  return _balance;
}

void FmodAudioSound::set_active(bool active) {
  fmod_audio_debug("set_active(active="<<active<<")");
  if (!active) {
    // Once looping works, a looping sound should be paused, not
    // stopped.  When the sound is activated again, it is unpaused.
    this->stop();
  }
  _active = active;
}

bool FmodAudioSound::get_active() const {
  fmod_audio_debug("get_active() returning "<<_active);
  return _active;
}

const string& FmodAudioSound::get_name() const {
  //fmod_audio_debug("get_name() returning "<<_file_name);
  return _file_name;
}

float FmodAudioSound::length() const {
  return _length;
}

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
