// Filename: milesAudioSound.cxx
// Created by:  skyler (June 6, 2001)
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
#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioManager.h"


MilesAudioSound::
MilesAudioSound(MilesAudioManager& manager,
    HAUDIO audio, string file_name)
    : _manager(manager), _file_name(file_name),
    _start_time(0), _volume(1.0), _balance(0),
    _loop_count(1),
    _active(true), _paused(false) {
  nassertv(audio);
  nassertv(!file_name.empty());
  audio_debug("MilesAudioSound::MilesAudioSound(manager=0x"<<(void*)&manager
      <<", audio=0x"<<(void*)audio<<", file_name="<<file_name<<")");
  // Make our own copy of the sound header data:
  _audio=AIL_quick_copy(audio);
}

MilesAudioSound::
~MilesAudioSound() {
  audio_debug("MilesAudioSound::~MilesAudioSound() "<<get_name());
  _manager.release_sound(this);
  AIL_quick_unload(_audio);
}

void MilesAudioSound::
play() {
  audio_debug("MilesAudioSound::play() "<<get_name());
  if (_active) {
    // Start playing:
    if (AIL_quick_play(_audio, _loop_count)) {
      audio_debug("  started sound");
    } else {
      audio_debug("  failed to play sound "<<AIL_last_error());
    }
  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused");
    _paused=true;
  }
}

void MilesAudioSound::
stop() {
  audio_debug("MilesAudioSound::stop() "<<get_name());
  AIL_quick_halt(_audio);
}

void MilesAudioSound::
set_loop(bool loop) {
  audio_debug("MilesAudioSound::set_loop(loop="<<loop<<") "<<get_name());
  set_loop_count((loop)?0:1);
}

bool MilesAudioSound::
get_loop() const {
  audio_debug("MilesAudioSound::get_loop() returning "<<(_loop_count==0));
  return _loop_count == 0;
}

void MilesAudioSound::
set_loop_count(unsigned long loop_count) {
  audio_debug("MilesAudioSound::set_loop_count(loop_count="<<loop_count<<") "<<get_name());
  if (_loop_count!=loop_count) {
    _loop_count=loop_count;
    if (_loop_count==0) {
      audio_debug("\n\n          looping "<<get_name()<<"\n");
    }
    if (status()==PLAYING) {
      stop();
      play();
    }
  }
}

unsigned long MilesAudioSound::
get_loop_count() const {
  audio_debug("MilesAudioSound::get_loop_count() returning "<<_loop_count);
  return _loop_count;
}

void MilesAudioSound::set_time(float start_time) {
  audio_debug("MilesAudioSound::set_time(start_time="<<start_time<<") "<<get_name());
  _start_time=start_time;
  S32 milisecond_start_time=S32(1000*_start_time);
  AIL_quick_set_ms_position(_audio, milisecond_start_time);
}

float MilesAudioSound::get_time() const {
  audio_debug("MilesAudioSound::get_time() returning "<<_start_time);
  return _start_time;
}

void MilesAudioSound::set_volume(float volume) {
  audio_debug("MilesAudioSound::set_volume(volume="<<volume<<") "<<get_name());
  // *Set the volume even if our volume is not changing, because the
  // *MilesAudioManager will call set_volume when *its* volume changes.
  // Set the volume:
  _volume=volume;
  // Account for the category of sound:
  volume*=_manager.get_volume();
  // Change to Miles volume, range 0 to 127:
  S32 milesVolume=(S32(127*volume))%128;
  // Account for type:
  S32 audioType=AIL_quick_type(_audio);
  if (audioType==AIL_QUICK_XMIDI_TYPE
      ||
      audioType==AIL_QUICK_DLS_XMIDI_TYPE) {
    // ...it's a midi file.
    AIL_quick_set_volume(_audio, milesVolume, 0); // 0 delay.
    audio_debug("  volume for this midi is now "<<milesVolume);
  } else {
    // ...it's a wav or mp3.
    // Convert balance of -1.0..1.0 to 0..127:
    S32 milesBalance=(S32(63.5*(_balance+1.0)))%128;
    AIL_quick_set_volume(_audio, milesVolume, milesBalance);
    audio_debug("  volume for this wav or mp3 is now "<<milesVolume
        <<", balance="<<milesBalance);
  }
}

float MilesAudioSound::get_volume() const {
  audio_debug("MilesAudioSound::get_volume() returning "<<_volume);
  return _volume;
}

void MilesAudioSound::set_active(bool active) {
  audio_debug("MilesAudioSound::set_active(active="<<active<<") "<<get_name());
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

bool MilesAudioSound::get_active() const {
  audio_debug("MilesAudioSound::get_active() returning "<<_active);
  return _active;
}

void MilesAudioSound::set_balance(float balance_right) {
  audio_debug("MilesAudioSound::set_balance(balance_right="<<balance_right<<") "<<get_name());
  _balance=balance_right;
  // Call set_volume to effect the change:
  set_volume(_volume);
}

float MilesAudioSound::get_balance() const {
  audio_debug("MilesAudioSound::get_balance() returning "<<_balance);
  return _balance;
}

float MilesAudioSound::length() const {
  float length;
  if (AIL_quick_status(_audio)==QSTAT_PLAYING) {
    length=((float)AIL_quick_ms_length(_audio))*0.001;
  } else {
    AIL_quick_play(_audio, 1);
    length=((float)AIL_quick_ms_length(_audio))*0.001;
    AIL_quick_halt(_audio);
  }
  audio_debug("MilesAudioSound::length() returning "<<length);
  return length;
}

const string& MilesAudioSound::get_name() const {
  //audio_debug("MilesAudioSound::get_name() returning "<<_file_name);
  return _file_name;
}

AudioSound::SoundStatus MilesAudioSound::status() const {
  if (!_audio) {
    return AudioSound::BAD;
  }
  switch (AIL_quick_status(_audio)) {
    case QSTAT_LOADED:
    case QSTAT_DONE:
      return AudioSound::READY;
    case QSTAT_PLAYING:
      return AudioSound::PLAYING;
    default:
      return AudioSound::BAD;
  }
}
/*
bool MilesAudioSound::operator==(const AudioSound& other) const {
  // These sounds only match if they are the same instance (address):
  return ((int)this) == ((int)&other);
}

bool MilesAudioSound::operator!=(const AudioSound& other) const {
  return ! (*this == other);
}
*/


#endif //]
