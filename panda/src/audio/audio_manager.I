// Filename: audio_manager.I
// Created by:  cary (24Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::play (AudioSample)
//       Access: Public, Static
//  Description: Play an audio sample
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::play(AudioSample* sample) {
  get_ptr()->ns_play(sample);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::play (AudioMusic)
//       Access: Public, Static
//  Description: Play an audio music instance
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::play(AudioMusic* music) {
  get_ptr()->ns_play(music);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::update
//       Access: Public, Static
//  Description: make sure buffers are full
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::update(void) {
  mutex_lock l(_manager_mutex);
  if (_update_func != (UpdateFunc*)0L)
    (*_update_func)();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawn_update
//       Access: Public, Static
//  Description: spawn a thread to call update
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::spawn_update(void) {
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
//     Function: AudioManager::set_volume (sample)
//       Access: Public, Static
//  Description: set the volume on a sample
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_volume(AudioSample* sample, int v) {
  get_ptr()->ns_set_volume(sample, v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_volume (music)
//       Access: Public, Static
//  Description: set the volume on music
////////////////////////////////////////////////////////////////////
INLINE void AudioManager::set_volume(AudioMusic* music, int v) {
  get_ptr()->ns_set_volume(music, v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one AudioManager
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE AudioManager::AudioManager(void) {}
