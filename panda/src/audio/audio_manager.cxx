// Filename: audio_manager.cxx
// Created by:  cary (24Sep00)
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

#include "audio_manager.h"
#include "config_audio.h"

AudioManager* AudioManager::_global_ptr = (AudioManager*)0L;
AudioManager::UpdateFunc* AudioManager::_update_func =
    (AudioManager::UpdateFunc*)0L;
AudioManager::ShutdownFunc* AudioManager::_shutdown_func =
    (AudioManager::ShutdownFunc*)0L;
mutex AudioManager::_manager_mutex;
bool AudioManager::_quit;
thread* AudioManager::_spawned = (thread*)0L;
AudioManager::LoopSet* AudioManager::_loopset = (AudioManager::LoopSet*)0L;
AudioManager::LoopSet* AudioManager::_loopcopy = (AudioManager::LoopSet*)0L;
bool AudioManager::_sfx_active = false;
bool AudioManager::_music_active = false;
bool AudioManager::_hard_sfx_active = false;
bool AudioManager::_hard_music_active = false;
bool AudioManager::_master_volume_change = false;
float AudioManager::_master_sfx_volume = 0.;
float AudioManager::_master_music_volume = 0.;

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::destructor
//       Access: Public
//  Description: delete the AudioManager singleton
////////////////////////////////////////////////////////////////////
AudioManager::
~AudioManager() {
  shutdown();
  _global_ptr = (AudioManager*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_update_func
//       Access: Public, Static
//  Description: register a function that will maintain the buffers
//               for audio output
////////////////////////////////////////////////////////////////////
void AudioManager::
set_update_func(AudioManager::UpdateFunc* func) {
  if (_update_func) {
    audio_error("There maybe be more then one audio driver installed");
  }
  _update_func = func;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::copy_loopset
//       Access: Public, Static
//  Description: make a copy of the loopset to use for the rest of
//               update
////////////////////////////////////////////////////////////////////
void AudioManager::
copy_loopset() {
  if (!_loopcopy) {
    _loopcopy = new LoopSet;
  }
  if (_loopset) {
    *_loopcopy = *_loopset;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_update
//       Access: Public, Static
//  Description: do generic update stuff
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_update() {
  // handle looping
  if (_loopcopy) {
    for (LoopSet::iterator i=_loopcopy->begin(); i!=_loopcopy->end(); ++i) {
      AudioSound* sound = *i;
      if (sound->status() == AudioSound::READY) {
        audio_debug("AudioManager::ns_update looping '" 
            << sound->get_name() << "'");
        AudioManager::play(sound);
        AudioManager::set_loop(sound, true);
      } else if (AudioManager::_master_volume_change) {
        if (sound->get_player()->adjust_volume(sound->get_state())) {
          audio_debug("AudioManager::ns_update sound is turned "
              << "off, stopping '" << sound->get_name() << "'");
          AudioManager::stop(sound);
        }
      }
    }
  }
  AudioManager::_master_volume_change = false;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_shutdown_func
//       Access: Public, Static
//  Description: register a function that will shutdown the internal
//               audio state
////////////////////////////////////////////////////////////////////
void AudioManager::
set_shutdown_func(AudioManager::ShutdownFunc* func) {
  if (_shutdown_func) {
    audio_error("There maybe be more then one audio driver installed");
  }
  _shutdown_func = func;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one AudioManager object in the system.
////////////////////////////////////////////////////////////////////
AudioManager* AudioManager::
get_ptr() {
  if (!_global_ptr) {
    _global_ptr = new AudioManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_play (AudioSound)
//       Access: Private
//  Description: get the player off the sound, and start it playing
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_play(AudioSound* sound, float start_time) {
  audio_debug("AudioManager: playing sound 0x" 
      << (void*)sound << " (" << sound->get_name() << ")");
  if (sound->status() == AudioSound::PLAYING) {
    this->ns_stop(sound);
  }
  sound->get_player()->play_sound(sound->get_sound(), 
      sound->get_state(), start_time);
  sound->get_player()->adjust_volume(sound->get_state());
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_stop (AudioSound)
//       Access: Private
//  Description: get the player off the sound, and stop it playing
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_stop(AudioSound* sound) {
  audio_debug("AudioManager: stopping sound 0x" 
      << (void*)sound << " (" << sound->get_name() << ")");
  this->ns_set_loop(sound, false);
  if (sound->status() == AudioSound::PLAYING) {
    sound->get_player()->stop_sound(sound->get_sound(), sound->get_state());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_loop (AudioSound)
//       Access: Private
//  Description: set the looping state of the given sound
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_set_loop(AudioSound* sound, bool state) {
  mutex_lock l(_manager_mutex);
  if (!_loopset) {
    _loopset = new LoopSet;
  }
  if (state) {
    _loopset->insert(sound);
  } else if (_loopset->find(sound) != _loopset->end()) {
    _loopset->erase(sound);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_get_loop (AudioSound)
//       Access: Private
//  Description: get the looping state of the given sound
////////////////////////////////////////////////////////////////////
bool AudioManager::
ns_get_loop(AudioSound* sound) {
  mutex_lock l(_manager_mutex);
  if (!_loopset) {
    return false;
  }
  LoopSet::iterator i = _loopset->find(sound);
  return i != _loopset->end();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawned_update
//       Access: static
//  Description: the thread function to call update forever.
////////////////////////////////////////////////////////////////////
void* AudioManager::
spawned_update(void* data) {
  bool* flag = (bool*)data;
  try {
    // *flag connects to AudioManager::_quit
    while (! (*flag)) {
      AudioManager::update();
      ipc_traits::sleep(0, audio_auto_update_delay);
    }
  } catch (...) {
    audio_error("Uncought Exception in audio update thread.");
    throw;
  }
  // Switch the flag back to false,
  // so AudioManager::ns_shutdown()
  // knows we're done:
  *flag = false;
  audio_debug("exiting update thread");
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_volume (AudioSound)
//       Access: Private
//  Description: get the player off the sound, and set volume on it
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_set_volume(AudioSound* sound, float v) {
  sound->get_player()->set_volume(sound->get_state(), v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_spawn_update
//       Access: Private
//  Description: spawn a thread that calls update every so often
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_spawn_update() {
  if (!_spawned) {
    _quit = false;
    thread::priority_t pri;
    switch (audio_thread_priority) {
    case 0:
      pri = thread::PRIORITY_LOW;
      break;
    case 1:
      pri = thread::PRIORITY_NORMAL;
      break;
    case 2:
      pri = thread::PRIORITY_HIGH;
      break;
    default:
      audio_error("audio-thread-priority set to something other "
          << "then low, normal, or high");
      audio_thread_priority = 1;
      pri = thread::PRIORITY_NORMAL;
    }
    _spawned = thread::create(spawned_update, &_quit, pri);
  } else {
    audio_error("tried to spawn 2 update threads");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_shutdown
//       Access: Private
//  Description: non-static implementation of shutdown stuff
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_shutdown() {
  audio_debug("AudioManager::ns_shutdown()");
  _quit = true;
  if (_shutdown_func) {
    (*_shutdown_func)();
  }
  if (_spawned) {
    while (_quit) {
      // waiting on update thread to stop spinning.
    }
  }
  audio_debug("update thread has shutdown");
}
