// Filename: audio_manager.cxx
// Created by:  cary (24Sep00)
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
bool* AudioManager::_quit = (bool*)0L;
thread* AudioManager::_spawned = (thread*)0L;

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::destructor
//       Access: Public
//  Description: delete the AudioManager singleton
////////////////////////////////////////////////////////////////////
AudioManager::~AudioManager(void) {
  shutdown();
  _global_ptr = (AudioManager*)0L;
}

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
//     Function: AudioManager::set_shutdown_func
//       Access: Public, Static
//  Description: register a function that will shutdown the internal
//               audio state
////////////////////////////////////////////////////////////////////
void AudioManager::set_shutdown_func(AudioManager::ShutdownFunc* func) {
  if (_shutdown_func != (AudioManager::ShutdownFunc*)0L)
    audio_cat->error() << "There maybe be more then one audio driver installed"
		       << endl;
  _shutdown_func = func;
  if (_quit == (bool*)0L)
    _quit = new bool(false);
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
//     Function: AudioManager::ns_play (AudioSound)
//       Access: Private
//  Description: get the player off the sound, and start it playing
////////////////////////////////////////////////////////////////////
void AudioManager::ns_play(AudioSound* sound) {
  sound->get_player()->play_sound(sound->get_sound(), sound->get_state());
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::spawned_update
//       Access: static
//  Description: the thread function to call update forever.
////////////////////////////////////////////////////////////////////
void* AudioManager::spawned_update(void* data) {
  bool* flag = (bool*)data;
  while (! (*flag)) {
    AudioManager::update();
    ipc_traits::sleep(0, audio_auto_update_delay);
  }
  *flag = false;
  audio_cat->debug() << "exiting update thread" << endl;
  return (void*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_set_volume (AudioSound)
//       Access: Private
//  Description: get the player off the sound, and set volume on it
////////////////////////////////////////////////////////////////////
void AudioManager::ns_set_volume(AudioSound* sound, int v) {
  sound->get_player()->set_volume(sound->get_state(), v);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_spawn_update
//       Access: Private
//  Description: spawn a thread that calls update every so often
////////////////////////////////////////////////////////////////////
void AudioManager::ns_spawn_update(void) {
  if (_spawned == (thread*)0L) {
    if (_quit == (bool*)0L)
      _quit = new bool(false);
    *_quit = false;
    _spawned = thread::create(spawned_update, _quit, thread::PRIORITY_NORMAL);
  } else {
    audio_cat->error() << "tried to spawn 2 update threads" << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioManager::ns_shutdown
//       Access: Private
//  Description: non-static implementation of shutdown stuff
////////////////////////////////////////////////////////////////////
void AudioManager::ns_shutdown(void) {
  if (_quit != (bool*)0L)
    *_quit = true;
  if (_shutdown_func != (ShutdownFunc*)0L)
    (*_shutdown_func)();
  if (_spawned != (thread*)0L)
    while (*_quit);
  audio_cat->debug() << "update thread has shutdown" << endl;
  delete _quit;
}
