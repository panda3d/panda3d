// Filename: openalAudioManager.cxx
// Created by:  Ben Buchwald <bb2@alumni.cmu.edu>
//
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

#ifdef HAVE_OPENAL //[

//Panda headers.
#include "config_audio.h"
#include "config_util.h"
#include "config_express.h"
#include "openalAudioManager.h"
#include "openalAudioSound.h"
#include "virtualFileSystem.h"
#include "movieAudio.h"

#include <algorithm>

//OpenAL Headers.
#include <al.h>
#include <alc.h>

TypeHandle OpenALAudioManager::_type_handle;

int OpenALAudioManager::_active_managers = 0;
bool OpenALAudioManager::_openal_active = false;
ALCdevice* OpenALAudioManager::_device = NULL;
ALCcontext* OpenALAudioManager::_context = NULL;

// This is the list of all OpenALAudioManager objects in the world.  It
// must be a pointer rather than a concrete object, so it won't be
// destructed at exit time before we're done removing things from it.
OpenALAudioManager::Managers *OpenALAudioManager::_managers = NULL;

OpenALAudioManager::SourceCache *OpenALAudioManager::_al_sources = NULL;


////////////////////////////////////////////////////////////////////
// Central dispatcher for audio errors.
////////////////////////////////////////////////////////////////////
void al_audio_errcheck(const char *context) {
  ALenum result = alGetError();
  if (result != AL_NO_ERROR) {
    audio_error(context << ": " << alGetString(result) );
  }
}

