// Filename: fmodAudioManager.cxx
// Created by:  cort (January 22, 2003)
// Prior system by: cary
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

#include "pandabase.h"
#ifdef HAVE_FMOD //[

#include "config_audio.h"
#include "config_util.h"
#include "config_express.h"
#include "filename.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "nullAudioSound.h"

#include <algorithm>
#include <cctype>
#include <fmod.h>
#include <iostream>
using std::cerr;

PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() Fmod.");
  return new FmodAudioManager;
}

int FmodAudioManager::_active_managers = 0;

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::FmodAudioManager
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
FmodAudioManager() {
  audio_debug("FmodAudioManager::FmodAudioManager()");
  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);

  _active = audio_active;

  // Initialize FMOD, if this is the first manager created.
  if (_active_managers == 0) {
    do {
      audio_debug("Initializing FMOD for real.");
      float fmod_dll_version = FSOUND_GetVersion();
      if (fmod_dll_version < FMOD_VERSION) {
	audio_error("Wrong FMOD DLL version.  You have "<<fmod_dll_version
		    <<".  You need "<<FMOD_VERSION);
	_is_valid = false;
	break;
      }
      if (FSOUND_Init(44100, 32, 0) == 0) {
	audio_error("Fmod initialization failure.");
	_is_valid = false;
	break;
      }
    }
    while(0);
    _is_valid = true;
  }

  // increment regardless of whether an error has occured -- the
  // destructor will do the right thing.
  ++_active_managers;
  audio_debug("  _active_managers="<<_active_managers);

  if (_is_valid)  {
    assert(is_valid());    
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::~FmodAudioManager
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FmodAudioManager::
~FmodAudioManager() {
  // Be sure to delete associated sounds before deleting the manager!
  nassertv(_soundsOnLoan.empty());
  clear_cache();
  --_active_managers;
  audio_debug("~FmodAudioManager(): "<<_active_managers<<" still active");
  if (_active_managers == 0) {
    audio_debug("Shutting down FMOD");
    FSOUND_Close();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::is_valid
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
is_valid() {
  // verify the cache and LRU list here.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(AudioSound) FmodAudioManager::
get_sound(const string &file_name) {
  audio_debug("FmodAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  if(!is_valid()) {
     audio_debug("invalid FmodAudioManager returning NullSound");
     return new NullAudioSound();
  }

  assert(is_valid());
  Filename path = file_name;

  audio_debug("  resolved file_name is '"<<path<<"'");

  // Get the sound, either from the cache or from disk.
  SoundMap::iterator si = _sounds.find(path);
  SoundCacheEntry *entry = NULL;
  if (si != _sounds.end()) {
    // The sound was found in the cache.
    entry = &(*si).second;
    audio_debug("Sound file '"<<path<<"' found in cache.");
  } else {
    // The sound was not found in the cache.  Load it from disk.
    SoundCacheEntry new_entry;
    new_entry.size = get_file_size(path);
    if (new_entry.size == 0) {
      audio_error("FmodAudioManager::get_file_size failed.");
      return 0;
    }
    new_entry.data = load(path, new_entry.size);
    if (!new_entry.data) {
      audio_error("FmodAudioManager::load failed.");
      return 0;
    }
    new_entry.refcount = 0;
    new_entry.stale = true;

    // Add to the cache
    entry = &new_entry;
    _sounds[path] = new_entry;
  }
  assert(entry != NULL);
  
  // Create an FMOD object from the memory-mapped file.  Here remains
  // one last vestige of special-case MIDI code: apparently, FMOD
  // doesn't like creating streams from memory-mapped MIDI files.
  // They must therefore be streamed from disk every time.  This
  // causes strange things to happen when the same MIDI file is loaded
  // twice, and played simultaneously...so, *don't do that then*.  all
  // I can say is that MIDI support will be significantly improved in
  // FMOD v4.0!
  FSOUND_STREAM *stream = NULL;
  int flags = FSOUND_LOADMEMORY | FSOUND_MPEGACCURATE;
  string os_path = path.to_os_specific();
  string suffix = path.get_extension();
  std::transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
  if (suffix == "mid" || suffix == "rmi" || suffix == "midi") {
    stream = FSOUND_Stream_OpenFile(os_path.c_str(), 0, 0);
  } else {
    stream = FSOUND_Stream_OpenFile((const char*)(entry->data),
				    flags, entry->size);
  }
  if (stream == NULL) {
    audio_error("FmodAudioManager::get_sound failed.");
    return 0;
  }
  inc_refcount(path);

  // determine length of sound
  float length = (float)FSOUND_Stream_GetLengthMs(stream) * 0.001f;

  // Build a new AudioSound from the audio data.
  PT(AudioSound) audioSound = 0;
  PT(FmodAudioSound) fmodAudioSound = new FmodAudioSound(this, stream, path,
							 length);
  fmodAudioSound->set_active(_active);
  _soundsOnLoan.insert(fmodAudioSound);
  audioSound = fmodAudioSound;
  
  audio_debug("  returning 0x" << (void*)audioSound);
  assert(is_valid());
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::uncache_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
uncache_sound(const string& file_name) {
  audio_debug("FmodAudioManager::uncache_sound(\""<<file_name<<"\")");
  Filename path = file_name;
  assert(is_valid());
  SoundMap::iterator itor = _sounds.find(path);
  if (itor == _sounds.end()) {
    audio_error("FmodAudioManager::uncache_sound: no such entry "<<file_name);
    return;
  }

  // Mark the entry as stale -- when its refcount reaches zero, it will
  // be removed from the cache.  If the refcount is already zero, it can be
  // purged right now!
  SoundCacheEntry *entry = &(*itor).second;
  if (entry->refcount == 0) {
    audio_debug("FmodAudioManager::dec_refcount: purging "<<path
		<< " from the cache.");
    delete [] entry->data;
    _sounds.erase(itor);
  } else {
    entry->stale = true;
  }

  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::clear_cache
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
clear_cache() {
  // Mark all cache entries as stale.  Delete those which already have 
  // refcounts of zero.
  SoundMap::iterator itor = _sounds.begin();
  for( ; itor != _sounds.end(); ++itor) {
    SoundCacheEntry *entry = &(*itor).second;
    if (entry->refcount == 0) {
      audio_debug("FmodAudioManager: purging "<< (*itor).first
		  << " from the cache.");
      delete [] entry->data;
      _sounds.erase(itor);
      itor = _sounds.begin();
    } else {
      entry->stale = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_cache_limit(int) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int FmodAudioManager::
get_cache_limit() {
  // intentionally blank.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::release_sound
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
release_sound(FmodAudioSound* audioSound) {
  audio_debug("FmodAudioManager::release_sound(audioSound=\""
      <<audioSound->get_name()<<"\")");
  dec_refcount(audioSound->get_name());
  _soundsOnLoan.erase(audioSound);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_volume(float) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioManager::
get_volume() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::set_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
set_active(bool active) {
  audio_debug("FmodAudioManager::set_active(flag="<<active<<")");
  if (_active!=active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_soundsOnLoan.begin();
    for (; i!=_soundsOnLoan.end(); ++i) {
      (**i).set_active(_active);
    }
  }
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool FmodAudioManager::
get_active() {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::stop_all_sounds
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
stop_all_sounds(void) {
  audio_debug("FmodAudioManager::stop_all_sounds()");
  AudioSet::iterator i=_soundsOnLoan.begin();
  for (; i!=_soundsOnLoan.end(); ++i) {
    if((**i).status()==AudioSound::PLAYING)
      (**i).stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::inc_refcount
//       Access: Protected
//  Description: Increments the refcount of a file's cache entry.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
inc_refcount(const string& file_name) {
  Filename path = file_name;
  SoundMap::iterator itor = _sounds.find(path.to_os_specific());
  if (itor != _sounds.end()) {
    SoundCacheEntry *entry = &(*itor).second;
    entry->refcount++;
    entry->stale = false; // definitely not stale!
    audio_debug("FmodAudioManager: "<<path<<" has a refcount of "
		<< entry->refcount);
  } else {
    audio_debug("FmodAudioManager::inc_refcount: no such file "<<path);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::dec_refcount
//       Access: Protected
//  Description: Decrements the refcount of a file's cache entry. If
//               the refcount reaches zero and the entry is stale, it
//               will be removed from the cache.
////////////////////////////////////////////////////////////////////
void FmodAudioManager::
dec_refcount(const string& file_name) {
  Filename path = file_name;
  SoundMap::iterator itor = _sounds.find(path.to_os_specific());
  if (itor != _sounds.end()) {
    SoundCacheEntry *entry = &(*itor).second;
    entry->refcount--;
    audio_debug("FmodAudioManager: "<<path<<" has a refcount of "
		<< entry->refcount);
    if (entry->refcount == 0 && entry->stale) {
      audio_debug("FmodAudioManager: purging "<<path<< " from the cache.");
      delete [] entry->data;
      _sounds.erase(itor);
    }
  } else {
    audio_debug("FmodAudioManager::dec_refcount: no such file "<<path);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::load
//       Access: Private
//  Description: Loads the specified file into memory.
//               Returns NULL if an error occurs.
////////////////////////////////////////////////////////////////////
void* FmodAudioManager::
load(const Filename& filename, const size_t size) const {
  // Check file type (based on filename suffix
  string suffix = filename.get_extension();
  std::transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
  bool bSupported = false;
  if (suffix == "wav" || suffix == "mp3" || suffix == "mid"
      || suffix == "rmi" || suffix == "midi") {
    bSupported = true;
  }
  if (!bSupported) {
    audio_error("FmodAudioManager::load: "<<filename
		<<" is not a supported file format.");
    audio_error("Supported formats are: WAV, MP3, MIDI");
    return NULL;
  }

  // open the file.
  string os_filename = filename.to_os_specific();
  FILE *audioFile = fopen(os_filename.c_str(), "rb");
  if (!audioFile) {
    audio_error("File "<<filename<<" does not exist.");
    return NULL;
  }
  
  // Read the entire file into memory.
  char *buffer = new char[size];
  if (buffer == NULL) {
    audio_error("out-of-memory error while loading "<<filename);
    fclose(audioFile);
    return NULL;
  }
  long bytes_read = fread(buffer, size, 1, audioFile);
  if (bytes_read != 1) {
    audio_error("Read error while loading "<<filename);
    fclose(audioFile);
    delete [] buffer;
    return NULL;
  }

  fclose(audioFile);
  return buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioManager::get_file_size
//       Access: Private
//  Description: Calculates the size of the given file, in bytes.
////////////////////////////////////////////////////////////////////
size_t FmodAudioManager::
get_file_size(const Filename& filename) const {
  // open the file.
  string os_filename = filename.to_os_specific();
  FILE *audioFile = fopen(os_filename.c_str(), "rb");
  if (!audioFile) {
    audio_error("File "<<filename<<" does not exist.");
    return 0;
  }
  
  // Read the entire file into memory.
  if (fseek(audioFile, 0, SEEK_END) != 0) {
    audio_error("Seek error while loading "<<filename);
    fclose(audioFile);
    return 0;
  }
  size_t file_size = ftell(audioFile);
  fclose(audioFile);
  return file_size;
}

#endif //]
