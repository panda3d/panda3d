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
  mutex_lock l(_manager_mutex);
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
INLINE AudioManager::AudioManager(void) {}
