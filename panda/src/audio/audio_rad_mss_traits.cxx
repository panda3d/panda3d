// Filename: audio_rad_mss_traits.cxx
// Created by:  cary (27Sep00)
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

#include "audio_rad_mss_traits.h"

#ifdef AUDIO_USE_RAD_MSS //[

#include "audio_manager.h"
#include "config_audio.h"
#include <config_util.h>

#include <direct.h>

namespace {
  void initialize() {
    static bool have_initialized;
    if (have_initialized) {
      return;
    }
    if (!audio_is_active) {
      return;
    }
    audio_debug("audio_rad_mss_traits initialize");
    ///S32 startup=AIL_quick_startup(1, 1, 22050, 16, 2);
    S32 startup=AIL_quick_startup(1, 1, 0, 0, 0); // Allow MSS to choose defaults.
    if (startup) {
      audio_debug("  AIL_quick_startup: ok "<<startup);
    } else {
      audio_error("AIL_quick_startup: **failed** "<<AIL_last_error());
    }
    have_initialized=true;
    audio_debug("out of milesAudio initialize");
  }

  void shutdown() {
    audio_debug("audio_rad_mss_traits shutdown");
    AIL_quick_shutdown();
    audio_debug("out of milesaudio shutdown"<<AIL_last_error());
  }

  bool isInactive(MilesPlaying::Category category) {
    if (!audio_is_active) {
      return true;
    }
    if (category == MilesPlaying::EFFECT
        &&
        !AudioManager::get_sfx_active()) {
      return true;
    } else if (category == MilesPlaying::MUSIC
        &&
        !AudioManager::get_music_active()) {
      return true;
    }
    return false;
  }
  
  void set_volume_helper(HAUDIO audio, float volume, 
      MilesPlaying::Category category) {
    // Account for the category of sound:
    if (category == MilesPlaying::EFFECT) {
      volume*=AudioManager::get_master_sfx_volume();
    } else if (category == MilesPlaying::MUSIC) {
      volume*=AudioManager::get_master_music_volume();
    }
    // Change to Miles:
    S32 milesVolume=S32(127*volume); // range is 0 to 127.
    milesVolume%=128;
    // Account for type:
    S32 audioType=AIL_quick_type(audio);
    if (audioType==AIL_QUICK_XMIDI_TYPE
        ||
        audioType==AIL_QUICK_DLS_XMIDI_TYPE) {
      // ...it's a midi file.
      audio_debug("adjusting sound on midi file");
      volume*=0.05;
      AIL_quick_set_volume(audio, milesVolume, 0); // 0 delay.
    } else {
      // ...it's a wav or mp3.
      audio_debug("adjusting sound on wav or mp3 (non-midi) file");
      AIL_quick_set_volume(audio, milesVolume, 64); // 64 center stereo pan.
    }
    audio_debug("  Miles volume is "<<milesVolume);
  }
}

MilesSound::~MilesSound() {
  audio_debug("MilesSound::~MilesSound");
  AIL_quick_unload(_audio);
}

float MilesSound::length() const {
  float length=float(AIL_quick_ms_length(_audio))/1000.0;
  audio_debug("MilesSound::length returning "<<length);
  return length;
}

