/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file milesAudioSound.cxx
 * @author drose
 * @date 2007-07-30
 */

#include "milesAudioSound.h"
#ifdef HAVE_RAD_MSS //[

#include "milesAudioManager.h"

TypeHandle MilesAudioSound::_type_handle;

#undef miles_audio_debug

#ifndef NDEBUG //[
#define miles_audio_debug(x) \
    audio_debug("MilesAudioSound \""<<get_name()<<"\" "<< x )
#else //][
#define miles_audio_debug(x) ((void)0)
#endif //]

/**

 */
MilesAudioSound::
MilesAudioSound(MilesAudioManager *manager,
                const string &file_name) :
  _manager(manager),
  _file_name(file_name),
  _volume(1.0f), _balance(0), _play_rate(1.0f),
  _loop_count(1),
  _active(true),
  _paused(false),
  _start_time(0.0f),
  _got_start_time(false)
{
  nassertv(!file_name.empty());
}

/**

 */
void MilesAudioSound::
set_loop(bool loop) {
  // loop count of 0 means always loop
  set_loop_count((loop)?0:1);
}

/**

 */
bool MilesAudioSound::
get_loop() const {
  return (_loop_count == 0);
}

/**

 */
void MilesAudioSound::
set_loop_count(unsigned long loop_count) {
  if (_loop_count != loop_count) {
    _loop_count = loop_count;
    if (status() == PLAYING) {
      // hack:
      // For now, the loop count is picked up when the sound starts playing.
      // There may be a way to change the loop count of a playing sound, but
      // I'm going to focus on other things.  If you would like to change the
      // need to stop and start the sound, feel free.  Or, maybe I'll spend
      // time on it in the future.  Please set the loop option before starting
      // the sound.
      play();
    }
  }
}

/**

 */
unsigned long MilesAudioSound::
get_loop_count() const {
  return _loop_count;
}

/**

 */
PN_stdfloat MilesAudioSound::
get_volume() const {
  return _volume;
}

/**

 */
PN_stdfloat MilesAudioSound::
get_balance() const {
  return _balance;
}

/**

 */
PN_stdfloat MilesAudioSound::
get_play_rate() const {
  return _play_rate;
}

/**

 */
void MilesAudioSound::
set_time(PN_stdfloat time) {
  miles_audio_debug("set_time(time="<<time<<")");

  // Mark this position for the next play().
  _start_time = time;
  _got_start_time = true;
}

/**

 */
void MilesAudioSound::
set_active(bool active) {
  if (_manager == (MilesAudioManager *)NULL) {
    return;
  }

  miles_audio_debug("set_active(active="<<active<<")");
  if (_active != active) {
    _active = active;
    if (_active) {
      // ...activate the sound.
      if (_paused && _loop_count==0) {
        // ...this sound was looping when it was paused.
        _paused = false;
        play();
      }

    } else {
      // ...deactivate the sound.
      if (status() == PLAYING) {
        if (_loop_count == 0) {
          // ...we're pausing a looping sound.
          _paused = true;
        }
        _start_time = get_time();
        _got_start_time = true;
        stop();
      }
    }
  }
}

/**

 */
bool MilesAudioSound::
get_active() const {
  return _active;
}

/**
 * This is no longer implemented.
 */
void MilesAudioSound::
set_finished_event(const string &event) {
  _finished_event = event;
}

/**
 * This is no longer implemented.
 */
const string &MilesAudioSound::
get_finished_event() const {
  return _finished_event;
}

/**

 */
const string &MilesAudioSound::
get_name() const {
  return _file_name;
}

/**
 * Stops the sound from playing and releases any associated resources, in
 * preparation for releasing the sound or shutting down the sound system.
 */
void MilesAudioSound::
cleanup() {
}

#endif  //]
