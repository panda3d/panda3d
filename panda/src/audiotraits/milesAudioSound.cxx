// Filename: milesAudioSound.cxx
// Created by:  skyler (June 6, 2001)
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

#define NEED_MILES_LENGTH_WORKAROUND

#if (((MSS_MAJOR_VERSION == 6) && (MSS_MINOR_VERSION >= 5)) || (MSS_MAJOR_VERSION >= 7))
#define MILES_6_5
#endif

#ifndef NDEBUG //[
  namespace {
    char
    getStatusChar(HAUDIO audio) {
      if (!audio) {
        return '0'; // NULL.
      }
      switch (AIL_quick_status(audio)) {
        case QSTAT_LOADED:
        case QSTAT_DONE:
          return 'r'; // Ready.
        case QSTAT_PLAYING:
          return 'p'; // Playing.
        default:
          return 'x'; // bad.
      }
    }
  }

  #define miles_audio_debug(x) \
      audio_debug("MilesAudioSound "<<getStatusChar(_audio)<<" \""<<get_name() \
      <<"\" "<< x )
#else //][
  #define miles_audio_debug(x) ((void)0)
#endif //]

MilesAudioSound::
MilesAudioSound(MilesAudioManager* manager,
    HAUDIO audio, string file_name, float length)
    : _manager(manager), _file_name(file_name),
    _volume(1.0f), _balance(0),
    _loop_count(1), _length(length),
    _active(true), _paused(false) {
  nassertv(audio);
  nassertv(!file_name.empty());
  audio_debug("MilesAudioSound(manager=0x"<<(void*)&manager
      <<", audio=0x"<<(void*)audio<<", file_name="<<file_name<<")");
  // Make our own copy of the sound header data:
  _audio=AIL_quick_copy(audio);
}

MilesAudioSound::
~MilesAudioSound() {
  miles_audio_debug("~MilesAudioSound()");
  _manager->release_sound(this);
  AIL_quick_unload(_audio);
}

void MilesAudioSound::
play() {
  #if 0
  if(_file_name.find(".mid")!=string::npos) {
     miles_audio_debug("play() midi");
  }
  #endif
  
  miles_audio_debug("play()");
  if (_active) {
    if(_manager->_bExclusive) {
        // stop any other sound that parent mgr is playing
        _manager->stop_all_sounds();
    }
    // Start playing:
    if (AIL_quick_play(_audio, _loop_count)) {
      audio_debug("  started sound " << _file_name );
    } else {
      audio_debug("  sound " << _file_name<<" failed to start, err: " <<AIL_last_error());
    }
  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused "<<_file_name );
    _paused=true;
  }
}

void MilesAudioSound::
stop() {
  #if 0
  if(_file_name.find(".mid")!=string::npos) {
        miles_audio_debug("stop() midi");
  } 
  #endif
  miles_audio_debug("stopping snd " << _file_name);
  _paused=false;
  AIL_quick_halt(_audio);
}

void MilesAudioSound::
halt() {
  #if 0
  if(_file_name.find(".mid")!=string::npos) {
        miles_audio_debug("halt() midi");
        int i=1;
  }
  #endif

  miles_audio_debug("halt()");
  AIL_quick_halt(_audio);
}

void MilesAudioSound::
set_loop(bool loop) {
  miles_audio_debug("set_loop(loop="<<loop<<")");
  // loop count of 0 means always loop
  set_loop_count((loop)?0:1);
}

bool MilesAudioSound::
get_loop() const {
  miles_audio_debug("get_loop() returning "<<(_loop_count==0));
  return (_loop_count == 0);
}

void MilesAudioSound::
set_loop_count(unsigned long loop_count) {
  miles_audio_debug("set_loop_count(loop_count="<<loop_count<<")");
  if (_loop_count!=loop_count) {
    _loop_count=loop_count;
    if (status()==PLAYING) {
      // hack:
      // For now, the loop count is picked up when the sound starts playing.
      // There may be a way to change the loop count of a playing sound, but
      // I'm going to focus on other things.  If you would like to change the
      // need to stop and start the sound, feel free.  Or, maybe I'll spend
      // time on it in the future.  Please set the loop option before starting
      // the sound.
      halt();
      play();
    }
  }
}

unsigned long MilesAudioSound::
get_loop_count() const {
  miles_audio_debug("get_loop_count() returning "<<_loop_count);
  return _loop_count;
}

void MilesAudioSound::
set_time(float start_time) {
  miles_audio_debug("set_time(start_time="<<start_time<<")");
  S32 milisecond_start_time=S32(1000*start_time);
  AIL_quick_set_ms_position(_audio, milisecond_start_time);
}

float MilesAudioSound::
get_time() const {
  S32 milisecond_start_time=AIL_quick_ms_position(_audio);
  float start_time=float(milisecond_start_time*.001);
  miles_audio_debug("get_time() returning "<<start_time);
  return start_time;
}

