// Filename: audio_pool.cxx
// Created by:  cary (22Sep00)
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

#include "audio_pool.h"
#include "config_audio.h"
#include <config_util.h>

AudioPool* AudioPool::_global_ptr; // static is 0 by default.
typedef pmap<string, AudioPool::SoundLoadFunc*> SoundLoaders;
static SoundLoaders* _sound_loaders; // static is 0 by default.

////////////////////////////////////////////////////////////////////
//     Function: check_sound_loaders
//       Access: Static
//  Description: ensure that the sound loaders map has been initialized
////////////////////////////////////////////////////////////////////
static void 
check_sound_loaders() {
  if (!_sound_loaders) {
    _sound_loaders = new SoundLoaders;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one AudioPool object in the system.
////////////////////////////////////////////////////////////////////
AudioPool* AudioPool::
get_ptr() {
  if (!_global_ptr) {
    _global_ptr = new AudioPool;
  }
  audio_load_loaders();
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_has_sound
//       Access: Private
//  Description: The nonstatic implementation of has_sound().
////////////////////////////////////////////////////////////////////
bool AudioPool::
ns_has_sound(Filename filename) {
  filename.resolve_filename(get_sound_path());

  SoundMap::const_iterator si;
  si = _sounds.find(filename);
  return si != _sounds.end();
}


////////////////////////////////////////////////////////////////////
//     Function: AudioPool::call_sound_loader
//       Access: Private
//  Description: Call the sound loader to load the sound, without
//               looking in the sound pool.
////////////////////////////////////////////////////////////////////
PT(AudioTraits::SoundClass) AudioPool::
call_sound_loader(Filename fileName) {
  if (!fileName.exists()) {
    audio_info("'" << fileName << "' does not exist");
    return 0;
  }
  audio_info("Loading sound " << fileName);
  // Determine which sound loader to use:
  string ext = fileName.get_extension();
  SoundLoaders::const_iterator sli;
  check_sound_loaders();
  sli = _sound_loaders->find(ext);
  if (sli == _sound_loaders->end()) {
    audio_error("no loader available for audio type '" << ext << "'");
    return 0;
  }
  // Call the sound loader:
  PT(AudioTraits::SoundClass) sound=(*((*sli).second))(fileName);
  if (!sound) {
    audio_error("could not load '" << fileName << "'");
  }
  return sound;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_load_sound
//       Access: Private
//  Description: The nonstatic implementation of load_sound().
//
//               First we will search the pool for the sound.
//               If that fails, we will call the loader for the
//               sound, and then add this sound to the pool.
////////////////////////////////////////////////////////////////////
AudioSound* AudioPool::
ns_load_sound(Filename fileName) {
  audio_debug("in AudioPool::ns_load_sound");
  fileName.resolve_filename(get_sound_path());
  audio_debug("resolved fileName is '" << fileName << "'");

  PT(AudioTraits::SoundClass) sound=0;
  // Get the sound, either from the pool or from a loader:
  SoundMap::const_iterator si;
  si = _sounds.find(fileName);
  if (si != _sounds.end()) {
    // ...found the sound in the pool.
    sound = (*si).second;
    audio_debug("sound found in pool (0x" << (void*)sound << ")");
  } else {
    // ...the sound was not found in the cache/pool.
    sound=call_sound_loader(fileName);
    if (sound) {
      // Put it in the pool:
      _sounds[fileName] = sound;
    }
  }
  // Create an AudioSound from the sound:
  AudioSound* audioSound = 0;
  if (sound) {
    audioSound = new AudioSound(sound, 
      sound->get_state(), sound->get_player(),
      sound->get_delstate(), fileName);
  }
  audio_debug("AudioPool: returning 0x" << (void*)audioSound);
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_sound
//       Access: Private
//  Description: The nonstatic implementation of release_sound().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_sound(AudioSound* sound) {
  audio_debug("AudioPool: releasing sound 0x" << (void*)sound);
  string filename = sound->get_name();
  SoundMap::iterator si;
  si = _sounds.find(filename);
  if (si != _sounds.end() && (*si).second == sound->get_sound()) {
    _sounds.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_all_sounds
//       Access: Private
//  Description: The nonstatic implementation of release_all_sounds().
////////////////////////////////////////////////////////////////////
void AudioPool::
ns_release_all_sounds() {
  audio_debug("AudioPool: releasing all sounds");
  _sounds.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::register_sound_loader
//       Access: Public, static
//  Description: A static function to register a function for loading
//               audio sounds.
//               ext: a file name extension (e.g. .mp3 or .wav).
//               func: a function that will be called to load a file
//                     with the extension 'ext'.
////////////////////////////////////////////////////////////////////
void AudioPool::
register_sound_loader(const string& ext,
    AudioPool::SoundLoadFunc* func) {
  check_sound_loaders();
  #ifndef NDEBUG //[
    // Debug check: See if the loader is already registered:
    SoundLoaders::const_iterator sli = _sound_loaders->find(ext);
    if (sli != _sound_loaders->end()) {
      audio_cat->warning() 
        << "attempted to register a loader for audio type '"
        << ext << "' more then once." << endl;
      return;
    }
  #endif //]
  (*_sound_loaders)[ext] = func;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_get_nth_sound_name
//       Access: Public
//  Description: return the name of the nth loaded  sound
////////////////////////////////////////////////////////////////////
string AudioPool::
ns_get_nth_sound_name(int n) const {
  SoundMap::const_iterator i;
  int j;
  for (i=_sounds.begin(), j=0; j<n; ++i, ++j) {
    // the work is being done in the for statement.
  }
  return (*i).first;
}
