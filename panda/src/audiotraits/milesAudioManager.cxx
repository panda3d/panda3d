// Filename: milesAudioManager.cxx
// Created by:  skyler (June 6, 2001)
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
#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioManager.h"
#include "config_audio.h"
#include "config_util.h"
#include <algorithm>

int MilesAudioManager::_active_managers;
HDLSFILEID MilesAudioManager::_dls_field;

PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManger()");
  return new MilesAudioManager();
}


////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::MilesAudioManager
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::
MilesAudioManager() {
  audio_debug("MilesAudioManager::MilesAudioManager()");
  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);
  _active = audio_active;
  _volume = audio_volume;
  _cache_limit = audio_cache_limit;
  _is_valid = true;
  if (!_active_managers) {
    S32 use_digital=(audio_play_wave || audio_play_mp3)?1:0;
    S32 use_MIDI=(audio_play_midi)?1:0;
    if (audio_play_midi && audio_software_midi) {
      use_MIDI=AIL_QUICK_DLS_ONLY;
    }
    audio_debug("  use_digital="<<use_digital);
    audio_debug("  use_MIDI="<<use_MIDI);
    audio_debug("  audio_output_rate="<<audio_output_rate);
    audio_debug("  audio_output_bits="<<audio_output_bits);
    audio_debug("  audio_output_channels="<<audio_output_channels);
    audio_debug("  audio_software_midi="<<audio_software_midi);
    #ifndef NDEBUG //[
      char version[8];
      AIL_MSS_version(version, 8);
      audio_debug("  Mss32.dll Version: "<<version);
    #endif //]
    if (AIL_quick_startup(use_digital, 
        use_MIDI, audio_output_rate, 
        audio_output_bits, audio_output_channels)) {
      if (audio_software_midi) {
        // Load the downloadable sounds file:
        HDLSDEVICE dls;
        AIL_quick_handles(0, 0, &dls);
        nassertv(audio_dls_file);
        nassertv(!_dls_field);
        if (audio_dls_file->empty()) {
          audio_debug("  using default audio_dls_file");
          get_gm_file_path(*audio_dls_file);
        }
        audio_debug("  audio_dls_file=\""<<*audio_dls_file<<"\"");
        _dls_field=AIL_DLS_load_file(dls, audio_dls_file->c_str(), 0);
        if (!_dls_field) {
          audio_error("  AIL_DLS_load_file() failed, \""<<AIL_last_error()
              <<"\" Switching to hardware midi");
          AIL_quick_shutdown();
          if (!AIL_quick_startup(use_digital, 1, audio_output_rate, 
              audio_output_bits, audio_output_channels)) {
            audio_error("  midi hardware startup failed, "<<AIL_last_error());
            _is_valid = false;
          }
        } else {
          audio_debug("  using Miles software midi");
        }
      }
    } else {
      audio_debug("  AIL_quick_startup failed: "<<AIL_last_error());
      _is_valid = false;
    }
  }
  // We increment _active_managers regardless of possible errors above.
  // The miles shutdown call will do the right thing when it's called,
  // either way.
  ++_active_managers;
  audio_debug("  _active_managers="<<_active_managers);
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::~MilesAudioManager
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::
~MilesAudioManager() {
  audio_debug("MilesAudioManager::~MilesAudioManager()");
  // Be sure to delete associated sounds before deleting the manager:
  nassertv(_soundsOnLoan.empty());
  clear_cache();
  --_active_managers;
  audio_debug("  _active_managers="<<_active_managers);
  if (!_active_managers) {
    if (audio_software_midi) {
      HDLSDEVICE dls;
      AIL_quick_handles(0, 0, &dls);
      AIL_DLS_unload(dls, _dls_field);
      #ifndef NDEBUG //[
        // Clear _dls_field in debug version (for assert in ctor):
        _dls_field=0;
      #endif //]
    }
    AIL_quick_shutdown();
    audio_debug("  AIL_quick_shutdown()");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::is_valid
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
is_valid() {
  bool check=true;
  if (_sounds.size() != _lru.size()) {
    audio_debug("--sizes--");
    check=false;
  } else {
    LRU::const_iterator i=_lru.begin();
    for (; i != _lru.end(); ++i) {
      SoundMap::const_iterator smi=_sounds.find(**i);
      if (smi == _sounds.end()) {
        audio_debug("--"<<**i<<"--");
        check=false;
        break;
      }
    }
  }
  return _is_valid && check;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::load
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
HAUDIO MilesAudioManager::
load(Filename file_name) {
  string stmp = file_name.to_os_specific();
  audio_debug("  \"" << stmp << "\"");
  HAUDIO audio = AIL_quick_load(stmp.c_str());
  if (!audio) {
    audio_debug("  MilesAudioManager::load failed "<<AIL_last_error());
  }
  return audio;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(AudioSound) MilesAudioManager::
get_sound(const string& file_name) {
  audio_debug("MilesAudioManager::get_sound(file_name=\""<<file_name<<"\")");
  assert(is_valid());
  Filename path = file_name;
  path.resolve_filename(get_sound_path());
  audio_debug("  resolved file_name is '"<<path<<"'");

  HAUDIO audio=0;
  // Get the sound, either from the cache or from a loader:
  SoundMap::const_iterator si=_sounds.find(path);
  if (si != _sounds.end()) {
    // ...found the sound in the cache.
    audio = (*si).second;
    audio_debug("  sound found in pool 0x" << (void*)audio);
  } else {
    // ...the sound was not found in the cache/pool.
    audio=load(path);
    if (audio) {
      while (_sounds.size() >= _cache_limit) {
        uncache_a_sound();
      }
      // Put it in the pool:
      // The following is roughly like: _sounds[path] = audio;
      // But, it gives us an iterator into the map.
      pair<SoundMap::const_iterator, bool> ib 
          =_sounds.insert(pair<string, HAUDIO>(path, audio));
      if (!ib.second) {
        // The insert failed.
        audio_debug("  failed map insert of "<<path);
        assert(is_valid());
        return 0;
      }
      // Set si, so that we can get a reference to the path 
      // for the MilesAudioSound.
      si=ib.first;
    }
  }
  // Create an AudioSound from the sound:
  PT(AudioSound) audioSound = 0;
  if (audio) {
    most_recently_used((*si).first);
    PT(MilesAudioSound) milesAudioSound
        =new MilesAudioSound(this, audio, (*si).first);
    nassertr(milesAudioSound, 0);
    milesAudioSound->set_active(_active);
    _soundsOnLoan.insert(milesAudioSound);
    audioSound=milesAudioSound;
  }
  audio_debug("  returning 0x" << (void*)audioSound);
  assert(is_valid());
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::uncache_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
uncache_sound(const string& file_name) {
  audio_debug("MilesAudioManager::uncache_sound(file_name=\""
      <<file_name<<"\")");
  assert(is_valid());
  Filename path = file_name;
  path.resolve_filename(get_sound_path());
  audio_debug("  path=\""<<path<<"\"");
  SoundMap::iterator i=_sounds.find(path);
  if (i != _sounds.end()) {
    assert(_lru.size()>0);
    LRU::iterator lru_i=find(_lru.begin(), _lru.end(), &(i->first));
    assert(lru_i != _lru.end());
    _lru.erase(lru_i);
    AIL_quick_unload(i->second);
    _sounds.erase(i);
  }
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::uncache_a_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
uncache_a_sound() {
  audio_debug("MilesAudioManager::uncache_a_sound()");
  assert(is_valid());
  // uncache least recently used:
  assert(_lru.size()>0);
  LRU::reference path=_lru.front();
  SoundMap::iterator i = _sounds.find(*path);
  assert(i != _sounds.end());
  _lru.pop_front();
  
  if (i != _sounds.end()) {
    audio_debug("  uncaching \""<<i->first<<"\"");
    AIL_quick_unload(i->second);
    _sounds.erase(i);
  }
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::most_recently_used
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
most_recently_used(const string& path) {
  audio_debug("MilesAudioManager::most_recently_used(path=\""
      <<path<<"\")");
  LRU::iterator i=find(_lru.begin(), _lru.end(), &path);
  if (i != _lru.end()) {
    _lru.erase(i);
  }
  // At this point, path should not exist in the _lru:
  assert(find(_lru.begin(), _lru.end(), &path) == _lru.end());
  _lru.push_back(&path);
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::clear_cache
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
clear_cache() {
  audio_debug("MilesAudioManager::clear_cache()");
  assert(is_valid());
  SoundMap::iterator i=_sounds.begin();
  for (; i!=_sounds.end(); ++i) {
    AIL_quick_unload(i->second);
  }
  _sounds.clear();
  _lru.clear();
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_cache_limit(int count) {
  audio_debug("MilesAudioManager::set_cache_limit(count="
      <<count<<")");
  assert(is_valid());
  while (_lru.size() > count) {
    uncache_a_sound();
  }
  _cache_limit=count;
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int MilesAudioManager::
get_cache_limit() {
  audio_debug("MilesAudioManager::get_cache_limit() returning "
      <<_cache_limit);
  return _cache_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::release_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
release_sound(MilesAudioSound* audioSound) {
  audio_debug("MilesAudioManager::release_sound(audioSound=\""
      <<audioSound->get_name()<<"\")");
  _soundsOnLoan.erase(audioSound);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_volume
//       Access: Public
//  Description: set the overall volume
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_volume(float volume) {
  audio_debug("MilesAudioManager::set_volume(volume="<<volume<<")");
  if (_volume!=volume) {
    _volume = volume;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_soundsOnLoan.begin();
    for (; i!=_soundsOnLoan.end(); ++i) {
      (**i).set_volume((**i).get_volume());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_volume
//       Access: Public
//  Description: get the overall volume
////////////////////////////////////////////////////////////////////
float MilesAudioManager::
get_volume() {
  audio_debug("MilesAudioManager::get_volume() returning "<<_volume);
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_active
//       Access: Public
//  Description: turn on/off
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_active(bool active) {
  audio_debug("MilesAudioManager::set_active(flag="<<active<<")");
  if (_active!=active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_soundsOnLoan.begin();
    for (; i!=_soundsOnLoan.end(); ++i) {
      (**i).set_active(_active);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
get_active() {
  audio_debug("MilesAudioManager::get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_registry_entry
//       Access: private
//  Description: Combine base\\subKeyname\\keyName to get
//               'result' from the Windows registry.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
get_registry_entry(HKEY base, const char* subKeyName, 
    const char* keyName, string& result) {
  // Create a key to access the registry:
  HKEY key;
  long r=RegCreateKeyEx(base, subKeyName, 0, "", 
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL);
  if (r==ERROR_SUCCESS) {
    DWORD len=0;
    // Read the size of the value at keyName:
    r=RegQueryValueEx(key, keyName, 0, 0, 0, &len);
    if (r==ERROR_SUCCESS) {
      char* src = new char[len];
      DWORD type;
      // Read the value at keyName:
      r=RegQueryValueEx(key, keyName, 0, &type, (unsigned char*)src, &len);
      if (r==ERROR_SUCCESS) {
        if (type==REG_EXPAND_SZ) {
          // Find the size of the expanded string:
          DWORD destSize=ExpandEnvironmentStrings(src, 0, 0);
          // Get a destination buffer of that size:
          char* dest = new char[destSize];
          // Do the expansion:
          ExpandEnvironmentStrings(src, dest, destSize);
          // Propagate the result:
          result=dest;
          delete [] dest;
        } else if (type==REG_SZ) {
          result=src;
        } else {
          audio_error("MilesAudioManager::get_gm_file_path() Unknown reg key type.");
        }
      }
      delete [] src;
    }
    RegCloseKey(key);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_gm_file_path
//       Access: private
//  Description: Get path of downloadable sound midi instruments file.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
get_gm_file_path(string& result) {
  get_registry_entry(HKEY_LOCAL_MACHINE, 
      "SOFTWARE\\Microsoft\\DirectMusic", "GMFilePath", result);
  audio_debug("MilesAudioManager::get_gm_file_path() result out=\""<<result<<"\"");
}


#endif //]