AudioTraits::PlayingClass* MilesSound::get_state() const {
  MilesPlaying* ret = new MilesPlaying((MilesSound*)this);
  audio_debug("MilesSound::get_state returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::PlayerClass* MilesSound::get_player() const {
  AudioTraits::PlayerClass* ret = MilesPlayer::get_instance();
  audio_debug("MilesSound::get_player returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::DeletePlayingFunc* MilesSound::get_delstate() const {
  audio_debug("MilesSound::get_delstate returning 0x"
               << (void*)(MilesPlaying::destroy));
  return MilesPlaying::destroy;
}

MilesSound* MilesSound::load(Filename filename) {
  audio_debug("MilesSound::load");
  initialize();
  if (!audio_is_active) {
    return 0;
  }
  // Load the file:
  string stmp = filename.to_os_specific();
  audio_debug("  \"" << stmp << "\"");
  HAUDIO audio = AIL_quick_load(stmp.c_str());
  if (!audio) {
    audio_debug("  MilesSound::load failed " << AIL_last_error());
    return 0;
  }
  // Create the sound:
  MilesSound* sound = new MilesSound(audio);
  return sound;
}

MilesSound* MilesSound::load_raw(unsigned char* data, unsigned long size) {
  audio_debug("MilesSound::load_raw(data=0x"<<(void*)data<<", size="<<size<<")");
  initialize();
  if (!audio_is_active) {
    return 0;
  }
  if (!data) {
    audio_debug("  data is null, returning same");
    return 0;
  }
  // Load the wave:
  HAUDIO audio = AIL_quick_load_mem(data, size);
  if (!audio) {
    audio_debug("  mss load_raw failed " << AIL_last_error());
    return 0;
  }
  // Create the sound:
  MilesSound* sound = new MilesSound(audio);
  return sound;
}

MilesPlaying::MilesPlaying(AudioTraits::SoundClass* sound)
  : AudioTraits::PlayingClass(sound) {
  initialize();
  if (!audio_is_active) {
    return;
  }
  nassertv(sound);
  audio_debug("MilesPlaying::MilesPlaying(sound=0x"<<(void*)sound<<")");
  // Make our own copy of the sound header data:
  MilesSound* milesSound=static_cast<MilesSound*>(sound);
  _audio=AIL_quick_copy(milesSound->_audio);
  audio_debug("in MilesPlaying constructor");
}

MilesPlaying::~MilesPlaying() {
  audio_debug("MilesPlaying::~MilesPlaying");
  AIL_quick_unload(_audio);
}

void MilesPlaying::destroy(AudioTraits::PlayingClass* playing) {
  audio_debug("MilesPlaying::destroy(playing=0x" << (void*)playing<<")");
  delete playing;
}

AudioTraits::PlayingClass::PlayingStatus MilesPlaying::status() {
  if (!_audio) {
    return AudioTraits::PlayingClass::BAD;
  }
  switch (AIL_quick_status(_audio)) {
  case QSTAT_LOADED:
  case QSTAT_DONE:
    return AudioTraits::PlayingClass::READY;
  case QSTAT_PLAYING:
    return AudioTraits::PlayingClass::PLAYING;
  default:
    return AudioTraits::PlayingClass::BAD;
  }
}

MilesPlayer* MilesPlayer::_global_instance = (MilesPlayer*)0L;

MilesPlayer::~MilesPlayer() {
  audio_debug("MilesPlayer::~MilesPlayer");
}

void MilesPlayer::play_sound(AudioTraits::SoundClass* sound,
    AudioTraits::PlayingClass* playing, float start_time, int loop) {
  audio_debug("MilesPlayer::play_sound(sound=0x"<<(void*)sound<<", playing=0x"<<(void*)playing<<", start_time="<<start_time<<")");
  initialize();
  if (!audio_is_active) {
    return;
  }
  nassertv(sound);
  nassertv(playing);
  MilesPlaying* milesPlaying = static_cast<MilesPlaying*>(playing);
  if (isInactive(milesPlaying->get_category())) {
    return;
  }
  // Set the start time:
  S32 milisecond_start_time=S32(1000*start_time);
  //audio_debug("  setting milisecond start time to "<<milisecond_start_time);
  AIL_quick_set_ms_position(milesPlaying->_audio, milisecond_start_time);
  // Set the volume:
  //audio_debug("  playing->get_volume "<<playing->get_volume());
  //audio_debug("  AudioManager::get_master_sfx_volume "<<AudioManager::get_master_sfx_volume());
  //audio_debug("  AudioManager::get_master_music_volume "<<AudioManager::get_master_music_volume());
  set_volume_helper(milesPlaying->_audio, playing->get_volume(), playing->get_category());
  // Start playing:
  if (AIL_quick_play(milesPlaying->_audio, (loop)?0:1)) {
    audio_debug("started sound");
  } else {
    audio_debug("failed to play sound "<<AIL_last_error());
  }
  audio_debug("out of MilesPlayer play_sound");
}

void MilesPlayer::stop_sound(AudioTraits::SoundClass*,
               AudioTraits::PlayingClass* playing) {
  audio_debug("MilesPlayer::stop_sound");
  initialize();
  nassertv(playing);
  MilesPlaying* milesPlaying = static_cast<MilesPlaying*>(playing);
  if (isInactive(milesPlaying->get_category())) {
    return;
  }
  AIL_quick_halt(milesPlaying->_audio);
}

void MilesPlayer::set_volume(AudioTraits::PlayingClass* playing, float volume) {
  audio_debug("MilesPlayer::set_volume(playing=0x"<<(void*)playing<<", volume="<<volume<<")");
  initialize();
  nassertv(playing);
  MilesPlaying* milesPlaying = static_cast<MilesPlaying*>(playing);
  if (isInactive(milesPlaying->get_category())) {
    return;
  }
  set_volume_helper(milesPlaying->_audio, volume, milesPlaying->get_category());
}

bool MilesPlayer::adjust_volume(AudioTraits::PlayingClass* playing) {
  return false;
}

MilesPlayer* MilesPlayer::get_instance() {
  audio_debug("MilesPlayer::get_instance");
  if (!_global_instance) {
    _global_instance = new MilesPlayer();
  }
  audio_debug("MilesPlayer returning 0x" << (void*)_global_instance);
  return _global_instance;
}

#endif //]