void MilesAudioSound::
set_volume(float volume) {
  miles_audio_debug("set_volume(volume="<<volume<<")");
  // *Set the volume even if our volume is not changing, because the
  // *MilesAudioManager will call set_volume when *its* volume changes.
  // Set the volume:
  _volume=volume;
  // Account for the category of sound:
  volume*=_manager->get_volume();
  #ifdef MILES_6_5
    // Change to Miles volume, range 0 to 1.0:
    F32 milesVolume=volume;
    milesVolume=min(milesVolume,1.0f);
    milesVolume=max(milesVolume,0.0f);
  #else
    // Change to Miles volume, range 0 to 127:
    S32 milesVolume=((S32)(127*volume))%128;
  #endif
  // Account for type:
  S32 audioType=AIL_quick_type(_audio);
  if ((audioType==AIL_QUICK_XMIDI_TYPE) || (audioType==AIL_QUICK_DLS_XMIDI_TYPE)) {
    // ...it's a midi file.

    // 0 delay, set to this volume immediately    
    #ifdef MILES_6_5
      F32 midiVolDelay =0.0f; 
    #else
      S32 midiVolDelay =0;
    #endif
    
    AIL_quick_set_volume(_audio, milesVolume, midiVolDelay); 
    audio_debug("  volume for this midi is now "<<milesVolume);
  } else {
    // ...it's a wav or mp3.
    #ifdef MILES_6_5
      // Convert balance of -1.0..1.0 to 0-1.0:
      F32 milesBalance=(F32)((_balance+1.0f)*0.5f);
    #else
      // Convert balance of -1.0..1.0 to 0..127:
      S32 milesBalance=((S32)(63.5f*(_balance+1.0f)))%128;  
    #endif
    AIL_quick_set_volume(_audio, milesVolume, milesBalance);
    audio_debug("  volume for this wav or mp3 is now " << milesVolume
        <<", balance="<<milesBalance);
  }
}

float MilesAudioSound::
get_volume() const {
  miles_audio_debug("get_volume() returning "<<_volume);
  return _volume;
}

void MilesAudioSound::
set_active(bool active) {
  miles_audio_debug("set_active(active="<<active<<")");
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

        // unlike the user fn stop(), halt() does NOT alter the 'paused' status
        // this is important, dont want to 'stop' infin looping sounds unless user
        // explicitly uses "stop()"
        halt();
      }
    }
  }
}

bool MilesAudioSound::
get_active() const {
  miles_audio_debug("get_active() returning "<<_active);
  return _active;
}

void MilesAudioSound::
set_balance(float balance_right) {
  miles_audio_debug("set_balance(balance_right="<<balance_right<<")");
  _balance=balance_right;
  // Call set_volume to effect the change:
  set_volume(_volume);
}

float MilesAudioSound::
get_balance() const {
  audio_debug("MilesAudioSound::get_balance() returning "<<_balance);
  return _balance;
}

float MilesAudioSound::
length() const {
  if (_length == 0.0f) {
   #ifndef NEED_MILES_LENGTH_WORKAROUND
        _length=((float)AIL_quick_ms_length(_audio))*0.001f;
        if (_length == 0.0f) {
            audio_error("ERROR: Miles returned length 0 for "<<_file_name << "!");
        }
   #else
        // hack:
        // For now, the sound needs to be playing, in order to
        // get the right length.  I'm in contact with RAD about the problem.  I've
        // sent them example code.  They've told me they're looking into it.
        // Until then, we'll play the sound to get the length.

        // Miles 6.5c note:  seems to be fixed for .mid, .mp3 also seems mostly fixed,
        // but not 100% positive (need to look for errors with CATCH_ERROR on)

        // #define CATCH_MILES_LENGTH_ERROR
        #ifdef CATCH_MILES_LENGTH_ERROR     
        _length=((float)AIL_quick_ms_length(_audio))*0.001f;
        if (_length == 0.0f) {
            audio_error("ERROR: Miles returned length 0 for "<<_file_name << "!");
            exit(1);
        }
        #endif

        if (AIL_quick_status(_audio)==QSTAT_PLAYING) {
          _length=((float)AIL_quick_ms_length(_audio))*0.001f;
        } else {
          AIL_quick_play(_audio, 1);
          _length=((float)AIL_quick_ms_length(_audio))*0.001f;
          AIL_quick_halt(_audio);
        }
   #endif
  }

  //audio_cat->info() << "MilesAudioSound::length() returning " << _length << endl;
  audio_debug("MilesAudioSound::length() returning "<<_length);
  return _length;
}

const string& MilesAudioSound::
get_name() const {
  //audio_debug("MilesAudioSound::get_name() returning "<<_file_name);
  return _file_name;
}

AudioSound::SoundStatus MilesAudioSound::
status() const {
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


#endif //]
