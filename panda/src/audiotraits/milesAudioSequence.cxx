// Filename: milesAudioSequence.cxx
// Created by:  drose (31Jul07)
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

#include "milesAudioSequence.h"

#ifdef HAVE_RAD_MSS //[

#include "milesAudioManager.h"


TypeHandle MilesAudioSequence::_type_handle;

#undef miles_audio_debug

#ifndef NDEBUG //[
#define miles_audio_debug(x) \
    audio_debug("MilesAudioSequence \""<<get_name()<<"\" "<< x )
#else //][
#define miles_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::Constructor
//       Access: Private
//  Description: This constructor is called only by the
//               MilesAudioManager.
////////////////////////////////////////////////////////////////////
MilesAudioSequence::
MilesAudioSequence(MilesAudioManager *manager, MilesAudioManager::SoundData *sd, 
                 const string &file_name) :
  MilesAudioSound(manager, file_name),
  _sd(sd)
{
  nassertv(sd != NULL);
  audio_debug("MilesAudioSequence(manager=0x"<<(void*)&manager
              <<", sd=0x"<<(void*)sd<<", file_name="<<file_name<<")");

  _sequence = 0;
  _sequence_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioSequence::
~MilesAudioSequence() {
  miles_audio_debug("~MilesAudioSequence()");
  cleanup();
  _manager->release_sound(this);
  miles_audio_debug("~MilesAudioSequence() done");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::play
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
play() {
  miles_audio_debug("play()");
  if (_active) {
    stop();
    
    if (_sd->_raw_data.empty()) {
      milesAudio_cat.warning()
        << "Could not play " << _file_name << ": no data\n";
    } else {
      _manager->starting_sound(this);
      nassertv(_sequence == 0);

      GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
      if (!mgr->get_sequence(_sequence, _sequence_index, this)){ 
        milesAudio_cat.warning()
          << "Could not play " << _file_name << ": too many open sequences\n";
        _sequence = 0;
      } else {
        AIL_init_sequence(_sequence, &_sd->_raw_data[0], 0);
        AIL_set_sequence_user_data(_sequence, 0, (SINTa)this);
        AIL_register_sequence_callback(_sequence, finish_callback);

        set_volume(_volume);
        set_play_rate(_play_rate);
        AIL_set_sequence_loop_count(_sequence, _loop_count);

        if (_got_start_time) {
          do_set_time(_start_time);
          AIL_resume_sequence(_sequence);
        } else {
          AIL_start_sequence(_sequence);
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
//     Function: MilesAudioSequence::stop
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
stop() {
  miles_audio_debug("stop()");
  _manager->stopping_sound(this);
  // The _paused flag should not be cleared here.  _paused is not like
  // the Pause button on a cd/dvd player.  It is used as a flag to say
  // that it was looping when it was set inactive.  There is no need to
  // make this symmetrical with play().  set_active() is the 'owner' of
  // _paused.  play() accesses _paused to help in the situation where
  // someone calls play on an inactive sound().

  if (_sequence != 0) {
    AIL_end_sequence(_sequence);

    GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
    mgr->release_sequence(_sequence_index, this);

    _sequence = 0;
    _sequence_index = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::get_time
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSequence::
get_time() const {
  if (_sequence == 0) {
    if (_got_start_time) {
      return _start_time;
    }
    return 0.0f;
  }

  S32 current_ms;
  AIL_sequence_ms_position(_sequence, NULL, &current_ms);
  PN_stdfloat time = PN_stdfloat(current_ms * 0.001f);

  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::set_volume
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
set_volume(PN_stdfloat volume) {
  miles_audio_debug("set_volume(volume="<<volume<<")");

  // Set the volume even if our volume is not changing, because the
  // MilesAudioManager will call set_volume() when *its* volume
  // changes.

  // Set the volume:
  _volume = volume;

  if (_sequence != 0) {
    volume *= _manager->get_volume();
    
    // Change to Miles volume, range 0 to 127:
    S32 milesVolume = (S32)(volume * 127.0f);
    milesVolume = min(milesVolume, 127);
    milesVolume = max(milesVolume, 0);
    
    AIL_set_sequence_volume(_sequence, milesVolume, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::set_balance
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
set_balance(PN_stdfloat balance_right) {
  miles_audio_debug("set_balance(balance_right="<<balance_right<<")");
  _balance = balance_right;

  // Balance has no effect on a MIDI file.
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::set_play_rate
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
set_play_rate(PN_stdfloat play_rate) {
  miles_audio_debug("set_play_rate(play_rate="<<play_rate<<")");

  // Set the play_rate:
  _play_rate = play_rate;

  if (_sequence != 0) {
    play_rate *= _manager->get_play_rate();

    S32 percent = (S32)(play_rate * 100.0f);
    AIL_set_sequence_tempo(_sequence, percent, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::length
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSequence::
length() const {
  if (_sequence == 0) {
    // The MIDI file hasn't been started yet.  See if the length is
    // cached in the SoundData.
    if (!_sd->_has_length) {
      // It isn't cached, so load the sequence temporarily to
      // determine its length.
      ((MilesAudioSequence *)this)->determine_length();
    }
     
    return _sd->get_length();
  }

  // The MIDI file has already been started, so we can ask it
  // directly.
  S32 length_ms;
  AIL_sequence_ms_position(_sequence, &length_ms, NULL);
  PN_stdfloat time = (PN_stdfloat)length_ms * 0.001f;
  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::status
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus MilesAudioSequence::
status() const {
  if (_sequence == 0) {
    return AudioSound::READY;
  }
  switch (AIL_sequence_status(_sequence)) {
  case SEQ_DONE:
  case SEQ_STOPPED:
  case SEQ_FREE:
    return AudioSound::READY;

  case SEQ_PLAYING:
  case SEQ_PLAYINGBUTRELEASED:
    return AudioSound::PLAYING;

  default:
    return AudioSound::BAD;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::cleanup
//       Access: Public, Virtual
//  Description: Stops the sound from playing and releases any
//               associated resources, in preparation for releasing
//               the sound or shutting down the sound system.
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
cleanup() {
  stop();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::internal_stop
//       Access: Private
//  Description: Called by the GlobalMilesManager when it is detected
//               that this particular sound has already stopped, and
//               its sequence handle will be recycled.
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
internal_stop() {
  _sequence = 0;
  _sequence_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::finish_callback
//       Access: Private, Static
//  Description: This callback is made by Miles (possibly in a
//               sub-thread) when the sequence finishes.
////////////////////////////////////////////////////////////////////
void AILCALLBACK MilesAudioSequence::
finish_callback(HSEQUENCE sequence) {
  MilesAudioSequence *self = (MilesAudioSequence *)AIL_sequence_user_data(sequence, 0);
  if (milesAudio_cat.is_debug()) {
    milesAudio_cat.debug()
      << "finished " << *self << "\n";
  }
  self->_manager->_sounds_finished = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::do_set_time
//       Access: Private
//  Description: Sets the start time of an already allocated stream.
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
do_set_time(PN_stdfloat time) {
  miles_audio_debug("do_set_time(time="<<time<<")");

  nassertv(_sequence != 0);

  S32 time_ms = (S32)(1000.0f * time);

    // Ensure we don't inadvertently run off the end of the sound.
  S32 length_ms;
  AIL_sequence_ms_position(_sequence, &length_ms, NULL);
  time_ms = min(time_ms, length_ms);
  
  AIL_set_sequence_ms_position(_sequence, time_ms);
}


////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSequence::determine_length
//       Access: Private
//  Description: Temporarily loads the sequence to determine its
//               length.  Stores the result on the _sd.
////////////////////////////////////////////////////////////////////
void MilesAudioSequence::
determine_length() {
  nassertv(_sequence == 0);

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  if (!mgr->get_sequence(_sequence, _sequence_index, this)){ 
    milesAudio_cat.warning()
      << "Could not determine length of " << _file_name << ": too many open sequences\n";
    _sequence = 0;
  } else {
    AIL_init_sequence(_sequence, &_sd->_raw_data[0], 0);
    S32 length_ms;
    AIL_sequence_ms_position(_sequence, &length_ms, NULL);
    PN_stdfloat time = (PN_stdfloat)length_ms * 0.001f;
    mgr->release_sequence(_sequence_index, this);
    _sequence = 0;
    _sequence_index = 0;
    
    _sd->set_length(time);
  }
}

#endif //]
