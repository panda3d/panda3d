// Filename: milesAudioSample.cxx
// Created by:  skyler (June 6, 2001)
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

#include "milesAudioSample.h"

#ifdef HAVE_RAD_MSS //[

#include "milesAudioManager.h"


TypeHandle MilesAudioSample::_type_handle;

#undef miles_audio_debug

#ifndef NDEBUG //[
#define miles_audio_debug(x) \
    audio_debug("MilesAudioSample \""<<get_name()<<"\" "<< x )
#else //][
#define miles_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::Constructor
//       Access: Private
//  Description: This constructor is called only by the
//               MilesAudioManager.
////////////////////////////////////////////////////////////////////
MilesAudioSample::
MilesAudioSample(MilesAudioManager *manager, MilesAudioManager::SoundData *sd, 
                 const string &file_name) :
  MilesAudioSound(manager, file_name),
  _sd(sd)
{
  nassertv(sd != NULL);
  audio_debug("MilesAudioSample(manager=0x"<<(void*)&manager
              <<", sd=0x"<<(void*)sd<<", file_name="<<file_name<<")");

  _sample = 0;
  _sample_index = 0;
  _original_playback_rate = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioSample::
~MilesAudioSample() {
  miles_audio_debug("~MilesAudioSample()");
  cleanup();
  _manager->release_sound(this);
  miles_audio_debug("~MilesAudioSample() done");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::play
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
play() {
  miles_audio_debug("play()");
  if (_active) {
    if (_sd->_raw_data.empty()) {
      milesAudio_cat.warning()
        << "Could not play " << _file_name << ": no data\n";
    } else {
      stop();
      _manager->starting_sound(this);

      nassertv(_sample == 0);

      GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
      if (!mgr->get_sample(_sample, _sample_index, this)){ 
        milesAudio_cat.warning()
          << "Could not play " << _file_name << ": too many open samples\n";
        _sample = 0;
      } else {
        AIL_set_named_sample_file(_sample, _sd->_basename.c_str(), 
                                  &_sd->_raw_data[0], _sd->_raw_data.size(),
                                  0);
        _original_playback_rate = AIL_sample_playback_rate(_sample);
        AIL_set_sample_user_data(_sample, 0, (SINTa)this);
        AIL_register_EOS_callback(_sample, finish_callback);

        set_volume(_volume);
        set_play_rate(_play_rate);
        AIL_set_sample_loop_count(_sample, _loop_count);

        if (_got_start_time) {
          do_set_time(_start_time);
          AIL_resume_sample(_sample);
        } else {
          AIL_start_sample(_sample);
        }
      }
      
      _got_start_time = false;
    }
  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused "<<_file_name );
    _paused = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::stop
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
stop() {
  miles_audio_debug("stop()");
  _manager->stopping_sound(this);
  // The _paused flag should not be cleared here.  _paused is not like
  // the Pause button on a cd/dvd player.  It is used as a flag to say
  // that it was looping when it was set inactive.  There is no need to
  // make this symmetrical with play().  set_active() is the 'owner' of
  // _paused.  play() accesses _paused to help in the situation where
  // someone calls play on an inactive sound().

  if (_sample != 0) {
    AIL_end_sample(_sample);

    GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
    mgr->release_sample(_sample_index, this);

    _sample = 0;
    _sample_index = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_time
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float MilesAudioSample::
get_time() const {
  if (_sample == 0) {
    if (_got_start_time) {
      return _start_time;
    }
    return 0.0f;
  }

  S32 current_ms;
  AIL_sample_ms_position(_sample, NULL, &current_ms);
  float time = float(current_ms * 0.001f);

  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_volume
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_volume(float volume) {
  miles_audio_debug("set_volume(volume="<<volume<<")");

  // Set the volume even if our volume is not changing, because the
  // MilesAudioManager will call set_volume() when *its* volume
  // changes.

  // Set the volume:
  _volume = volume;

  if (_sample != 0) {
    volume *= _manager->get_volume();
    
    // Change to Miles volume, range 0 to 1.0:
    F32 milesVolume = volume;
    milesVolume = min(milesVolume, 1.0f);
    milesVolume = max(milesVolume, 0.0f);
    
    // Convert balance of -1.0..1.0 to 0-1.0:
    F32 milesBalance = (F32)((_balance + 1.0f) * 0.5f);
    
    AIL_set_sample_volume_pan(_sample, milesVolume, milesBalance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_balance
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_balance(float balance_right) {
  miles_audio_debug("set_balance(balance_right="<<balance_right<<")");
  _balance = balance_right;

  // Call set_volume to effect the change:
  set_volume(_volume);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_play_rate
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_play_rate(float play_rate) {
  miles_audio_debug("set_play_rate(play_rate="<<play_rate<<")");

  // Set the play_rate:
  _play_rate = play_rate;

  if (_sample != 0) {
    play_rate *= _manager->get_play_rate();

    // wave and mp3 use sample rate (e.g. 44100)
    S32 speed = (S32)(play_rate * (float)_original_playback_rate);
    AIL_set_sample_playback_rate(_sample, speed);
    audio_debug("  play_rate for this wav or mp3 is now " << speed);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::length
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
float MilesAudioSample::
length() const {
  return _sd->get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::status
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus MilesAudioSample::
status() const {
  if (_sample == 0) {
    return AudioSound::READY;
  }
  switch (AIL_sample_status(_sample)) {
  case SMP_DONE:
  case SMP_STOPPED:
  case SMP_FREE:
    return AudioSound::READY;

  case SMP_PLAYING:
  case SMP_PLAYINGBUTRELEASED:
    return AudioSound::PLAYING;

  default:
    return AudioSound::BAD;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::cleanup
//       Access: Public, Virtual
//  Description: Stops the sound from playing and releases any
//               associated resources, in preparation for releasing
//               the sound or shutting down the sound system.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
cleanup() {
  stop();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " " << status();
  if (!_sd.is_null()) {
    out << " " << (_sd->_raw_data.size() + 1023) / 1024 << "K";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::internal_stop
//       Access: Private
//  Description: Called by the GlobalMilesManager when it is detected
//               that this particular sound has already stopped, and
//               its sample handle will be recycled.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
internal_stop() {
  _sample = 0;
  _sample_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::finish_callback
//       Access: Private, Static
//  Description: This callback is made by Miles (possibly in a
//               sub-thread) when the sample finishes.
////////////////////////////////////////////////////////////////////
void AILCALLBACK MilesAudioSample::
finish_callback(HSAMPLE sample) {
  MilesAudioSample *self = (MilesAudioSample *)AIL_sample_user_data(sample, 0);
  if (milesAudio_cat.is_debug()) {
    milesAudio_cat.debug()
      << "finished " << *self << "\n";
  }
  self->_manager->_sounds_finished = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::do_set_time
//       Access: Private
//  Description: Sets the start time of an already allocated sample.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
do_set_time(float time) {
  miles_audio_debug("do_set_time(time="<<time<<")");
  nassertv(_sample != 0);

  // Ensure we don't inadvertently run off the end of the sound.
  float max_time = length();
  if (time > max_time) {
    milesAudio_cat.warning()
      << "set_time(" << time << ") requested for sound of length " 
      << max_time << "\n";
    time = max_time;
  }
  
  S32 time_ms = (S32)(1000.0f * time);
  AIL_set_sample_ms_position(_sample, time_ms);
}

#endif //]
