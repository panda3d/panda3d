// Filename: audio_pool.cxx
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_pool.h"
#include "config_audio.h"
#include <config_util.h>

AudioPool* AudioPool::_global_ptr = (AudioPool*)0L;
typedef map<string, AudioPool::SampleLoadFunc*> SampleLoaders;
SampleLoaders* _sample_loaders = (SampleLoaders*)0L;
typedef map<string, AudioPool::MusicLoadFunc*> MusicLoaders;
MusicLoaders* _music_loaders = (MusicLoaders*)0L;

////////////////////////////////////////////////////////////////////
//     Function: check_sample_loaders
//       Access: Static
//  Description: ensure that the sample loaders map has been initialized
////////////////////////////////////////////////////////////////////
static void check_sample_loaders(void) {
  if (_sample_loaders == (SampleLoaders*)0L)
    _sample_loaders = new SampleLoaders;
}

////////////////////////////////////////////////////////////////////
//     Function: check_music_loaders
//       Access: Static
//  Description: ensure that the music loaders map has been initialized
////////////////////////////////////////////////////////////////////
static void check_music_loaders(void) {
  if (_music_loaders == (MusicLoaders*)0L)
    _music_loaders = new MusicLoaders;
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
//     Function: AudioPool::ns_has_sample
//       Access: Private
//  Description: The nonstatic implementation of has_sample().
////////////////////////////////////////////////////////////////////
bool AudioPool::ns_has_sample(Filename filename) {
  filename.resolve_filename(get_sound_path());

  SampleMap::const_iterator si;
  si = _samples.find(filename);
  if (si != _samples.end()) {
    // this sample was previously loaded
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_load_sample
//       Access: Private
//  Description: The nonstatic implementation of load_sample().
////////////////////////////////////////////////////////////////////
AudioSample* AudioPool::ns_load_sample(Filename filename) {
  filename.resolve_filename(get_sound_path());

  SampleMap::const_iterator si;
  si = _samples.find(filename);
  if (si != _samples.end()) {
    // this sample was previously loaded
    return (*si).second;
  }
  audio_cat.info() << "Loading sample " << filename << "\n";
  AudioTraits::SampleClass* sample = (AudioTraits::SampleClass*)0L;
  AudioTraits::PlayerClass* player = (AudioTraits::PlayerClass*)0L;
  AudioTraits::DeleteSampleFunc* destroy = (AudioTraits::DeleteSampleFunc*)0L;
  string ext = filename.get_extension();
  SampleLoaders::const_iterator sli;
  check_sample_loaders();
  sli = _sample_loaders->find(ext);
  if (sli == _sample_loaders->end()) {
    audio_cat->error() << "no loader available for audio type '" << ext
		       << "'" << endl;
    return (AudioSample*)0L;
  }
  (*((*sli).second))(&sample, &player, &destroy, filename);
  if ((sample == (AudioTraits::SampleClass*)0L) ||
      (player == (AudioTraits::PlayerClass*)0L) ||
      (destroy == (AudioTraits::DeleteSampleFunc*)0L)) {
    audio_cat->error() << "could not load '" << filename << "'" << endl;
    return (AudioSample*)0L;
  }
  PT(AudioSample) the_sample = new AudioSample(sample,
					       (AudioTraits::PlayingClass*)0L,
					       player, destroy, filename);
  _samples[filename] = the_sample;
  return the_sample;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_sample
//       Access: Private
//  Description: The nonstatic implementation of release_sample().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_sample(AudioSample* sample) {
  string filename = sample->get_name();
  SampleMap::iterator si;
  si = _samples.find(filename);
  if (si != _samples.end() && (*si).second == sample) {
    _samples.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_all_samples
//       Access: Private
//  Description: The nonstatic implementation of release_all_samples().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_all_samples(void) {
  _samples.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::register_sample_loader
//       Access: Public, static
//  Description: A static function to register a function for loading
//               audio samples.
////////////////////////////////////////////////////////////////////
void AudioPool::register_sample_loader(const string& ext,
				       AudioPool::SampleLoadFunc* func) {
  SampleLoaders::const_iterator sli;
  check_sample_loaders();
  sli = _sample_loaders->find(ext);
  if (sli != _sample_loaders->end()) {
    audio_cat->warning() << "attempted to register a loader for audio type '"
			 << ext << "' more then once." << endl;
    return;
  }
  (*_sample_loaders)[ext] = func;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_has_music
//       Access: Private
//  Description: The nonstatic implementation of has_music().
////////////////////////////////////////////////////////////////////
bool AudioPool::ns_has_music(Filename filename) {
  filename.resolve_filename(get_sound_path());

  MusicMap::const_iterator si;
  si = _music.find(filename);
  if (si != _music.end()) {
    // this music was previously loaded
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_load_music
//       Access: Private
//  Description: The nonstatic implementation of load_music().
////////////////////////////////////////////////////////////////////
AudioMusic* AudioPool::ns_load_music(Filename filename) {
  filename.resolve_filename(get_sound_path());

  MusicMap::const_iterator si;
  si = _music.find(filename);
  if (si != _music.end()) {
    // this sample was previously loaded
    return (*si).second;
  }
  audio_cat.info() << "Loading music " << filename << "\n";
  AudioTraits::MusicClass* music = (AudioTraits::MusicClass*)0L;
  AudioTraits::PlayerClass* player = (AudioTraits::PlayerClass*)0L;
  AudioTraits::DeleteMusicFunc* destroy = (AudioTraits::DeleteMusicFunc*)0L;
  string ext = filename.get_extension();
  MusicLoaders::const_iterator sli;
  check_music_loaders();
  sli = _music_loaders->find(ext);
  if (sli == _music_loaders->end()) {
    audio_cat->error() << "no loader available for audio type '" << ext
		       << "'" << endl;
    return (AudioMusic*)0L;
  }
  (*((*sli).second))(&music, &player, &destroy, filename);
  if ((music == (AudioTraits::MusicClass*)0L) ||
      (player == (AudioTraits::PlayerClass*)0L) ||
      (destroy == (AudioTraits::DeleteMusicFunc*)0L)) {
    audio_cat->error() << "could not load '" << filename << "'" << endl;
    return (AudioMusic*)0L;
  }
  PT(AudioMusic) the_music = new AudioMusic(music,
					    (AudioTraits::PlayingClass*)0L,
					    player, destroy, filename);
  _music[filename] = the_music;
  return the_music;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_music
//       Access: Private
//  Description: The nonstatic implementation of release_music().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_music(AudioMusic* music) {
  string filename = music->get_name();
  MusicMap::iterator si;
  si = _music.find(filename);
  if (si != _music.end() && (*si).second == music) {
    _music.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::ns_release_all_music
//       Access: Private
//  Description: The nonstatic implementation of release_all_music().
////////////////////////////////////////////////////////////////////
void AudioPool::ns_release_all_music(void) {
  _music.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::register_music_loader
//       Access: Public, static
//  Description: A static function to register a function for loading
//               audio music.
////////////////////////////////////////////////////////////////////
void AudioPool::register_music_loader(const string& ext,
				      AudioPool::MusicLoadFunc* func) {
  MusicLoaders::const_iterator sli;
  check_music_loaders();
  sli = _music_loaders->find(ext);
  if (sli != _music_loaders->end()) {
    audio_cat->warning() << "attempted to register a loader for audio type '"
			 << ext << "' more then once." << endl;
    return;
  }
  (*_music_loaders)[ext] = func;
}
