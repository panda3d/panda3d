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

#ifdef AUDIO_USE_RAD_MSS //[
#include "mss.h"
#endif //]


// Statics (all default to zero):
AudioManager* AudioManager::_global_ptr;
AudioManager::UpdateFunc* AudioManager::_update_func;
AudioManager::ShutdownFunc* AudioManager::_shutdown_func;
mutex AudioManager::_manager_mutex;
volatile bool AudioManager::_quit;
thread* AudioManager::_spawned;
AudioManager::LoopSet* AudioManager::_loopset;
AudioManager::LoopSet* AudioManager::_loopcopy;
bool AudioManager::_sfx_active;
bool AudioManager::_music_active;
bool AudioManager::_hard_sfx_active;
bool AudioManager::_hard_music_active;
bool AudioManager::_master_volume_change;
float AudioManager::_master_sfx_volume;
float AudioManager::_master_music_volume;

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::destructor
//       Access: Public
//  Description: delete the AudioManager singleton
////////////////////////////////////////////////////////////////////
AudioManager::
~AudioManager() {
  shutdown();
  _global_ptr = 0;
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
  #ifndef AUDIO_USE_RAD_MSS //[
    // handle looping
    if (_loopcopy) {
      LoopSet::iterator i=_loopcopy->begin();
      for (; i!=_loopcopy->end(); ++i) {
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
  #endif //]
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
ns_play(AudioSound* sound, float start_time, int loop) {
  audio_debug("AudioManager::ns_play(sound=0x"<<(void*)sound<<", start_time="<<start_time<<", loop="<<loop<<")");
  audio_debug("  name="<<sound->get_name());
  if (sound->status() == AudioSound::PLAYING) {
    this->ns_stop(sound);
  }
  #ifndef AUDIO_USE_RAD_MSS //[
    sound->get_player()->play_sound(sound->get_sound(), 
        sound->get_state(), start_time, loop);
    ns_set_loop(sound, loop);
    sound->get_player()->adjust_volume(sound->get_state());
  #else //][
    sound->get_player()->play_sound(sound->get_sound(), 
        sound->get_state(), start_time, loop);
  #endif //]
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
  #ifndef AUDIO_USE_RAD_MSS //[
    this->ns_set_loop(sound, false);
  #endif //]
  if (sound->status() == AudioSound::PLAYING) {
    sound->get_player()->stop_sound(sound->get_sound(), sound->get_state());
  }
}

#ifndef AUDIO_USE_RAD_MSS //[
////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_loop (AudioSound)
//       Access: Private
//  Description: set the looping state of the given sound
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_set_loop(AudioSound* sound, bool state) {
  #ifndef AUDIO_USE_RAD_MSS //[
    mutex_lock l(_manager_mutex);
    if (!_loopset) {
      _loopset = new LoopSet;
    }
    if (state) {
      _loopset->insert(sound);
    } else {
      _loopset->erase(sound);
    }
  #else //][
    /////sound->get_player()->set_loop_count((state)?0:1);
  #endif //]
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
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawned_update
//       Access: static
//  Description: the thread function to call update forever.
////////////////////////////////////////////////////////////////////
void* AudioManager::
spawned_update(void* data) {
  #ifndef HAVE_RAD_MSS //[
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
  #else //][
    return 0;
  #endif //]
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
//     Function: AudioManager::set_master_sfx_volume
//       Access: Public, Static
//  Description: set the overall volume of SFX
////////////////////////////////////////////////////////////////////
void AudioManager::set_master_sfx_volume(float v) {
  AudioManager::_master_sfx_volume = v;
  AudioManager::_master_volume_change = true;
  #ifdef AUDIO_USE_RAD_MSS //[
    HDIGDRIVER dig;
    AIL_quick_handles(&dig, 0, 0);
    S32 volume=((int)(127*v))%128;
    AIL_set_digital_master_volume(dig, volume);
  #endif //]
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_master_music_volume
//       Access: Public, Static
//  Description: set the overall volume of music
////////////////////////////////////////////////////////////////////
void AudioManager::set_master_music_volume(float v) {
  get_ptr();
  AudioManager::_master_music_volume = v;
  AudioManager::_master_volume_change = true;
  #ifdef AUDIO_USE_RAD_MSS //[
    HMDIDRIVER mdi;
    AIL_quick_handles(0, &mdi, 0);
    S32 volume=((int)(127*v))%128;
    AIL_set_XMIDI_master_volume(mdi, volume);
  #endif //]
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_sfx_active
//       Access: Public, Static
//  Description: turn on/off SFX
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_sfx_active(bool f) {
  get_ptr();
  if (f) {
    if (AudioManager::_hard_sfx_active) {
      AudioManager::_sfx_active = f;
      AudioManager::_master_volume_change = true;
    }
  } else {
    AudioManager::_sfx_active = f;
    AudioManager::_master_volume_change = true;
  }
  #ifdef AUDIO_USE_RAD_MSS //[
    if (f) {
      set_master_sfx_volume(_master_sfx_volume);
    } else {
      HDIGDRIVER dig;
      AIL_quick_handles(&dig, 0, 0);
      AIL_set_digital_master_volume(dig, 0);
    }
  #endif //]
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_music_active
//       Access: Public, Static
//  Description: turn on/off music
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_music_active(bool f) {
  get_ptr();
  if (f) {
    if (AudioManager::_hard_music_active) {
      AudioManager::_music_active = f;
      AudioManager::_master_volume_change = true;
    }
  } else {
    AudioManager::_music_active = f;
    AudioManager::_master_volume_change = true;
  }
  #ifdef AUDIO_USE_RAD_MSS //[
    if (f) {
      set_master_music_volume(_master_music_volume);
    } else {
      HMDIDRIVER mdi;
      AIL_quick_handles(0, &mdi, 0);
      AIL_set_XMIDI_master_volume(mdi, 0);
    }
  #endif //]
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_spawn_update
//       Access: Private
//  Description: spawn a thread that calls update every so often
////////////////////////////////////////////////////////////////////
void AudioManager::
ns_spawn_update() {
  #ifndef HAVE_RAD_MSS //[
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
      _spawned = thread::create(spawned_update, (void*)&_quit, pri);
    } else {
      audio_error("tried to spawn 2 update threads");
    }
  #endif //]
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
  #ifndef HAVE_RAD_MSS //[
    if (_spawned) {
      while (_quit) {
        // waiting on update thread to stop spinning.
      }
    }
  #endif //]
  audio_debug("update thread has shutdown");
}
