// Filename: audio_pool.cxx
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_pool.h"
#include "config_audio.h"
#include <config_util.h>

AudioPool* AudioPool::_global_ptr = (AudioPool*)0L;
typedef map<string, AudioPool::SoundLoadFunc*> SoundLoaders;
SoundLoaders* _sound_loaders = (SoundLoaders*)0L;

////////////////////////////////////////////////////////////////////
//     Function: check_sound_loaders
//       Access: Static
//  Description: ensure that the sound loaders map has been initialized
////////////////////////////////////////////////////////////////////
static void check_sound_loaders(void) {
  if (_sound_loaders == (SoundLoaders*)0L)
    _sound_loaders = new SoundLoaders;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one AudioPool object in the system.
////////////////////////////////////////////////////////////////////
AudioPool* AudioPool::get_ptr(void) {
  if (_global_ptr == (AudioPool*)0L)
    _global_ptr = new AudioPool;
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_has_sound
//       Access: Private
//  Description: The nonstatic implementation of has_sound().
////////////////////////////////////////////////////////////////////
bool AudioPool::ns_has_sound(Filename filename) {
  filename.resolve_filename(get_sound_path());

  SoundMap::const_iterator si;
  si = _sounds.find(filename);
  if (si != _sounds.end()) {
    // this sound was previously loaded
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_load_sound
//       Access: Private
//  Description: The nonstatic implementation of load_sound().
////////////////////////////////////////////////////////////////////
AudioSound* AudioPool::ns_load_sound(Filename filename) {
  filename.resolve_filename(get_sound_path());

  SoundMap::const_iterator si;
  si = _sounds.find(filename);
  if (si != _sounds.end()) {
    // this sound was previously loaded
    AudioTraits::SoundClass* sc = (*si).second;
    return new AudioSound(sc, sc->get_state(), sc->get_player(),
			  sc->get_delstate(), filename);
  }
  if (!filename.exists()) {
    audio_cat.info() << "'" << filename << "' does not exist" << endl;
    return (AudioSound*)0L;
  }
  audio_cat.info() << "Loading sound " << filename << endl;
  string ext = filename.get_extension();
  SoundLoaders::const_iterator sli;
  check_sound_loaders();
  sli = _sound_loaders->find(ext);
  if (sli == _sound_loaders->end()) {
    audio_cat->error() << "no loader available for audio type '" << ext
		       << "'" << endl;
    return (AudioSound*)0L;
  }
  AudioTraits::SoundClass* sound = (*((*sli).second))(filename);
  if (sound == (AudioTraits::SoundClass*)0L) {
    audio_cat->error() << "could not load '" << filename << "'" << endl;
    return (AudioSound*)0L;
  }
  PT(AudioSound) the_sound = new AudioSound(sound, sound->get_state(),
					    sound->get_player(),
					    sound->get_delstate(), filename);
  _sounds[filename] = sound;
  return the_sound;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_sound
//       Access: Private
//  Description: The nonstatic implementation of release_sound().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_sound(AudioSound* sound) {
  string filename = sound->get_name();
  SoundMap::iterator si;
  si = _sounds.find(filename);
  if (si != _sounds.end() && (*si).second == sound->get_sound()) {
    AudioTraits::SoundClass* sc = (*si).second;
    (*(sc->get_destroy()))(sc);
    _sounds.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_all_sounds
//       Access: Private
//  Description: The nonstatic implementation of release_all_sounds().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_all_sounds(void) {
  for (SoundMap::iterator i=_sounds.begin(); i!=_sounds.end(); ++i) {
    AudioTraits::SoundClass* sc = (*i).second;
    (*(sc->get_destroy()))(sc);
  }
  _sounds.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::register_sound_loader
//       Access: Public, static
//  Description: A static function to register a function for loading
//               audio sounds.
////////////////////////////////////////////////////////////////////
void AudioPool::register_sound_loader(const string& ext,
				       AudioPool::SoundLoadFunc* func) {
  SoundLoaders::const_iterator sli;
  check_sound_loaders();
  sli = _sound_loaders->find(ext);
  if (sli != _sound_loaders->end()) {
    audio_cat->warning() << "attempted to register a loader for audio type '"
			 << ext << "' more then once." << endl;
    return;
  }
  (*_sound_loaders)[ext] = func;
}
