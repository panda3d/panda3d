// Filename: audio_manager.I
// Created by:  cary (24Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::play (AudioSound)
//       Access: Public, Static
//  Description: Play some audio
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::play(AudioSound* sound, float start_time) {
  if (audio_is_active)
    get_ptr()->ns_play(sound, start_time);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::update
//       Access: Public, Static
//  Description: make sure buffers are full
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::update(void) {
  if (!audio_is_active)
    return;
  {
    mutex_lock l(_manager_mutex);
    get_ptr()->copy_loopset();
  }
  if (_update_func != (UpdateFunc*)0L)
    (*_update_func)();
  get_ptr()->ns_update();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawn_update
//       Access: Public, Static
//  Description: spawn a thread to call update
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::spawn_update(void) {
  if (audio_is_active)
    get_ptr()->ns_spawn_update();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::shutdown
//       Access: Public, Static
//  Description: kill any internal threads, free any internal data
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::shutdown(void) {
  get_ptr()->ns_shutdown();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_volume (sound)
//       Access: Public, Static
//  Description: set the volume on a sound instance
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_volume(AudioSound* sound, float v) {
  if (audio_is_active)
    get_ptr()->ns_set_volume(sound, v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::stop
//       Access: Public, Static
//  Description: stop playing a given sound
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::stop(AudioSound* sound) {
  if (audio_is_active)
    get_ptr()->ns_stop(sound);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::stop
//       Access: Public, Static
//  Description: set the looping state of the given sound
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_loop(AudioSound* sound, bool state) {
  if (audio_is_active)
    get_ptr()->ns_set_loop(sound, state);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_loop
//       Access: Public, Static
//  Description: return the looping state of the given sound
////////////////////////////////////////////////////////////////////
INLINE bool AudioManager::get_loop(AudioSound* sound) {
  return get_ptr()->ns_get_loop(sound);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one AudioManager
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE AudioManager::AudioManager(void) {
  _sfx_active = _hard_sfx_active = audio_sfx_active;
  _music_active = _hard_music_active = audio_music_active;
  _master_sfx_volume = audio_master_sfx_volume;
  _master_music_volume = audio_master_music_volume;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_master_sfx_volume
//       Access: Public, Static
//  Description: set the overall volume of SFX
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_master_sfx_volume(float v) {
  AudioManager::_master_sfx_volume = v;
  AudioManager::_master_volume_change = true;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_master_music_volume
//       Access: Public, Static
//  Description: set the overall volume of music
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_master_music_volume(float v) {
  get_ptr();
  AudioManager::_master_music_volume = v;
  AudioManager::_master_volume_change = true;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_master_sfx_volume
//       Access: Public, Static
//  Description: return the overall volume of SFX
////////////////////////////////////////////////////////////////////
INLINE float AudioManager::get_master_sfx_volume(void) {
  get_ptr();
  return AudioManager::_master_sfx_volume;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_master_music_volume
//       Access: Public, Static
//  Description: return the overall volume of music
////////////////////////////////////////////////////////////////////
INLINE float AudioManager::get_master_music_volume(void) {
  get_ptr();
  return AudioManager::_master_music_volume;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_all_sound_active
//       Access: Public, Static
//  Description: turn on/off all audio
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_all_sound_active(bool f) {
  get_ptr();
  audio_is_active = f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_all_sound_active
//       Access: Public, Static
//  Description: turn on/off all audio
////////////////////////////////////////////////////////////////////
INLINE bool AudioManager::get_all_sound_active(void) {
  get_ptr();
  return audio_is_active;
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
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_sfx_active
//       Access: Public, Static
//  Description: return the state of SFX playing
////////////////////////////////////////////////////////////////////
INLINE bool AudioManager::get_sfx_active(void) {
  get_ptr();
  return (AudioManager::_sfx_active && audio_is_active);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_music_active
//       Access: Public, Static
//  Description: return the state of music playing
////////////////////////////////////////////////////////////////////
INLINE bool AudioManager::get_music_active(void) {
  get_ptr();
  return (AudioManager::_music_active && audio_is_active);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_hard_sfx_active
//       Access: Public, Static
//  Description: set the hardware availability state for SFX
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_hard_sfx_active(bool f) {
  get_ptr();
  AudioManager::_hard_sfx_active = f;
  if (!f)
    AudioManager::_sfx_active = f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_hard_music_active
//       Access: Public, Static
//  Description: set the hardware availability state for music
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_hard_music_active(bool f) {
  get_ptr();
  AudioManager::_hard_music_active = f;
  if (!f)
    AudioManager::_music_active = f;
}
