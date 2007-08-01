// Filename: milesAudioStream.cxx
// Created by:  drose (26Jul07)
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

#include "milesAudioStream.h"
#ifdef HAVE_RAD_MSS //[

#include "milesAudioManager.h"
#include "pnotify.h"

TypeHandle MilesAudioStream::_type_handle;

#undef miles_audio_debug

#ifndef NDEBUG //[
#define miles_audio_debug(x) \
    audio_debug("MilesAudioStream \""<<get_name()<<"\" "<< x )
#else //][
#define miles_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioStream::
MilesAudioStream(MilesAudioManager *manager, const string &file_name,
                 const Filename &path) :
  MilesAudioSound(manager, file_name),
  _path(path)
{
  _stream = 0;
  _got_length = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioStream::
~MilesAudioStream() {
  miles_audio_debug("~MilesAudioStream()");
  cleanup();

  _manager->release_sound(this);
  miles_audio_debug("~MilesAudioStream() done");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::play
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
play() {
  miles_audio_debug("play()");
  if (_active) {

    _manager->starting_sound(this);

    if (_stream == 0) {
      GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
      _stream = AIL_open_stream(mgr->_digital_driver, _path.c_str(), 0);
    } else {
      // We already had the stream open.  Keep it open; just restart
      // it.
      AIL_pause_stream(_stream, 1);
      _manager->stop_service_stream(_stream);
    }

    if (_stream == 0) {
      milesAudio_cat.warning()
        << "Could not play " << _file_name << ": too many open streams\n";

    } else {
      // Start playing:
      HSAMPLE sample = AIL_stream_sample_handle(_stream);
      nassertv(sample != 0);
      
      _original_playback_rate = AIL_sample_playback_rate(sample);
      set_volume(_volume);
      set_play_rate(_play_rate);
      
      AIL_set_stream_loop_count(_stream, _loop_count);
      
      if (miles_audio_panda_threads) {
        AIL_auto_service_stream(_stream, 0);
        _manager->start_service_stream(_stream);
      } else {
        AIL_auto_service_stream(_stream, 1);
      }
      AIL_start_stream(_stream);
    }

  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused "<<_file_name );
    _paused = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::stop
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
stop() {
  miles_audio_debug("stop()");
  _manager->stopping_sound(this);

  // The _paused flag should not be cleared here.  _paused is not like
  // the Pause button on a cd/dvd player.  It is used as a flag to say
  // that it was looping when it was set inactive.  There is no need to
  // make this symmetrical with play().  set_active() is the 'owner' of
  // _paused.  play() accesses _paused to help in the situation where
  // someone calls play on an inactive sound().
  if (_stream != 0) {
    _manager->stop_service_stream(_stream);

    AIL_pause_stream(_stream, 1);
    AIL_close_stream(_stream);
    _stream = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
set_time(float time) {
  if (_stream != 0) {
    S32 time_ms = (S32)(1000.0f * time);

    // Ensure we don't inadvertently run off the end of the sound.
    S32 length_ms;
    AIL_stream_ms_position(_stream, &length_ms, NULL);
    time_ms = min(time_ms, length_ms);
    
    AIL_set_stream_ms_position(_stream, time_ms);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float MilesAudioStream::
get_time() const {
  nassertr(_stream, 0.0f);

  S32 current_ms;
  AIL_stream_ms_position(_stream, NULL, &current_ms);
  float time = float(current_ms * 0.001f);

  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
set_volume(float volume) {
  _volume = volume;

  if (_stream != 0) {
    HSAMPLE sample = AIL_stream_sample_handle(_stream);
    nassertv(sample != 0);

    volume *= _manager->get_volume();
    
    // Change to Miles volume, range 0 to 1.0:
    F32 milesVolume = volume;
    milesVolume = min(milesVolume, 1.0f);
    milesVolume = max(milesVolume, 0.0f);
    
    // Convert balance of -1.0..1.0 to 0-1.0:
    F32 milesBalance = (F32)((_balance + 1.0f) * 0.5f);
    
    AIL_set_sample_volume_pan(sample, milesVolume, milesBalance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::set_balance
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
set_balance(float balance_right) {
  _balance = balance_right;

  // Call set_volume to effect the change:
  set_volume(_volume);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::set_play_rate
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
set_play_rate(float play_rate) {
  _play_rate = play_rate;

  if (_stream != 0) {
    HSAMPLE sample = AIL_stream_sample_handle(_stream);
    nassertv(sample != 0);

    play_rate *= _manager->get_play_rate();

    // wave and mp3 use sample rate (e.g. 44100)
    S32 speed = (S32)(play_rate * (float)_original_playback_rate);
    AIL_set_sample_playback_rate(sample, speed);
    audio_debug("  play_rate for this wav or mp3 is now " << speed);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::length
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float MilesAudioStream::
length() const {
  if (!_got_length) {
    if (_stream == 0) {
      GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
      ((MilesAudioStream *)this)->_stream = AIL_open_stream(mgr->_digital_driver, _path.c_str(), 0);
    }
    
    S32 length_ms;
    AIL_stream_ms_position(_stream, &length_ms, NULL);
    _length = (float)length_ms * 0.001f;
    _got_length = true;
  }
    
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::status
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus MilesAudioStream::
status() const {
  if (!_stream) {
    return AudioSound::READY;
  }

  switch (AIL_stream_status(_stream)) {
  case SMP_STOPPED:
  case SMP_DONE:
    return AudioSound::READY;
  case SMP_PLAYING:
  case SMP_PLAYINGBUTRELEASED:
    return AudioSound::PLAYING;
  default:
    return AudioSound::BAD;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioStream::cleanup
//       Access: Public, Virtual
//  Description: Called to release any resources associated with the
//               sound.
////////////////////////////////////////////////////////////////////
void MilesAudioStream::
cleanup() {
  if (_stream) {
    stop();
  }
}


#endif //]
