// Filename: audio_manager.cxx
// Created by:  cary (24Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_manager.h"
#include "config_audio.h"

AudioManager* AudioManager::_global_ptr = (AudioManager*)0L;
AudioManager::UpdateFunc* AudioManager::_update_func =
    (AudioManager::UpdateFunc*)0L;
mutex AudioManager::_manager_mutex;

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::set_update_func
//       Access: Public, Static
//  Description: register a function that will maintain the buffers
//               for audio output
////////////////////////////////////////////////////////////////////
void AudioManager::set_update_func(AudioManager::UpdateFunc* func) {
  if (_update_func != (AudioManager::UpdateFunc*)0L)
    audio_cat->error() << "There maybe be more then one audio driver installed"
		       << endl;
  _update_func = func;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one AudioManager object in the system.
////////////////////////////////////////////////////////////////////
AudioManager* AudioManager::get_ptr(void) {
  if (_global_ptr == (AudioManager*)0L)
    _global_ptr = new AudioManager;
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_play (AudioSample)
//       Access: Private
//  Description: get the player off the sample, and start it playing
////////////////////////////////////////////////////////////////////
void AudioManager::ns_play(AudioSample* sample) {
  sample->get_player()->play_sample(sample->get_sample());
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_play (AudioMusic)
//       Access: Private
//  Description: get the player off the music, and start it playing
////////////////////////////////////////////////////////////////////
void AudioManager::ns_play(AudioMusic* music) {
  music->get_player()->play_music(music->get_music());
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawned_update
//       Access: static
//  Description: the thread function to call update forever.
////////////////////////////////////////////////////////////////////
void AudioManager::spawned_update(void*) {
  while (1) {
    AudioManager::update();
    ipc_traits::sleep(0, 1000000);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_volume (AudioSample)
//       Access: Private
//  Description: get the player off the sample, and set volume on it
////////////////////////////////////////////////////////////////////
void AudioManager::ns_set_volume(AudioSample* sample, int v) {
  sample->get_player()->set_volume(sample->get_sample(), v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_volume (AudioMusic)
//       Access: Private
//  Description: get the player off the music, and set volume on it
////////////////////////////////////////////////////////////////////
void AudioManager::ns_set_volume(AudioMusic* music, int v) {
  music->get_player()->set_volume(music->get_music(), v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_spawn_update
//       Access: Private
//  Description: spawn a thread that calls update every so often
////////////////////////////////////////////////////////////////////
void AudioManager::ns_spawn_update(void) {
  _spawned = thread::create(spawned_update, (void*)0L,
			    thread::PRIORITY_NORMAL);
}
