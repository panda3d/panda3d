// Filename: milesAudioManager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "milesAudioManager.h"

#ifdef HAVE_RAD_MSS //[

#include "milesAudioSound.h"
#include "milesAudioSample.h"
#include "milesAudioStream.h"
#include "globalMilesManager.h"
#include "config_audio.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "nullAudioSound.h"
#include "string_utils.h"
#include "mutexHolder.h"
#include "lightReMutexHolder.h"

#include <algorithm>


TypeHandle MilesAudioManager::_type_handle;

AudioManager *Create_MilesAudioManager() {
  audio_debug("Create_MilesAudioManager()");
  return new MilesAudioManager();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::Constructor
//       Access: Public
//  Description: Create an audio manager.  This may open the Miles
//               sound system if there were no other MilesAudioManager
//               instances.  Subsequent managers may use the same
//               Miles resources.
////////////////////////////////////////////////////////////////////
MilesAudioManager::
MilesAudioManager() : 
  _lock("MilesAudioManager::_lock"),
  _streams_lock("MilesAudioManager::_streams_lock"),
  _streams_cvar(_streams_lock)
{
  audio_debug("MilesAudioManager::MilesAudioManager(), this = " 
              << (void *)this);
  GlobalMilesManager::get_global_ptr()->add_manager(this);
  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);
  _cleanup_required = true;
  _active = audio_active;
  _volume = audio_volume;
  _play_rate = 1.0f;
  _cache_limit = audio_cache_limit;
  _concurrent_sound_limit = 0;
  _is_valid = true;
  _hasMidiSounds = false;
  _sounds_finished = false;

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
//     Function: MilesAudioManager::Destructor
//       Access: Public, Virtual
//  Description: Clean up this audio manager and possibly release
//               the Miles resources that are reserved by the 
//               application (the later happens if this is the last
//               active manager).
////////////////////////////////////////////////////////////////////
MilesAudioManager::
~MilesAudioManager() {
  audio_debug("MilesAudioManager::~MilesAudioManager(), this = " 
              << (void *)this);
  cleanup();
  GlobalMilesManager::get_global_ptr()->remove_manager(this);

  audio_debug("MilesAudioManager::~MilesAudioManager() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::shutdown
//       Access: Public, Virtual
//  Description: Call this at exit time to shut down the audio system.
//               This will invalidate all currently-active
//               AudioManagers and AudioSounds in the system.  If you
//               change your mind and want to play sounds again, you
//               will have to recreate all of these objects.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
shutdown() {
  audio_debug("shutdown() started");
  GlobalMilesManager::get_global_ptr()->cleanup();
  audio_debug("shutdown() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::is_valid
//       Access: Public, Virtual
//  Description: This is mostly for debugging, but it it could be
//               used to detect errors in a release build if you
//               don't mind the cpu cost.
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
is_valid() {
  LightReMutexHolder holder(_lock);
  return do_is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_sound
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(AudioSound) MilesAudioManager::
get_sound(const string &file_name, bool, int) {
  LightReMutexHolder holder(_lock);
  audio_debug("MilesAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  if (!do_is_valid()) {
     audio_debug("invalid MilesAudioManager returning NullSound");
     return get_null_sound();
  }

  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());
  audio_debug("Reading "<<path);
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
          = _sounds.insert(SoundMap::value_type(path, sd));
      if (!ib.second) {
        // The insert failed.
        audio_debug("  failed map insert of "<<path);
        nassertr(do_is_valid(), NULL);
        return get_null_sound();
      }
      // Set si, so that we can get a reference to the path
      // for the MilesAudioSound.
      si=ib.first;
    }
  }
  // Create an AudioSound from the sound:
  PT(AudioSound) audioSound;

  if (sd != (SoundData *)NULL) {
    most_recently_used((*si).first);
    if (sd->_file_type == AILFILETYPE_MIDI ||
        sd->_file_type == AILFILETYPE_XMIDI ||
        sd->_file_type == AILFILETYPE_XMIDI_DLS ||
        sd->_file_type == AILFILETYPE_XMIDI_MLS) {
      // MIDI file.
      audioSound = new MilesAudioSequence(this, sd, file_name);

    } else if (!sd->_raw_data.empty()) {
      // WAV or MP3 file preloaded into memory.
      audioSound = new MilesAudioSample(this, sd, file_name);

    } else {
      // WAV or MP3 file streamed from disk.
      audioSound = new MilesAudioStream(this, file_name, path);
    }

    audioSound->set_active(_active);

    bool inserted = _sounds_on_loan.insert((MilesAudioSound *)audioSound.p()).second;
    nassertr(inserted, audioSound);

    _hasMidiSounds |= (file_name.find(".mid")!=string::npos);
  } else {
    // Couldn't load the file; just return a NullAudioSound.
    audioSound = new NullAudioSound;
  }

  audio_debug("  returning 0x" << (void*)audioSound);
  nassertr(do_is_valid(), NULL);
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_sound
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(AudioSound) MilesAudioManager::
get_sound(MovieAudio *sound, bool, int) {
  nassert_raise("Miles audio manager does not support MovieAudio sources.");
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::uncache_sound
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
uncache_sound(const string &file_name) {
  audio_debug("MilesAudioManager::uncache_sound(file_name=\""
      <<file_name<<"\")");
  LightReMutexHolder holder(_lock);
  nassertv(do_is_valid());
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  audio_debug("  path=\""<<path<<"\"");
  SoundMap::iterator i = _sounds.find(path);
  if (i != _sounds.end()) {
    nassertv(_lru.size() > 0);
    LRU::iterator lru_i = find(_lru.begin(), _lru.end(), &(i->first));
    nassertv(lru_i != _lru.end());
    _lru.erase(lru_i);
    _sounds.erase(i);
  }
  nassertv(do_is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::clear_cache
//       Access: Public, Virtual
//  Description: Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
clear_cache() {
  audio_debug("MilesAudioManager::clear_cache()");
  LightReMutexHolder holder(_lock);
  do_clear_cache();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_cache_limit
//       Access: Public, Virtual
//  Description: Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_cache_limit(unsigned int count) {
  LightReMutexHolder holder(_lock);

  audio_debug("MilesAudioManager::set_cache_limit(count="<<count<<")");
  nassertv(do_is_valid());
  while (_lru.size() > count) {
    uncache_a_sound();
  }
  _cache_limit=count;
  nassertv(do_is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_cache_limit
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
unsigned int MilesAudioManager::
get_cache_limit() const {
  return _cache_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_volume
//       Access: Public, Virtual
//  Description: set the overall volume
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_volume(PN_stdfloat volume) {
  audio_debug("MilesAudioManager::set_volume(volume="<<volume<<")");
  LightReMutexHolder holder(_lock);
  if (_volume != volume) {
    _volume = volume;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i = _sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
      (*i)->set_volume((*i)->get_volume());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_volume
//       Access: Public, Virtual
//  Description: get the overall volume
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::
get_volume() const {
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_play_rate
//       Access: Public
//  Description: set the overall play rate
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_play_rate(PN_stdfloat play_rate) {
  audio_debug("MilesAudioManager::set_play_rate(play_rate="<<play_rate<<")");
  LightReMutexHolder holder(_lock);
  if (_play_rate != play_rate) {
    _play_rate = play_rate;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i = _sounds_on_loan.begin();
    for (; i != _sounds_on_loan.end(); ++i) {
      (*i)->set_play_rate((*i)->get_play_rate());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_play_rate
//       Access: Public
//  Description: get the overall speed/pitch/play rate
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::
get_play_rate() const {
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_active
//       Access: Public, Virtual
//  Description: turn on/off
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_active(bool active) {
  audio_debug("MilesAudioManager::set_active(flag="<<active<<")");
  LightReMutexHolder holder(_lock);
  if (_active != active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i = _sounds_on_loan.begin();
    for (; i != _sounds_on_loan.end(); ++i) {
      (*i)->set_active(_active);
    }

    if ((!_active) && _hasMidiSounds) {
      GlobalMilesManager::get_global_ptr()->force_midi_reset();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_active
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_concurrent_sound_limit
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  LightReMutexHolder holder(_lock);
  _concurrent_sound_limit = limit;
  do_reduce_sounds_playing_to(_concurrent_sound_limit);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::get_concurrent_sound_limit
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int MilesAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::reduce_sounds_playing_to
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  LightReMutexHolder holder(_lock);
  do_reduce_sounds_playing_to(count);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::stop_all_sounds
//       Access: Public, Virtual
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
stop_all_sounds() {
  audio_debug("MilesAudioManager::stop_all_sounds()");
  reduce_sounds_playing_to(0);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_set_listener_attributes
//       Access: Public
//  Description: Set spatial attributes of the listener for 3D
//               sounds.  Note that Y and Z are switched to
//               translate into Miles's coordinate system.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  audio_debug("MilesAudioManager::audio_3d_set_listener_attributes()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  AIL_set_listener_3D_position(mgr->_digital_driver, px, pz, py);
  AIL_set_listener_3D_velocity_vector(mgr->_digital_driver, vx, vz, vy);
  AIL_set_listener_3D_orientation(mgr->_digital_driver, fx, fz, fy, ux, uz, uy);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: Get spatial attributes of the listener for 3D
//               sounds.  Note that Y and Z are switched to
//               translate from Miles's coordinate system.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  audio_debug("MilesAudioManager::audio_3d_get_listener_attributes()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  float lpx, lpy, lpz, lvx, lvy, lvz, lfx, lfy, lfz, lux, luy, luz;
  AIL_listener_3D_position(mgr->_digital_driver, &lpx, &lpz, &lpy);
  AIL_listener_3D_velocity(mgr->_digital_driver, &lvx, &lvz, &lvy);
  AIL_listener_3D_orientation(mgr->_digital_driver, &lfx, &lfz, &lfy, &lux, &luz, &luy);

  *px = lpx;
  *py = lpy;
  *pz = lpz;
  *vx = lvx;
  *vy = lvy;
  *vz = lvz;
  *fx = lfx;
  *fy = lfy;
  *fz = lfz;
  *ux = lux;
  *uy = luy;
  *uz = luz;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: Set factor to allow user to easily work in a
//               different scale.  1.0 represents meters.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::audio_3d_set_distance_factor(PN_stdfloat factor) {
  audio_debug("MilesAudioManager::audio_3d_set_distance_factor( factor= " << factor << ")");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  AIL_set_3D_distance_factor(mgr->_digital_driver, factor);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: Get factor controlling working units.
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::audio_3d_get_distance_factor() const {
  audio_debug("MilesAudioManager::audio_3d_get_distance_factor()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  return AIL_3D_distance_factor(mgr->_digital_driver);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_set_doppler_factor
//       Access: Public
//  Description: Exaggerates or diminishes the Doppler effect. 
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void MilesAudioManager::audio_3d_set_doppler_factor(PN_stdfloat factor) {
  audio_debug("MilesAudioManager::audio_3d_set_doppler_factor(factor="<<factor<<")");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  AIL_set_3D_doppler_factor(mgr->_digital_driver, factor);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: Get the factor controlling the Doppler effect.
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::audio_3d_get_doppler_factor() const {
  audio_debug("MilesAudioManager::audio_3d_get_doppler_factor()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  return AIL_3D_doppler_factor(mgr->_digital_driver);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_set_drop_off_factor
//       Access: Public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void MilesAudioManager::audio_3d_set_drop_off_factor(PN_stdfloat factor) {
  audio_debug("MilesAudioManager::audio_3d_set_drop_off_factor("<<factor<<")");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  AIL_set_3D_rolloff_factor(mgr->_digital_driver, factor);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: Get the factor controlling how quickly sound falls
//               off with distance.
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::audio_3d_get_drop_off_factor() const {
  audio_debug("MilesAudioManager::audio_3d_get_drop_off_factor()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
  return AIL_3D_rolloff_factor(mgr->_digital_driver);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::set_speaker_configuration
//       Access: Published
//  Description: Works similarly to MilesAudioSound::set_speaker_levels,
//               but specifies the 3D positions of the speakers in space.
//
//               Once a NULL value is found for a speaker position,
//               no more speaker positions will be used.
//
//               Note that Y and Z are switched to translate from Miles's
//               coordinate system.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
set_speaker_configuration(LVecBase3 *speaker1, LVecBase3 *speaker2, LVecBase3 *speaker3, LVecBase3 *speaker4, LVecBase3 *speaker5, LVecBase3 *speaker6, LVecBase3 *speaker7, LVecBase3 *speaker8, LVecBase3 *speaker9) {
  audio_debug("MilesAudioManager::set_speaker_configuration()");

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();

  MSSVECTOR3D speakers[9];

  if(speaker1 != NULL) {
    speakers[0].x = speaker1->get_x();
    speakers[0].y = speaker1->get_z();
    speakers[0].z = speaker1->get_y();
  }
  if(speaker2 != NULL) {
    speakers[1].x = speaker2->get_x();
    speakers[1].y = speaker2->get_z();
    speakers[1].z = speaker2->get_y();
  }
  if(speaker3 != NULL) {
    speakers[2].x = speaker3->get_x();
    speakers[2].y = speaker3->get_z();
    speakers[2].z = speaker3->get_y();
  }
  if(speaker4 != NULL) {
    speakers[3].x = speaker4->get_x();
    speakers[3].y = speaker4->get_z();
    speakers[3].z = speaker4->get_y();
  }
  if(speaker5 != NULL) {
    speakers[4].x = speaker5->get_x();
    speakers[4].y = speaker5->get_z();
    speakers[4].z = speaker5->get_y();
  }
  if(speaker6 != NULL) {
    speakers[5].x = speaker6->get_x();
    speakers[5].y = speaker6->get_z();
    speakers[5].z = speaker6->get_y();
  }
  if(speaker7 != NULL) {
    speakers[6].x = speaker7->get_x();
    speakers[6].y = speaker7->get_z();
    speakers[6].z = speaker7->get_y();
  }
  if(speaker8 != NULL) {
    speakers[7].x = speaker8->get_x();
    speakers[7].y = speaker8->get_z();
    speakers[7].z = speaker8->get_y();
  }
  if(speaker9 != NULL) {
    speakers[8].x = speaker9->get_x();
    speakers[8].y = speaker9->get_z();
    speakers[8].z = speaker9->get_y();
  }

  if(speaker1 == NULL) {
    audio_error("No valid speaker positions specified in MilesAudioManager::set_speaker_configuration().");
  } else if(speaker2 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 1, 1.0);
  } else if(speaker3 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 2, 1.0);
  } else if(speaker4 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 3, 1.0);
  } else if(speaker5 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 4, 1.0);
  } else if(speaker6 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 5, 1.0);
  } else if(speaker7 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 6, 1.0);
  } else if(speaker8 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 7, 1.0);
  } else if(speaker9 == NULL) {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 8, 1.0);
  } else {
    AIL_set_speaker_configuration(mgr->_digital_driver, speakers, 9, 1.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::update()
//       Access: Public, Virtual
//  Description: Must be called every frame.  Failure to call this
//               every frame could cause problems for some audio
//               managers.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
update() {
  {
    MutexHolder holder(_streams_lock);
    if (_stream_thread.is_null() && !_streams.empty()) {
      // If we don't have a sub-thread, we have to service the streams
      // in the main thread.
      do_service_streams();
    }
  }

  if (_sounds_finished) {
    _sounds_finished = false;
    
    // If the _sounds_finished flag was set, we should scan our list
    // of playing sounds and see if any of them have finished
    // recently.  We don't do this in the finished callback, because
    // that might have been called in a sub-thread (and we may not
    // have threading supported--and mutex protection--compiled in).

    SoundsPlaying::iterator si = _sounds_playing.begin();
    while (si != _sounds_playing.end()) {
      MilesAudioSound *sound = (*si);
      ++si;

      if (sound->status() == AudioSound::READY) {
        sound->stop();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::release_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
release_sound(MilesAudioSound *audioSound) {
  audio_debug("MilesAudioManager::release_sound(audioSound=\""
              <<audioSound->get_name()<<"\"), this = " << (void *)this);
  LightReMutexHolder holder(_lock);
  AudioSet::iterator ai = _sounds_on_loan.find(audioSound);
  if (ai != _sounds_on_loan.end()) {
    _sounds_on_loan.erase(ai);
  }

  audio_debug("MilesAudioManager::release_sound() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::cleanup
//       Access: Public
//  Description: Shuts down the audio manager and releases any
//               resources associated with it.  Also cleans up all
//               AudioSounds created via the manager.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
cleanup() {
  audio_debug("MilesAudioManager::cleanup(), this = " << (void *)this
              << ", _cleanup_required = " << _cleanup_required);
  LightReMutexHolder holder(_lock);
  if (!_cleanup_required) {
    return;
  }

  // Be sure to cleanup associated sounds before cleaning up the manager:
  AudioSet orig_sounds;
  orig_sounds.swap(_sounds_on_loan);
  AudioSet::iterator ai;
  for (ai = orig_sounds.begin(); ai != orig_sounds.end(); ++ai) {
    (*ai)->cleanup();
  }

  do_clear_cache();

  // Now stop the thread, if it has been started.
  if (!_stream_thread.is_null()) {
    milesAudio_cat.info()
      << "Stopping audio streaming thread.\n";
    PT(StreamThread) old_thread;
    {
      MutexHolder holder(_streams_lock);
      nassertv(!_stream_thread.is_null());
      _stream_thread->_keep_running = false;
      _streams_cvar.notify();
      old_thread = _stream_thread;
      _stream_thread.clear();
    }
    old_thread->join();
  }

  _cleanup_required = false;
  audio_debug("MilesAudioManager::cleanup() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
output(ostream &out) const {
  LightReMutexHolder holder(_lock);
  out << get_type() << ": " << _sounds_playing.size()
      << " / " << _sounds_on_loan.size() << " sounds playing / total"; 
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
write(ostream &out) const {
  LightReMutexHolder holder(_lock);

  out << (*this) << "\n";
  AudioSet::const_iterator ai;
  for (ai = _sounds_on_loan.begin(); ai != _sounds_on_loan.end(); ++ai) {
    MilesAudioSound *sound = (*ai);
    out << "  " << *sound << "\n";
  }

  size_t total_preload = 0;
  size_t num_preloaded = 0;
  SoundMap::const_iterator si;
  for (si = _sounds.begin(); si != _sounds.end(); ++si) {
    if (!(*si).second->_raw_data.empty()) {
      ++num_preloaded;
      total_preload += (*si).second->_raw_data.size();
    }
  }
  out << num_preloaded << " of " << _sounds.size() << " sounds preloaded, size used is " << (total_preload + 1023) / 1024 << "K\n";

  {
    MutexHolder holder(_streams_lock);
    out << _streams.size() << " streams opened.\n";
    if (!_stream_thread.is_null()) {
      out << "(Audio streaming thread has been started.)\n";
    }
  }

  GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
 
  int num_samples = mgr->get_num_samples();
  out << num_samples << " sample handles allocated globally.\n";

  int num_sequences = mgr->get_num_sequences();
  out << num_sequences << " sequence handles allocated globally.\n";
}


////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::do_is_valid
//       Access: Private
//  Description: Implementation of is_valid().  Assumes the lock is
//               already held.
////////////////////////////////////////////////////////////////////
bool MilesAudioManager::
do_is_valid() {
  bool check = true;
  if (_sounds.size() != _lru.size()) {
    audio_debug("-- Error _sounds.size() != _lru.size() --");
    check = false;

  } else {
    LRU::const_iterator i = _lru.begin();
    for (; i != _lru.end(); ++i) {
      SoundMap::const_iterator smi = _sounds.find(**i);
      if (smi == _sounds.end()) {
        audio_debug("-- "<<**i<<" in _lru and not in _sounds --");
        check = false;
        break;
      }
    }
  }
  return _is_valid && check;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::do_reduce_sounds_playing_to
//       Access: Private
//  Description: Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
do_reduce_sounds_playing_to(unsigned int count) {
  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    assert(sound != _sounds_playing.end());
    (**sound).stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::do_clear_cache
//       Access: Private
//  Description: Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
do_clear_cache() {
  if (_is_valid) { nassertv(do_is_valid()); }
  _sounds.clear();
  _lru.clear();
  if (_is_valid) { nassertv(do_is_valid()); }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::start_service_stream
//       Access: Private
//  Description: Adds the indicated stream to the list of streams to
//               be serviced by a Panda sub-thread.  This is in lieu
//               of Miles' auto-service-stream mechanism.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
start_service_stream(HSTREAM stream) {
  MutexHolder holder(_streams_lock);
  nassertv(find(_streams.begin(), _streams.end(), stream) == _streams.end());
  _streams.push_back(stream);
  _streams_cvar.notify();

  if (_stream_thread.is_null() && Thread::is_threading_supported()) {
    milesAudio_cat.info()
      << "Starting audio streaming thread.\n";
    _stream_thread = new StreamThread(this);
    _stream_thread->start(TP_low, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::stop_service_stream
//       Access: Private
//  Description: Removes the indicated stream from the list of streams
//               to be serviced by a Panda sub-thread.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
stop_service_stream(HSTREAM stream) {
  MutexHolder holder(_streams_lock);
  Streams::iterator si = find(_streams.begin(), _streams.end(), stream);
  if (si != _streams.end()) {
    _streams.erase(si);
  }
}
  

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::most_recently_used
//       Access: Private
//  Description: Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
most_recently_used(const string &path) {
  audio_debug("MilesAudioManager::most_recently_used(path=\""
      <<path<<"\")");
  LRU::iterator i=find(_lru.begin(), _lru.end(), &path);
  if (i != _lru.end()) {
    _lru.erase(i);
  }
  // At this point, path should not exist in the _lru:
  assert(find(_lru.begin(), _lru.end(), &path) == _lru.end());
  _lru.push_back(&path);
  nassertv(do_is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::uncache_a_sound
//       Access: Private
//  Description: Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
uncache_a_sound() {
  audio_debug("MilesAudioManager::uncache_a_sound()");
  nassertv(do_is_valid());
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
  nassertv(do_is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::starting_sound
//       Access: Private
//  Description: Inform the manager that a sound is about to play.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
starting_sound(MilesAudioSound *audio) {
  LightReMutexHolder holder(_lock);
  if (_concurrent_sound_limit) {
    do_reduce_sounds_playing_to(_concurrent_sound_limit);
  }
  _sounds_playing.insert(audio);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::stopping_sound
//       Access: Private
//  Description: Inform the manager that a sound is finished or 
//               someone called stop on the sound (this should not
//               be called if a sound is only paused).
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
stopping_sound(MilesAudioSound *audio) {
  LightReMutexHolder holder(_lock);
  _sounds_playing.erase(audio);
  if (_hasMidiSounds && _sounds_playing.size() == 0) {
    GlobalMilesManager::get_global_ptr()->force_midi_reset();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::load
//       Access: Private
//  Description: Reads a sound file and allocates a SoundData pointer
//               for it.  Returns NULL if the sound file cannot be
//               loaded.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
PT(MilesAudioManager::SoundData) MilesAudioManager::
load(const Filename &file_name) {
  PT(SoundData) sd = new SoundData;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) file = vfs->get_file(file_name);
  if (file == (VirtualFile *)NULL) {
    milesAudio_cat.warning()
      << "No such file: " << file_name << "\n";
    return NULL;
  }

  if (file->get_file_size() == 0) {
    milesAudio_cat.warning()
      << "File " << file_name << " is empty\n";
    return NULL;
  }

  sd->_basename = file_name.get_basename();

  string extension = sd->_basename.get_extension();
  if (extension == "pz") {
    extension = Filename(sd->_basename.get_basename_wo_extension()).get_extension();
  }

  bool is_midi_file = (downcase(extension) == "mid");

  if ((miles_audio_preload_threshold == -1 || file->get_file_size() < (streamsize)miles_audio_preload_threshold) ||
      is_midi_file) {
    // If the file is sufficiently small, we'll preload it into
    // memory.  MIDI files cannot be streamed, so we always preload
    // them, regardless of size.

    if (!file->read_file(sd->_raw_data, true)) {
      milesAudio_cat.warning()
        << "Unable to read " << file_name << "\n";
      return NULL;
    }

    sd->_file_type = 
      AIL_file_type(&sd->_raw_data[0], sd->_raw_data.size());

    if (sd->_file_type == AILFILETYPE_MIDI) {
      // A standard MIDI file.  We have to convert this to XMIDI for
      // Miles.
      void *xmi;
      U32 xmi_size;
      if (AIL_MIDI_to_XMI(&sd->_raw_data[0], sd->_raw_data.size(),
                          &xmi, &xmi_size, 0)) {
        audio_debug("converted " << sd->_basename << " from standard MIDI ("
                    << sd->_raw_data.size() << " bytes) to XMIDI ("
                    << xmi_size << " bytes)");

        // Copy the data to our own buffer and free the
        // Miles-allocated data.
        sd->_raw_data.clear();
        sd->_raw_data.insert(sd->_raw_data.end(), 
                             (unsigned char *)xmi, (unsigned char *)xmi + xmi_size);
        AIL_mem_free_lock(xmi);
        sd->_file_type = AILFILETYPE_XMIDI;
      } else {
        milesAudio_cat.warning()
          << "Could not convert " << sd->_basename << " to XMIDI.\n";
      }
    }
    
    bool expand_to_wav = false;

    if (sd->_file_type != AILFILETYPE_MPEG_L3_AUDIO) {
      audio_debug(sd->_basename << " is not an mp3 file.");
    } else if ((int)sd->_raw_data.size() >= miles_audio_expand_mp3_threshold) {
      audio_debug(sd->_basename << " is too large to expand in-memory.");
    } else {
      audio_debug(sd->_basename << " will be expanded in-memory.");
      expand_to_wav = true;
    }
    
    if (expand_to_wav) {
      // Now convert the file to WAV format in-memory.  This is useful
      // to work around seek and length problems associated with
      // variable bit-rate MP3 encoding.
      void *wav_data;
      U32 wav_data_size;
      if (AIL_decompress_ASI(&sd->_raw_data[0], sd->_raw_data.size(),
                             sd->_basename.c_str(), &wav_data, &wav_data_size,
                             NULL)) {
        audio_debug("expanded " << sd->_basename << " from " << sd->_raw_data.size()
                    << " bytes to " << wav_data_size << " bytes.");
        
        if (wav_data_size != 0) {
          // Now copy the memory into our own buffers, and free the
          // Miles-allocated memory.
          sd->_raw_data.clear();
          sd->_raw_data.insert(sd->_raw_data.end(),
                               (unsigned char *)wav_data, (unsigned char *)wav_data + wav_data_size);
          sd->_file_type = AILFILETYPE_PCM_WAV;
          sd->_basename.set_extension("wav");
        }
        AIL_mem_free_lock(wav_data);
        
      } else {
        audio_debug("unable to expand " << sd->_basename);
      }
    }

  } else {
    // If the file is large, we'll stream it from disk instead of
    // preloading it.  This means we don't need to load any data at
    // this point.
  }

  return sd;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::thread_main
//       Access: Private
//  Description: Called to service the streaming audio channels
//               currently playing on the audio manager.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
thread_main(volatile bool &keep_running) {
  MutexHolder holder(_streams_lock);

  while (keep_running) {
    if (_streams.empty()) {
      // If there are no streams to service, block on the condition
      // variable.
      _streams_cvar.wait();
    } else {
      do_service_streams();
    }

    // Now yield to be polite to the main application.
    _streams_lock.release();
    Thread::force_yield();
    _streams_lock.acquire();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::do_service_streams
//       Access: Private
//  Description: Internal function to service all the streams.
//               Assumes _streams_lock is already held.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::
do_service_streams() {
  size_t i = 0;
  while (i < _streams.size()) {
    HSTREAM stream = _streams[i];
    
    // We must release the lock while we are servicing stream i.
    _streams_lock.release();
    AIL_service_stream(stream, 0);
    Thread::consider_yield();
    _streams_lock.acquire();
    
    ++i;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::StreamThread::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::StreamThread::
StreamThread(MilesAudioManager *mgr) : 
  Thread("StreamThread", "StreamThread"),
  _mgr(mgr) 
{
  _keep_running = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::StreamThread::thread_main
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioManager::StreamThread::
thread_main() {
  _mgr->thread_main(_keep_running);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::SoundData::
SoundData() :
  _raw_data(MilesAudioManager::get_class_type()),
  _has_length(false),
  _length(0.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioManager::SoundData::
~SoundData() {
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::get_length
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioManager::SoundData::
get_length() {
  if (!_has_length) {
    // Time to determine the length of the file.
    
    if (_raw_data.empty()) {
      _length = 0.0f;
      _has_length = true;

    } else if (_file_type == AILFILETYPE_MPEG_L3_AUDIO) {
      // If it's an mp3 file, we have to calculate its length by
      // walking through all of its frames.
      audio_debug("Computing length of mp3 file " << _basename);
      
      MP3_INFO info;
      AIL_inspect_MP3(&info, &_raw_data[0], _raw_data.size());
      _length = 0.0f;
      while (AIL_enumerate_MP3_frames(&info)) {
        _length += info.data_size * 8.0f / info.bit_rate;
      }
      _has_length = true;

    } else if (_file_type == AILFILETYPE_PCM_WAV ||
               _file_type == AILFILETYPE_ADPCM_WAV ||
               _file_type == AILFILETYPE_OTHER_ASI_WAV) {
      audio_debug("Getting length of wav file " << _basename);

      AILSOUNDINFO info;
      if (AIL_WAV_info(&_raw_data[0], &info)) {
        _length = (PN_stdfloat)info.samples / (PN_stdfloat)info.rate;
        audio_debug(info.samples << " samples at " << info.rate
                    << "; length is " << _length << " seconds.");
        _has_length = true;
      }
    }
  }

  nassertr(_has_length, 0.0f);
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioManager::SoundData::set_length
//       Access: Public
//  Description: Records the sample length, as determined externally.
////////////////////////////////////////////////////////////////////
void MilesAudioManager::SoundData::
set_length(PN_stdfloat length) {
  _length = length;
  _has_length = true;
}

#endif //]
