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
#include "config_express.h"
#include "virtualFileSystem.h"
#include "nullAudioSound.h"
#include <algorithm>

int MilesAudioManager::_active_managers = 0;
HDLSFILEID MilesAudioManager::_dls_field = NULL;
bool bMilesShutdownCalled = false;
bool bMilesShutdownAtExitRegistered = false;

PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() Miles.");
  return new MilesAudioManager();
}

void fMilesShutdown(void) {
    if(bMilesShutdownCalled)
      return;

    bMilesShutdownCalled = true;

    if (MilesAudioManager::_dls_field!=NULL) {
      HDLSDEVICE dls= NULL;
      AIL_quick_handles(0, 0, &dls);
      if(dls!=NULL) {
          AIL_DLS_unload(dls,MilesAudioManager::_dls_field);
      }

      #ifndef NDEBUG //[
        // Clear _dls_field in debug version (for assert in ctor):
        MilesAudioManager::_dls_field = NULL;  
      #endif //]
    }


   #define SHUTDOWN_HACK
   
   // if python crashes, the midi notes are left on,
   // so we need to turn them off.  Unfortunately
   // in Miles 6.5, AIL_quick_shutdown() crashes
   // when it's called from atexit after a python crash, probably
   // because mss32.dll has already been unloaded
   // workaround: use these internal values in the miles struct
   //             to reset the win32 midi ourselves
   
   #ifdef SHUTDOWN_HACK
    HMDIDRIVER hMid=NULL;
    AIL_quick_handles(0, &hMid, 0);
    if ((hMid!=NULL) && (hMid->deviceid != MIDI_NULL_DRIVER) && (hMid->hMidiOut != NULL)) {
      midiOutReset(hMid->hMidiOut);
      midiOutClose(hMid->hMidiOut);
    }
   #else
    audio_debug("  AIL_quick_shutdown()");
    AIL_quick_shutdown();
   #endif
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
  _bHasMidiSounds = false;
  if (_active_managers==0) {
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
          get_gm_file_path(*audio_dls_file);
          // we need more dbg info in logs, so bumping the msgs from 'debug' status to 'info' status
          audio_info("  using default audio_dls_file: "<< (*audio_dls_file) );
        }

        audio_debug("  audio_dls_file=\""<<*audio_dls_file<<"\"");

        // note: if AIL_DLS_load_file is not done, midi fails to play on some machines.
        _dls_field=AIL_DLS_load_file(dls, audio_dls_file->c_str(), 0);
        if (!_dls_field) {
          audio_error("  AIL_DLS_load_file() failed, \""<<AIL_last_error() <<"\" Switching to hardware midi");
          AIL_quick_shutdown();
          if (!AIL_quick_startup(use_digital, 1, audio_output_rate,
              audio_output_bits, audio_output_channels)) {
            audio_error("  midi hardware startup failed, "<<AIL_last_error());
            _is_valid = false;
          }
        } else {
          audio_info("  using Miles software midi");
        }
      } else {
          audio_info("  using Miles hardware midi");
      }

      if (use_vfs) {
        AIL_set_file_callbacks(vfs_open_callback,
                               vfs_close_callback,
                               vfs_seek_callback,
                               vfs_read_callback);
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

  if (_is_valid)  {
    assert(is_valid());

    if(!bMilesShutdownAtExitRegistered) {
       bMilesShutdownAtExitRegistered = true;
       atexit(fMilesShutdown);
    }
  }
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
  if (_active_managers==0) {
    if (audio_software_midi) {
      HDLSDEVICE dls;
      AIL_quick_handles(0, 0, &dls);
      AIL_DLS_unload(dls, _dls_field);
      #ifndef NDEBUG //[
        // Clear _dls_field in debug version (for assert in ctor):
        _dls_field=0;
      #endif //]
    }
    audio_debug("  AIL_quick_shutdown()");
    AIL_quick_shutdown();
    bMilesShutdownCalled = true;
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
  HAUDIO audio;

  if (use_vfs) {
    audio = AIL_quick_load(file_name.c_str());

  } else {
    string stmp = file_name.to_os_specific();
    audio_debug("  \"" << stmp << "\"");
    audio = AIL_quick_load(stmp.c_str());
  }
   
  if (!audio) {
    audio_error("  MilesAudioManager::load failed "<< AIL_last_error());
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

  if(!is_valid()) {
     audio_debug("invalid MilesAudioManager returning NullSound");
     return get_null_sound();
  }

  assert(is_valid());
  Filename path = file_name;

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(path, get_sound_path());
  } else {
    path.resolve_filename(get_sound_path());
  }

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
      while (_sounds.size() >= (unsigned int)_cache_limit) {
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
        return get_null_sound();
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

  _bHasMidiSounds |= (file_name.find(".mid")!=string::npos);
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

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(path, get_sound_path());
  } else {
    path.resolve_filename(get_sound_path());
  }

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
  if (_is_valid) { assert(is_valid()); }
  SoundMap::iterator i=_sounds.begin();
  for (; i!=_sounds.end(); ++i) {
    AIL_quick_unload(i->second);
  }
  _sounds.clear();
  _lru.clear();
  if (_is_valid) { assert(is_valid()); }
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
  while (_lru.size() > (unsigned int) count) {
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

void MilesAudioManager::
stop_all_sounds(void) {
  audio_debug("MilesAudioManager::stop_all_sounds()");
  AudioSet::iterator i=_soundsOnLoan.begin();
  for (; i!=_soundsOnLoan.end(); ++i) {
      if((**i).status()==AudioSound::PLAYING)
          (**i).stop();
  }

  if(_bHasMidiSounds) {
      forceMidiReset();
  }
}

void MilesAudioManager::
forceMidiReset(void) {
    if(!miles_audio_force_midi_reset) {
        audio_debug("MilesAudioManager::skipping forceMidiReset");  
        return;
    }

    audio_debug("MilesAudioManager::ForceMidiReset");

    // sometimes Miles seems to leave midi notes hanging, even after stop is called,
    // so perform an explicit reset using winMM.dll calls, just to ensure silence.
    HMDIDRIVER hMid=NULL;
    AIL_quick_handles(0, &hMid, 0);
    if ((hMid!=NULL) && (hMid->deviceid != MIDI_NULL_DRIVER) && (hMid->hMidiOut != NULL)) {
        audio_debug("MilesAudioManager::calling midiOutReset");
        midiOutReset(hMid->hMidiOut);
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

    if((!_active) && _bHasMidiSounds) {
        forceMidiReset();
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
bool MilesAudioManager::
get_registry_entry(HKEY base, const char* subKeyName,
    const char* keyName, string& result) {
  // Open the key to access the registry:
  HKEY key;
  long r=RegOpenKeyEx(base, subKeyName, 0, KEY_QUERY_VALUE, &key);
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
          audio_error("MilesAudioManager::get_reg_entry(): Unknown key type.");
        }
      }
      delete [] src;
    }
    RegCloseKey(key);
  }

  return (r==ERROR_SUCCESS);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_gm_file_path
//       Access: private
//  Description: Get path of downloadable sound midi instruments file.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
get_gm_file_path(string& result) {
  if(!get_registry_entry(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\DirectMusic", "GMFilePath", result)) {
          char sysdir[MAX_PATH+1];
          GetSystemDirectory(sysdir,MAX_PATH+1);
          result = sysdir;
          result.append("\\drivers\\gm.dls");
  }

  audio_debug("MilesAudioManager::get_gm_file_path() result out=\""<<result<<"\"");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::vfs_open_callback
//       Access: Private, Static
//  Description: A Miles callback to open a file for reading from the
//               VFS system.
////////////////////////////////////////////////////////////////////
U32 MilesAudioManager::
vfs_open_callback(const char *filename, U32 *file_handle) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *istr = vfs->open_read_file(filename);
  if (istr == (istream *)NULL) {
    // Unable to open.
    milesAudio_cat.warning()
      << "Unable to open " << filename << "\n";
    *file_handle = 0;
    return 0;
  }

  // Successfully opened.  Now we should return a U32 that we can
  // map back into this istream pointer later.  Strictly speaking,
  // we should allocate a table of istream pointers and assign each
  // one a unique number, but for now we'll cheat because we know
  // that the Miles code (presently) only runs on Win32, which
  // always has 32-bit pointers.
  *file_handle = (U32)istr;
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::vfs_read_callback
//       Access: Private, Static
//  Description: A Miles callback to read data from a file opened via
//               vfs_open_callback().
////////////////////////////////////////////////////////////////////
U32 MilesAudioManager::
vfs_read_callback(U32 file_handle, void *buffer, U32 bytes) {
  if (file_handle == 0) {
    // File was not opened.
    return 0;
  }
  istream *istr = (istream *)file_handle;
  istr->read((char *)buffer, bytes);
  size_t bytes_read = istr->gcount();

  return bytes_read;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::vfs_seek_callback
//       Access: Private, Static
//  Description: A Miles callback to seek within a file opened via
//               vfs_open_callback().
////////////////////////////////////////////////////////////////////
S32 MilesAudioManager::
vfs_seek_callback(U32 file_handle, S32 offset, U32 type) {
  if (file_handle == 0) {
    // File was not opened.
    return 0;
  }
  istream *istr = (istream *)file_handle;

  ios::seekdir dir = ios::beg;
  switch (type) {
  case AIL_FILE_SEEK_BEGIN:
    dir = ios::beg;
    break;
  case AIL_FILE_SEEK_CURRENT:
    dir = ios::cur;
    break;
  case AIL_FILE_SEEK_END:
    dir = ios::end;
    break;
  }

  istr->seekg(offset, dir);
  return istr->tellg();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::vfs_close_callback
//       Access: Private, Static
//  Description: A Miles callback to close a file opened via
//               vfs_open_callback().
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
vfs_close_callback(U32 file_handle) {
  if (file_handle == 0) {
    // File was not opened.
    return;
  }
  istream *istr = (istream *)file_handle;
  delete istr;
}


#endif //]
