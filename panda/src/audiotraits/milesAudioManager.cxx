// Filename: milesAudioManager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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


TypeHandle MilesAudioManager::_type_handle;

int MilesAudioManager::_active_managers = 0;
bool MilesAudioManager::_miles_active = false;
HDLSFILEID MilesAudioManager::_dls_field = NULL;

// This is the list of all MilesAudioManager objects in the world.  It
// must be a pointer rather than a concrete object, so it won't be
// destructed at exit time before we're done removing things from it.
MilesAudioManager::Managers *MilesAudioManager::_managers;

PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() Miles.");
  return new MilesAudioManager();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::MilesAudioManager
//       Access: Public
//  Description: Create an audio manager.  This may open the Miles
//               sound system if there were no other MilesAudioManager
//               instances.  Subsequent managers may use the same
//               Miles resources.
////////////////////////////////////////////////////////////////////
MilesAudioManager::
MilesAudioManager() {
  audio_debug("MilesAudioManager::MilesAudioManager(), this = " 
              << (void *)this);
  if (_managers == (Managers *)NULL) {
    _managers = new Managers;
  }
  _managers->insert(this);

  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);
  _cleanup_required = true;
  _active = audio_active;
  _volume = audio_volume;
  _cache_limit = audio_cache_limit;
  _concurrent_sound_limit = 0;
  _is_valid = true;
  _hasMidiSounds = false;
  if (_active_managers==0 || !_miles_active) {
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
      _miles_active = true;
      if (audio_software_midi) {
        // Load the downloadable sounds file:

        if (_dls_field == NULL) {
          HDLSDEVICE dls;
          AIL_quick_handles(0, 0, &dls);
          nassertv(dls != NULL);
          string dls_file = Filename(audio_dls_file).to_os_specific();
          if (dls_file.empty()) {
            get_gm_file_path(dls_file);
            // we need more dbg info in logs, so bumping the msgs from 'debug' status to 'info' status
            audio_info("  using default dls_file: "<< dls_file );
          }
          
          audio_debug("  dls_file=\""<<dls_file<<"\"");
          
          // note: if AIL_DLS_load_file is not done, midi fails to play on some machines.
          nassertv(_dls_field == NULL);
          audio_debug("  AIL_DLS_load_file(dls, " << dls_file << ", 0)");
          _dls_field = AIL_DLS_load_file(dls, dls_file.c_str(), 0);
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
        }
      } else {
        audio_info("  using Miles hardware midi");
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
  nassertv(_active_managers>0);

  // We used to hang a call to a force-shutdown function on atexit(),
  // so that any running sounds (particularly MIDI sounds) would be
  // silenced on exit, especially a sudden exit triggered by a Python
  // exception.  But that causes problems because Miles itself also
  // hangs a force-shutdown function on atexit(), and you can't call
  // AIL_cleanup() twice--that results in a crash.

  // Nowadays, we provide the AudioManager::shutdown() method instead,
  // which allows the app to force all sounds to stop cleanly before
  // we get to the atexit() stack.  In Python, we guarantee that this
  // method will be called in the sys.exitfunc chain.
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::~MilesAudioManager
//       Access: Public
//  Description: Clean up this audio manager and possibly release
//               the Miles resources that are reserved by the 
//               application (the later happens if this is the last
//               active manager).
////////////////////////////////////////////////////////////////////
MilesAudioManager::
~MilesAudioManager() {
  audio_debug("MilesAudioManager::~MilesAudioManager(), this = " 
              << (void *)this);
  nassertv(_managers != (Managers *)NULL);
  Managers::iterator mi = _managers->find(this);
  nassertv(mi != _managers->end());
  _managers->erase(mi);

  cleanup();
  audio_debug("MilesAudioManager::~MilesAudioManager() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::shutdown
//       Access: Published, Virtual
//  Description: Call this at exit time to shut down the audio system.
//               This will invalidate all currently-active
//               AudioManagers and AudioSounds in the system.  If you
//               change your mind and want to play sounds again, you
//               will have to recreate all of these objects.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
shutdown() {
  audio_debug("shutdown(), _miles_active = " << _miles_active);
  if (_managers != (Managers *)NULL) {
    Managers::iterator mi;
    for (mi = _managers->begin(); mi != _managers->end(); ++mi) {
      (*mi)->cleanup();
    }
  }

  nassertv(_active_managers == 0);
  audio_debug("shutdown() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::is_valid
//       Access:
//  Description: This is mostly for debugging, but it it could be
//               used to detect errors in a release build if you
//               don't mind the cpu cost.
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
is_valid() {
  bool check=true;
  if (_sounds.size() != _lru.size()) {
    audio_debug("-- Error _sounds.size() != _lru.size() --");
    check=false;
  } else {
    LRU::const_iterator i=_lru.begin();
    for (; i != _lru.end(); ++i) {
      SoundMap::const_iterator smi=_sounds.find(**i);
      if (smi == _sounds.end()) {
        audio_debug("-- "<<**i<<" in _lru and not in _sounds --");
        check=false;
        break;
      }
    }
  }
  return _is_valid && check;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::load
//       Access: Private
//  Description: Reads a sound file and allocates a SoundData pointer
//               for it.  Returns NULL if the sound file cannot be
//               loaded.
////////////////////////////////////////////////////////////////////
PT(MilesAudioManager::SoundData) MilesAudioManager::
load(Filename file_name) {
  // We used to use callbacks to hook AIL_quick_load() into the vfs
  // system directly.  The theory was that that would enable
  // AIL_quick_load() to stream the file from disk, avoiding the need
  // to keep the whole thing in memory at once.  But it turns out that
  // AIL_quick_load() always reads the whole file anyway, so it's a
  // moot point.

  // Nowadays we don't mess around with that callback nonsense, and
  // just read the whole file directly.  Not only is it simpler, but
  // preloading the sound files allows us to optionally convert MP3 to
  // WAV format in-memory at load time.

  PT(SoundData) sd = new SoundData;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->read_file(file_name, sd->_raw_data)) {
    milesAudio_cat.warning()
      << "Unable to read " << file_name << "\n";
    return NULL;
  }

  sd->_basename = file_name.get_basename();
  sd->_file_type = 
    AIL_file_type(sd->_raw_data.data(), sd->_raw_data.size());

  bool expand_to_wav = false;
  
  if (sd->_file_type != AILFILETYPE_MPEG_L3_AUDIO) {
    audio_debug(sd->_basename << " is not an mp3 file.");
  } else if ((int)sd->_raw_data.size() >= miles_audio_expand_mp3_threshold) {
    audio_debug(sd->_basename << " is too large to expand in-memory.");
  } else {
    expand_to_wav = true;
  }

  if (expand_to_wav) {
    // Now convert the file to WAV format in-memory.  This is useful
    // to work around seek and length problems associated with
    // variable bit-rate MP3 encoding.
    void *wav_data;
    U32 wav_data_size;
    if (AIL_decompress_ASI(sd->_raw_data.data(), sd->_raw_data.size(),
                           sd->_basename.c_str(), &wav_data, &wav_data_size,
                           NULL)) {
      audio_debug("expanded " << sd->_basename << " from " << sd->_raw_data.size()
                  << " bytes to " << wav_data_size << " bytes.");

      // Now copy the memory into our own buffers, and free the
      // Miles-allocated memory.
      sd->_raw_data.assign((char *)wav_data, wav_data_size);
      AIL_mem_free_lock(wav_data);
      sd->_file_type = AILFILETYPE_PCM_WAV;

    } else {
      audio_debug("unable to expand " << sd->_basename);
    }
  }

  sd->_audio = AIL_quick_load_mem(sd->_raw_data.data(), sd->_raw_data.size());

  if (!sd->_audio) {
    audio_error("  MilesAudioManager::load failed "<< AIL_last_error());
    return NULL;
  }

  // We still need to keep around the raw data value, since
  // AIL_quick_load_mem() doesn't make a copy.

  return sd;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PT(AudioSound) MilesAudioManager::
get_sound(const string& file_name, bool) {
  audio_debug("MilesAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  if(!is_valid()) {
     audio_debug("invalid MilesAudioManager returning NullSound");
     return get_null_sound();
  }

  assert(is_valid());
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_sound_path());
  audio_debug("  resolved file_name is '"<<path<<"'");

  PT(SoundData) sd;
  // Get the sound, either from the cache or from a loader:
  SoundMap::const_iterator si=_sounds.find(path);
  if (si != _sounds.end()) {
    // ...found the sound in the cache.
    sd = (*si).second;
    audio_debug("  sound found in pool 0x" << (void*)sd);
  } else {
    // ...the sound was not found in the cache/pool.
    sd = load(path);
    if (sd != (SoundData *)NULL) {
      while (_sounds.size() >= (unsigned int)_cache_limit) {
        uncache_a_sound();
      }
      // Put it in the pool:
      // The following is roughly like: _sounds[path] = sd;
      // But, it gives us an iterator into the map.
      pair<SoundMap::const_iterator, bool> ib
          =_sounds.insert(SoundMap::value_type(path, sd));
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
  if (sd != (SoundData *)NULL) {
    most_recently_used((*si).first);
    PT(MilesAudioSound) milesAudioSound
        =new MilesAudioSound(this, sd, (*si).first);
    nassertr(milesAudioSound, 0);
    milesAudioSound->set_active(_active);
    bool inserted = _sounds_on_loan.insert(milesAudioSound).second;
    nassertr(inserted, milesAudioSound.p());
    audioSound=milesAudioSound;
  }

  _hasMidiSounds |= (file_name.find(".mid")!=string::npos);
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

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_sound_path());

  audio_debug("  path=\""<<path<<"\"");
  SoundMap::iterator i=_sounds.find(path);
  if (i != _sounds.end()) {
    assert(_lru.size()>0);
    LRU::iterator lru_i=find(_lru.begin(), _lru.end(), &(i->first));
    assert(lru_i != _lru.end());
    _lru.erase(lru_i);
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
//  Description: Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
clear_cache() {
  audio_debug("MilesAudioManager::clear_cache()");
  if (_is_valid) { assert(is_valid()); }
  _sounds.clear();
  _lru.clear();
  if (_is_valid) { assert(is_valid()); }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_cache_limit
//       Access: Public
//  Description: Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("MilesAudioManager::set_cache_limit(count="<<count<<")");
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
unsigned int MilesAudioManager::
get_cache_limit() const {
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
              <<audioSound->get_name()<<"\"), this = " << (void *)this);
  AudioSet::iterator ai = _sounds_on_loan.find(audioSound);
  nassertv(ai != _sounds_on_loan.end());
  _sounds_on_loan.erase(ai);

  audio_debug("MilesAudioManager::release_sound() finished");
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
    AudioSet::iterator i=_sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
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
get_volume() const {
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
    AudioSet::iterator i=_sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
      (**i).set_active(_active);
    }

    if((!_active) && _hasMidiSounds) {
        force_midi_reset();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_active
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
get_active() const {
  audio_debug("MilesAudioManager::get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::starting_sound
//       Access: 
//  Description: Inform the manager that a sound is about to play.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
starting_sound(MilesAudioSound* audio) {
  if (_concurrent_sound_limit) {
    reduce_sounds_playing_to(_concurrent_sound_limit);
  }
  _sounds_playing.insert(audio);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::stopping_sound
//       Access: 
//  Description: Inform the manager that a sound is finished or 
//               someone called stop on the sound (this should not
//               be called if a sound is only paused).
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
stopping_sound(MilesAudioSound* audio) {
  _sounds_playing.erase(audio);
  if (_hasMidiSounds && _sounds_playing.size() == 0) {
    force_midi_reset();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  _concurrent_sound_limit = limit;
  reduce_sounds_playing_to(_concurrent_sound_limit);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int MilesAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::reduce_sounds_playing_to
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    assert(sound != _sounds_playing.end());
    (**sound).stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::stop_all_sounds
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
stop_all_sounds() {
  audio_debug("MilesAudioManager::stop_all_sounds()");
  reduce_sounds_playing_to(0);
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
//     Function: MilesAudioManager::force_midi_reset
//       Access: Private
//  Description: ?????.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
force_midi_reset() {
  if (!miles_audio_force_midi_reset) {
    audio_debug("MilesAudioManager::skipping force_midi_reset");  
    return;
  }
  audio_debug("MilesAudioManager::force_midi_reset");

  // sometimes Miles seems to leave midi notes hanging, even after
  // stop is called, so perform an explicit reset using winMM.dll
  // calls, just to ensure silence.

  HMDIDRIVER hMid=NULL;
  AIL_quick_handles(0, &hMid, 0);
  if ((hMid!=NULL) && (hMid->deviceid != MIDI_NULL_DRIVER) && (hMid->hMidiOut != NULL)) {
    audio_debug("MilesAudioManager::calling midiOutReset");
    midiOutReset(hMid->hMidiOut);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::cleanup
//       Access: Private
//  Description: Shuts down the audio manager and releases any
//               resources associated with it.  Also cleans up all
//               AudioSounds created via the manager.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
cleanup() {
  audio_debug("MilesAudioManager::cleanup(), this = " << (void *)this
              << ", _cleanup_required = " << _cleanup_required);
  if (!_cleanup_required) {
    return;
  }

  // Be sure to cleanup associated sounds before cleaning up the manager:
  AudioSet::iterator ai;
  for (ai = _sounds_on_loan.begin(); ai != _sounds_on_loan.end(); ++ai) {
    (*ai)->cleanup();
  }

  clear_cache();
  nassertv(_active_managers > 0);
  --_active_managers;
  audio_debug("  _active_managers="<<_active_managers);

  if (_active_managers == 0) {
    if (_dls_field != NULL) {
      HDLSDEVICE dls;
      AIL_quick_handles(0, 0, &dls);
      audio_debug("  AIL_DLS_unload()");
      AIL_DLS_unload(dls, _dls_field);
      _dls_field = NULL;
    }

    if (_miles_active) {
      audio_debug("  AIL_quick_shutdown()");
      AIL_quick_shutdown();
      _miles_active = false;
    }
  }
  _cleanup_required = false;
  audio_debug("MilesAudioManager::cleanup() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::SoundData::
SoundData() :
  _audio(0),
  _has_length(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::SoundData::
~SoundData() {
  if (_audio != 0) {
    if (_miles_active) {
      AIL_quick_unload(_audio);
    }
    _audio = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float MilesAudioManager::SoundData::
get_length() {
  if (!_has_length) {
    // Time to determine the length of the file.

    if (_file_type == AILFILETYPE_MPEG_L3_AUDIO &&
        (int)_raw_data.size() < miles_audio_calc_mp3_threshold) {
      // If it's an mp3 file, we may not trust Miles to compute its
      // length correctly (Miles doesn't correctly compute the length
      // of VBR MP3 files).  So in that case, decompress the whole
      // file to determine its precise length.
      audio_debug("Computing length of " << _basename);

      void *wav_data;
      U32 wav_data_size;
      if (AIL_decompress_ASI(_raw_data.data(), _raw_data.size(),
                             _basename.c_str(), &wav_data, &wav_data_size,
                             NULL)) {
        AILSOUNDINFO info;
        if (AIL_WAV_info(wav_data, &info)) {
          _length = (float)info.samples / (float)info.rate;
          audio_debug(info.samples << " samples at " << info.rate
                      << "; length is " << _length << " seconds.");
          _has_length = true;
        }

        AIL_mem_free_lock(wav_data);
      }
    }

    if (!_has_length) {
      // If it's not an mp3 file, or we don't care about precalcing
      // mp3 files, just ask Miles to do it.
      _length = ((float)AIL_quick_ms_length(_audio)) * 0.001f;

      audio_debug("Miles reports length of " << _length
                  << " for " << _basename);
    }
  }

  return _length;
}

#endif //]