void alc_audio_errcheck(const char *context,ALCdevice* device) {
  ALCenum result = alcGetError(device);
  if (result != ALC_NO_ERROR) {
    audio_error(context << ": " << alcGetString(device,result) );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Create_AudioManager
//       Access: Private
//  Description: Factory Function
////////////////////////////////////////////////////////////////////
PT(AudioManager) Create_AudioManager() {
  audio_debug("Create_AudioManager() OpenAL.");
  return new OpenALAudioManager;
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::OpenALAudioManager()
//       Access: Public
//  Description: Constructor
////////////////////////////////////////////////////////////////////
OpenALAudioManager::
OpenALAudioManager() {
  audio_debug("OpenALAudioManager::OpenALAudioManager(), this = " 
              << (void *)this);
  if (_managers == (Managers *)NULL) {
    _managers = new Managers;
    _al_sources = new SourceCache;
  }

  _managers->insert(this);

  audio_debug("  audio_active="<<audio_active);
  audio_debug("  audio_volume="<<audio_volume);

  _cleanup_required = true;
  _active = audio_active;
  _volume = audio_volume;
  _play_rate = 1.0f;
  
  _cache_limit = audio_cache_limit;

  _concurrent_sound_limit = 0;
  _is_valid = true;

  //Init 3D attributes
  _distance_factor = 3.28;
  _drop_off_factor = 1;

  _position[0] = 0;
  _position[1] = 0;
  _position[2] = 0;

  _velocity[0] = 0;
  _velocity[1] = 0;
  _velocity[2] = 0;

  _forward_up[0] = 0;
  _forward_up[1] = 0;
  _forward_up[2] = 0;
  _forward_up[3] = 0;
  _forward_up[4] = 0;
  _forward_up[5] = 0;

  // Initialization
  if (_active_managers==0 || !_openal_active) {
    _device = alcOpenDevice(NULL); // select the "preferred device"
    if (!_device) {
      // this is a unique kind of error
      audio_error("OpenALAudioManager: alcOpenDevice(NULL): ALC couldn't open device");
    } else {
      alcGetError(_device); // clear errors
      _context=alcCreateContext(_device,NULL);
      alc_audio_errcheck("alcCreateContext(_device,NULL)",_device);
      if (_context!=NULL) {
        _openal_active = true;
      }
    }
  }
  // We increment _active_managers regardless of possible errors above.
  // The shutdown call will do the right thing when it's called,
  // either way.
  ++_active_managers;
  audio_debug("  _active_managers="<<_active_managers);
  nassertv(_active_managers>0);

  if (!_device || !_context) {
    audio_error("OpenALAudioManager: No open device or context");
    _is_valid = false;
  } else {
    make_current();	

    // set 3D sound characteristics as they are given in the configrc
    audio_3d_set_doppler_factor(audio_doppler_factor);
    audio_3d_set_distance_factor(audio_distance_factor);
    audio_3d_set_drop_off_factor(audio_drop_off_factor);
  }
  
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::~OpenALAudioManager
//       Access: Public
//  Description: DESTRCUTOR !!!
////////////////////////////////////////////////////////////////////
OpenALAudioManager::
~OpenALAudioManager() {
  audio_debug("OpenALAudioManager::~OpenALAudioManager(), this = " 
              << (void *)this);
  nassertv(_managers != (Managers *)NULL);
  Managers::iterator mi = _managers->find(this);
  nassertv(mi != _managers->end());
  _managers->erase(mi);

  cleanup();
  audio_debug("OpenALAudioManager::~OpenALAudioManager() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::shutdown
//       Access: Published, Virtual
//  Description: Call this at exit time to shut down the audio system.
//               This will invalidate all currently-active
//               AudioManagers and AudioSounds in the system.  If you
//               change your mind and want to play sounds again, you
//               will have to recreate all of these objects.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
shutdown() {
  audio_debug("shutdown(), _openal_active = " << _openal_active);
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
//     Function: OpenALAudioManager::is_valid
//       Access:
//  Description: This is mostly for debugging, but it it could be
//               used to detect errors in a release build if you
//               don't mind the cpu cost.
////////////////////////////////////////////////////////////////////
bool OpenALAudioManager::
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
//     Function: OpenALAudioManager::make_current
//       Access: Private
//  Description: This makes this manager's OpenAL context the 
//         current context. Needed before any parameter sets.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
make_current() const {
  alcGetError(_device); // clear errors
  alcMakeContextCurrent(_context);
  alc_audio_errcheck("alcMakeContextCurrent(_context)",_device);
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::load
//       Access: Private
//  Description: Reads a sound file and allocates a SoundData pointer
//               for it.  Returns NULL if the sound file cannot be
//               loaded.
////////////////////////////////////////////////////////////////////
PT(OpenALAudioManager::SoundData) OpenALAudioManager::
load(Filename file_name) {
  string raw_data;
  PT(SoundData) sd = new SoundData(this);

  sd->_basename = file_name.get_basename();

  make_current();
  
  PT(MovieAudioCursor) source = MovieAudio::get(file_name)->open();
  if (source == 0) {
    audio_error("Could not load audio file "<<file_name);
    return NULL;
  }

  int channels = source->audio_channels();
  if ((channels != 1)&&(channels != 2)) {
    audio_error("Currently, only mono and stereo are supported.");
    return NULL;
  }    
  int samples = (int)(source->length() * source->audio_rate());
  if (samples > 10000000) {
    audio_error("Sound is too long for loading into RAM.");
    return NULL;
  }

  alGetError(); // clear errors
  sd->_buffer = 0;
  alGenBuffers(1, &sd->_buffer);
  al_audio_errcheck("alGenBuffers");
  if (sd->_buffer == 0) {
    audio_error("Could not create an OpenAL buffer object");
    return NULL;
  }
  
  PN_int16 *data = new PN_int16[samples * channels];
  source->read_samples(samples, data);
  alBufferData(sd->_buffer,
               (channels>1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
               data, samples * channels * 2, source->audio_rate());
  int err = alGetError();
  if (err != AL_NO_ERROR) {
    audio_error("alBufferData: " << alGetString(err));
    alDeleteBuffers(1, &sd->_buffer);
    return NULL;
  }
  audio_debug("Loaded "<<file_name<<" Src Len "<<source->length());
  double len = sd->get_length();
  return sd;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_sound()
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) OpenALAudioManager::
get_sound(const string &file_name, bool positional) {
  audio_debug("OpenALAudioManager::get_sound(file_name=\""<<file_name<<"\")");

  if(!is_valid()) {
     audio_debug("invalid OpenALAudioManager returning NullSound");
     return get_null_sound();
  }

  assert(is_valid());
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_sound_path()) ||
    vfs->resolve_filename(path, get_model_path());
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
      // for the OpenALAudioSound.
      si=ib.first;
    }
  }
  // Create an AudioSound from the sound:
  PT(AudioSound) audioSound = 0;
  if (sd != (SoundData *)NULL) {
    most_recently_used((*si).first);
    PT(OpenALAudioSound) openalAudioSound
        =new OpenALAudioSound(this, sd, (*si).first, positional);
    nassertr(openalAudioSound, 0);
    openalAudioSound->set_active(_active);
    bool inserted = _sounds_on_loan.insert(openalAudioSound).second;
    nassertr(inserted, openalAudioSound.p());
    audioSound=openalAudioSound;
  }

  audio_debug("  returning 0x" << (void*)audioSound);
  assert(is_valid());
  return audioSound;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::uncache_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
uncache_sound(const string& file_name) {
  audio_debug("OpenALAudioManager::uncache_sound(file_name=\""
      <<file_name<<"\")");
  assert(is_valid());
  Filename path = file_name;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_sound_path()) ||
    vfs->resolve_filename(path, get_model_path());

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
//     Function: OpenALAudioManager::uncache_a_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
uncache_a_sound() {
  audio_debug("OpenALAudioManager::uncache_a_sound()");
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
//     Function: OpenALAudioManager::most_recently_used
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
most_recently_used(const string& path) {
  audio_debug("OpenALAudioManager::most_recently_used(path=\""
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
//     Function: OpenALAudioManager::clear_cache
//       Access: Public
//  Description: Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
clear_cache() {
  audio_debug("OpenALAudioManager::clear_cache()");
  if (_is_valid) { assert(is_valid()); }
  _sounds.clear();
  _lru.clear();
  if (_is_valid) { assert(is_valid()); }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_cache_limit
//       Access: Public
//  Description: Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("OpenALAudioManager::set_cache_limit(count="<<count<<")");
  assert(is_valid());
  while (_lru.size() > count) {
    uncache_a_sound();
  }
  _cache_limit=count;
  assert(is_valid());
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_cache_limit
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
unsigned int OpenALAudioManager::
get_cache_limit() const {
  audio_debug("OpenALAudioManager::get_cache_limit() returning "
      <<_cache_limit);
  return _cache_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::release_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
release_sound(OpenALAudioSound* audioSound) {
  audio_debug("OpenALAudioManager::release_sound(audioSound=\""
              <<audioSound->get_name()<<"\"), this = " << (void *)this);
  AudioSet::iterator ai = _sounds_on_loan.find(audioSound);
  nassertv(ai != _sounds_on_loan.end());
  _sounds_on_loan.erase(ai);

  audio_debug("OpenALAudioManager::release_sound() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_volume(float volume)
//       Access: Public
//  Description: 
//        Sets listener gain
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::set_volume(float volume) {
  audio_debug("OpenALAudioManager::set_volume(volume="<<volume<<")");
  if (_volume!=volume) {
    _volume = volume;

    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
      (**i).set_volume((**i).get_volume());
    }

    /* 
    // this was neat alternative to the above look
    // when we had a seperate context for each manager
    make_current();

    alGetError(); // clear errors
    alListenerf(AL_GAIN,(ALfloat)_volume);
    al_audio_errcheck("alListerf(AL_GAIN)");*/
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_volume()
//       Access: Public
//  Description: 
//        Gets listener gain
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::
get_volume() const {
  audio_debug("OpenALAudioManager::get_volume() returning "<<_volume);
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_play_rate
//       Access: Public
//  Description: set the overall play rate
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_play_rate(float play_rate) {
  audio_debug("OpenALAudioManager::set_play_rate(play_rate="<<play_rate<<")");
  if (_play_rate!=play_rate) {
    _play_rate = play_rate;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
      (**i).set_play_rate((**i).get_play_rate());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_play_rate
//       Access: Public
//  Description: get the overall speed/pitch/play rate
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::
get_play_rate() const {
  audio_debug("OpenALAudioManager::get_play_rate() returning "<<_play_rate);
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_active(bool active)
//       Access: Public
//  Description: Turn on/off
//               Warning: not implemented.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_active(bool active) {
  audio_debug("OpenALAudioManager::set_active(flag="<<active<<")");
  if (_active!=active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AudioSet::iterator i=_sounds_on_loan.begin();
    for (; i!=_sounds_on_loan.end(); ++i) {
      (**i).set_active(_active);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_active()
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool OpenALAudioManager::
get_active() const {
  audio_debug("OpenALAudioManager::get_active() returning "<<_active);
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_set_listener_attributes
//       Access: Public
//  Description: Set position of the "ear" that picks up 3d sounds
//        NOW LISTEN UP!!! THIS IS IMPORTANT!
//        Both Panda3D and OpenAL use a right handed coordinate system.
//        But there is a major difference!
//        In Panda3D the Y-Axis is going into the Screen and the Z-Axis is going up.
//        In OpenAL the Y-Axis is going up and the Z-Axis is coming out of the screen.
//        The solution is simple, we just flip the Y and Z axis and negate the Z, as we move coordinates
//        from Panda to OpenAL and back.
//        What does did mean to average Panda user?  Nothing, they shouldn't notice anyway.
//        But if you decide to do any 3D audio work in here you have to keep it in mind.
//        I told you, so you can't say I didn't.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
audio_3d_set_listener_attributes(float px, float py, float pz, float vx, float vy, float vz, float fx, float fy, float fz, float ux, float uy, float uz) {
  _position[0] = px;
  _position[1] = pz;
  _position[2] = -py; 

  _velocity[0] = vx;
  _velocity[1] = vz;
  _velocity[2] = -vy;

  _forward_up[0] = fx;
  _forward_up[1] = fz;
  _forward_up[2] = -fy;

  _forward_up[3] = ux;
  _forward_up[4] = uz;
  _forward_up[5] = -uy;
    
  
  make_current();

  alGetError(); // clear errors
  alListenerfv(AL_POSITION,_position);
  al_audio_errcheck("alListerfv(AL_POSITION)");
  alListenerfv(AL_VELOCITY,_velocity);
  al_audio_errcheck("alListerfv(AL_VELOCITY)");
  alListenerfv(AL_ORIENTATION,_forward_up);
  al_audio_errcheck("alListerfv(AL_ORIENTATION)");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: Get position of the "ear" that picks up 3d sounds
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
audio_3d_get_listener_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz, float *fx, float *fy, float *fz, float *ux, float *uy, float *uz) {
  *px = _position[0];
  *py = -_position[2];
  *pz = _position[1];

  *vx = _velocity[0];
  *vy = -_velocity[2];
  *vz = _velocity[1];

  *fx = _forward_up[0];
  *fy = -_forward_up[2];
  *fz = _forward_up[1];
  
  *ux = _forward_up[3];
  *uy = -_forward_up[5];
  *uz = _forward_up[4];
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: Set units per foot
//               WARNING: OpenAL has no distance factor but we use this as a scale
//                        on the min/max distances of sounds to preserve FMOD compatibility.
//                        Also, adjusts the speed of sound to compensate for unit difference.
//                        OpenAL's default speed of sound is 343.3 m/s == 1126.3 ft/s
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
audio_3d_set_distance_factor(float factor) {
  _distance_factor = factor;

  make_current();

  alGetError(); // clear errors

  if (_distance_factor>0) {
    alSpeedOfSound(1126.3*_distance_factor);
    al_audio_errcheck("alSpeedOfSound()");
    // resets the doppler factor to the correct setting in case it was set to 0.0 by a distance_factor<=0.0
    alDopplerFactor(_doppler_factor);
    al_audio_errcheck("alDopplerFactor()");
  } else {
    audio_debug("can't set speed of sound if distance_factor <=0.0, setting doppler factor to 0.0 instead");
    alDopplerFactor(0.0);
    al_audio_errcheck("alDopplerFactor()");
  }

  AudioSet::iterator i=_sounds_on_loan.begin();
  for (; i!=_sounds_on_loan.end(); ++i) {
    (**i).set_3d_min_distance((**i).get_3d_min_distance());
    (**i).set_3d_max_distance((**i).get_3d_max_distance());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: Sets units per foot
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::
audio_3d_get_distance_factor() const {
  return _distance_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_set_doppler_factor
//       Access: Public
//  Description: Exaggerates or diminishes the Doppler effect. 
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
audio_3d_set_doppler_factor(float factor) {
  _doppler_factor = factor;

  make_current();
  
  alGetError(); // clear errors
  alDopplerFactor(_doppler_factor);
  al_audio_errcheck("alDopplerFactor()");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::
audio_3d_get_doppler_factor() const {
  return _doppler_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_set_drop_off_factor
//       Access: Public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
audio_3d_set_drop_off_factor(float factor) {
  _drop_off_factor = factor;

  AudioSet::iterator i=_sounds_on_loan.begin();
  for (; i!=_sounds_on_loan.end(); ++i) {
    (**i).set_3d_drop_off_factor((**i).get_3d_drop_off_factor());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::
audio_3d_get_drop_off_factor() const {
  return _drop_off_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::starting_sound
//       Access: 
//  Description: Inform the manager that a sound is about to play.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
starting_sound(OpenALAudioSound* audio) {
  ALuint source=0;

  // first give all sounds that have finished a chance to stop, so that these get stopped first
  update();

  if (_concurrent_sound_limit) {
    reduce_sounds_playing_to(_concurrent_sound_limit-1); // because we're about to add one
  }
  
  // get a source from the source pool or create a new source
  if (_al_sources->empty()) {
    make_current();
    alGetError(); // clear errors
    alGenSources(1,&source);
    ALenum result = alGetError();
    if (result!=AL_NO_ERROR) {
      audio_error("alGenSources(): " << alGetString(result) );
      // if we can't create any more sources, set stop a sound to free a source
      reduce_sounds_playing_to(_sounds_playing.size()-1);
      source = 0;
    }
  }
  // get a source from the source bool if we didn't just allocate one
  if (!source && !_al_sources->empty()) {
    source = *(_al_sources->begin());
    _al_sources->erase(source);
  }

  assert(!audio->_source);
  audio->_source = source;

  if (source)
    _sounds_playing.insert(audio);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::stopping_sound
//       Access: 
//  Description: Inform the manager that a sound is finished or 
//               someone called stop on the sound (this should not
//               be called if a sound is only paused).
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
stopping_sound(OpenALAudioSound* audio) {
  _sounds_playing.erase(audio);
  if (audio->_source) {
    _al_sources->insert(audio->_source);
    audio->_source = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  _concurrent_sound_limit = limit;
  reduce_sounds_playing_to(_concurrent_sound_limit);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int OpenALAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::reduce_sounds_playing_to
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  // first give all sounds that have finished a chance to stop, so that these get stopped first
  update();

  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    assert(sound != _sounds_playing.end());
    (**sound).stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::stop_all_sounds()
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
stop_all_sounds() {
  audio_debug("OpenALAudioManager::stop_all_sounds()");
  reduce_sounds_playing_to(0);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::update
//       Access: Public
//  Description: Perform all per-frame update functions.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
update() {
  //audio_debug("OpenALAudioManager::update()");

  // See if any of our playing sounds have ended
  // we must first collect a seperate list of finished sounds and then
  // iterated over those again calling their finished method. We 
  // can't call finished() within a loop iterating over _sounds_playing
  // since finished() modifies _sounds_playing
  SoundsPlaying sounds_finished;

  SoundsPlaying::iterator i=_sounds_playing.begin();
  for (; i!=_sounds_playing.end(); ++i) {
    if ((**i).status()!=AudioSound::PLAYING) {
      sounds_finished.insert(*i);
    }
  }

  i=sounds_finished.begin();
  for (; i!=sounds_finished.end(); ++i) {
    (**i).finished();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::cleanup
//       Access: Private
//  Description: Shuts down the audio manager and releases any
//               resources associated with it.  Also cleans up all
//               AudioSounds created via the manager.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
cleanup() {
  audio_debug("OpenALAudioManager::cleanup(), this = " << (void *)this
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
    if (_openal_active) {
      // empty the source cache
      int i=0;
      ALuint *sources;
      sources = new ALuint[_al_sources->size()];
      for (SourceCache::iterator si = _al_sources->begin(); si!=_al_sources->end(); ++si) {
        sources[i++]=*si;
      }
      make_current();
      alGetError(); // clear errors
      alDeleteSources(_al_sources->size(),sources);
      al_audio_errcheck("alDeleteSources()");
      delete [] sources;
      _al_sources->clear();

      // make sure that the context is not current when it is destroyed
      alcGetError(_device); // clear errors
      alcMakeContextCurrent(NULL);
      alc_audio_errcheck("alcMakeContextCurrent(NULL)",_device);
      
      alcDestroyContext(_context);
      alc_audio_errcheck("alcDestroyContext(_context)",_device);
      _context = NULL;

      if (_device) {
        alcCloseDevice(_device);
        //alc_audio_errcheck("alcCloseDevice(_device)",_device);
        _device = NULL;
      }

      _openal_active = false;
    }
    /*if (_managers) {
      delete _managers;
      _managers = NULL;
      delete _al_sources;
      _al_sources = NULL;
    }*/
  }
  _cleanup_required = false;
  audio_debug("OpenALAudioManager::cleanup() finished");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::SoundData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::SoundData::
SoundData(OpenALAudioManager* manager) :
  _manager(manager),
  _buffer(0),
  _has_length(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::SoundData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::SoundData::
~SoundData() {
  if (_buffer != 0) {
    if (_manager->_is_valid) {
      _manager->make_current();
      alDeleteBuffers(1,&_buffer);
    }
    _buffer = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::SoundData::get_length
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float OpenALAudioManager::SoundData::
get_length() {
  int freq,bits,channels,size;

  if (!_has_length) {
    // Time to determine the length of the file.

    audio_debug("Computing length of " << _basename);

    _manager->make_current();

    alGetError(); // clear errors
    alGetBufferi(_buffer,AL_FREQUENCY,&freq);
    audio_debug("Frequency = "<<freq);
    al_audio_errcheck("alGetBufferi(_buffer,AL_FREQUENCY)");
    alGetBufferi(_buffer,AL_BITS,&bits);
    audio_debug("Bits = "<<bits);
    al_audio_errcheck("alGetBufferi(_buffer,AL_BITS)");
    alGetBufferi(_buffer,AL_CHANNELS,&channels);
    audio_debug("Channels = "<<channels);
    al_audio_errcheck("alGetBufferi(_buffer,AL_CHANNELS)");
    alGetBufferi(_buffer,AL_SIZE,&size);
    audio_debug("Size = "<<size);
    al_audio_errcheck("alGetBufferi(_buffer,AL_SIZE)");
  
    _length = ((float)size)/channels/((float)bits/8)/freq;
    audio_debug("Length = "<<_length);
    _has_length = true;
  }

  return _length;
}

#endif //]
