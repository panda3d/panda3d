// Filename: openalAudioManager.cxx
// Created by:  Ben Buchwald <bb2@alumni.cmu.edu>
//
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

#include "pandabase.h"

//Panda headers.
#include "config_audio.h"
#include "config_util.h"
#include "config_express.h"
#include "openalAudioManager.h"
#include "openalAudioSound.h"
#include "virtualFileSystem.h"
#include "movieAudio.h"
#include "reMutexHolder.h"

#include <algorithm>

TypeHandle OpenALAudioManager::_type_handle;

ReMutex OpenALAudioManager::_lock;
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
//     Function: Create_OpenALAudioManager
//       Access: Private
//  Description: Factory Function
////////////////////////////////////////////////////////////////////
AudioManager *Create_OpenALAudioManager() {
  audio_debug("Create_OpenALAudioManager()");
  return new OpenALAudioManager;
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::
OpenALAudioManager() {
  ReMutexHolder holder(_lock);
  if (_managers == (Managers *)NULL) {
    _managers = new Managers;
    _al_sources = new SourceCache;
  }

  _managers->insert(this);

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
  if (_active_managers == 0 || !_openal_active) {
    _device = alcOpenDevice(NULL); // select the "preferred device"
    if (!_device) {
      // this is a unique kind of error
      audio_error("OpenALAudioManager: alcOpenDevice(NULL): ALC couldn't open device");
    } else {
      alcGetError(_device); // clear errors
      _context = alcCreateContext(_device, NULL);
      alc_audio_errcheck("alcCreateContext(_device, NULL)",_device);
      if (_context != NULL) {
        _openal_active = true;
      }
    }
  }
  // We increment _active_managers regardless of possible errors above.
  // The shutdown call will do the right thing when it's called,
  // either way.
  ++_active_managers;
  nassertv(_active_managers>0);

  if (!_device || !_context) {
    audio_error("OpenALAudioManager: No open device or context");
    _is_valid = false;
  } else {
    alcGetError(_device); // clear errors
    alcMakeContextCurrent(_context);
    alc_audio_errcheck("alcMakeContextCurrent(_context)", _device);

    // set 3D sound characteristics as they are given in the configrc
    audio_3d_set_doppler_factor(audio_doppler_factor);
    audio_3d_set_distance_factor(audio_distance_factor);
    audio_3d_set_drop_off_factor(audio_drop_off_factor);

    if (audio_cat.is_debug()) {
      audio_cat->debug()
        << "ALC_DEVICE_SPECIFIER:" << alcGetString(_device, ALC_DEVICE_SPECIFIER) << endl;
    }
  }

  if (audio_cat.is_debug()) {
    audio_cat->debug() << "AL_RENDERER:" << alGetString(AL_RENDERER) << endl;
    audio_cat->debug() << "AL_VENDOR:" << alGetString(AL_VENDOR) << endl;
    audio_cat->debug() << "AL_VERSION:" << alGetString(AL_VERSION) << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::
~OpenALAudioManager() {
  ReMutexHolder holder(_lock);
  nassertv(_managers != (Managers *)NULL);
  Managers::iterator mi = _managers->find(this);
  nassertv(mi != _managers->end());
  _managers->erase(mi);
  cleanup();
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
  ReMutexHolder holder(_lock);
  if (_managers != (Managers *)NULL) {
    Managers::iterator mi;
    for (mi = _managers->begin(); mi != _managers->end(); ++mi) {
      (*mi)->cleanup();
    }
  }

  nassertv(_active_managers == 0);
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
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::make_current
//       Access: Private
//  Description: This makes this manager's OpenAL context the 
//         current context. Needed before any parameter sets.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
make_current() const {
  // Since we only use one context, this is now a no-op.
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::can_use_audio
//       Access: Private
//  Description: Returns true if the specified MovieAudioCursor
//               can be used by this AudioManager.  Mostly, this
//               involves checking whether or not the format is
//               implemented/supported.
////////////////////////////////////////////////////////////////////
bool OpenALAudioManager::
can_use_audio(MovieAudioCursor *source) {
  ReMutexHolder holder(_lock);
  int channels = source->audio_channels();
  if ((channels != 1)&&(channels != 2)) {
    audio_error("Currently, only mono and stereo are supported.");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::should_load_audio
//       Access: Private
//  Description: Returns true if the specified MovieAudio should be
//               cached into RAM.  A lot of conditions have to be met
//               in order to allow caching - if any are not met,
//               the file will be streamed.
////////////////////////////////////////////////////////////////////
bool OpenALAudioManager::
should_load_audio(MovieAudioCursor *source, int mode) {
  ReMutexHolder holder(_lock);
  if (mode == SM_stream) {
    // If the user asked for streaming, give him streaming.
    return false;
  }
  if (source->get_source()->get_filename().empty()) {
    // Non-files cannot be preloaded.
    return false;
  }
  if (source->ready() != 0x40000000) {
    // Streaming sources cannot be preloaded.
    return false;
  }
  if (source->length() > 3600.0) {
    // Anything longer than an hour cannot be preloaded.
    return false;
  }
  int channels = source->audio_channels();
  int samples = (int)(source->length() * source->audio_rate());
  int bytes = samples * channels * 2;
  if ((mode == SM_heuristic)&&(bytes > audio_preload_threshold)) {
    // In heuristic mode, if file is long, stream it.
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_sound_data
//       Access: Private
//  Description: Obtains a SoundData for the specified sound.
//
//               When you are done with the SoundData, you need
//               to decrement the client count.
////////////////////////////////////////////////////////////////////
OpenALAudioManager::SoundData *OpenALAudioManager::
get_sound_data(MovieAudio *movie, int mode) {
  ReMutexHolder holder(_lock);
  const Filename &path = movie->get_filename();
  
  // Search for an already-cached sample or an already-opened stream.
  if (!path.empty()) {
    
    if (mode != SM_stream) {
      SampleCache::iterator lsmi=_sample_cache.find(path);
      if (lsmi != _sample_cache.end()) {
        SoundData *sd = (*lsmi).second;
        increment_client_count(sd);
        return sd;
      }
    }

    if (mode != SM_sample) {
      ExpirationQueue::iterator exqi;
      for (exqi=_expiring_streams.begin(); exqi!=_expiring_streams.end(); exqi++) {
        SoundData *sd = (SoundData*)(*exqi);
        if (sd->_movie->get_filename() == path) {
          increment_client_count(sd);
          return sd;
        }
      }
    }
  }
  
  PT(MovieAudioCursor) stream = movie->open();
  if (stream == 0) {
    audio_error("Cannot open file: "<<path);
    return NULL;
  }
  
  if (!can_use_audio(stream)) {
    audio_error("File is not in usable format: "<<path);
    return NULL;
  }
  
  SoundData *sd = new SoundData();
  sd->_client_count = 1;
  sd->_manager  = this;
  sd->_movie    = movie;
  sd->_rate     = stream->audio_rate();
  sd->_channels = stream->audio_channels();
  sd->_length   = stream->length();
  audio_debug("Creating: " << sd->_movie->get_filename().get_basename());
  audio_debug("  - Rate: " << sd->_rate);
  audio_debug("  - Channels: " << sd->_channels);
  audio_debug("  - Length: " << sd->_length);

  if (should_load_audio(stream, mode)) {
    audio_debug(path.get_basename() << ": loading as sample");
    make_current();
    alGetError(); // clear errors
    sd->_sample = 0;
    alGenBuffers(1, &sd->_sample);
    al_audio_errcheck("alGenBuffers");
    if (sd->_sample == 0) {
      audio_error("Could not create an OpenAL buffer object");
      delete sd;
      return NULL;
    }
    int channels = stream->audio_channels();
    int samples = (int)(stream->length() * stream->audio_rate());
    PN_int16 *data = new PN_int16[samples * channels];
    stream->read_samples(samples, data);
    alBufferData(sd->_sample,
                 (channels>1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
                 data, samples * channels * 2, stream->audio_rate());
    int err = alGetError();
    if (err != AL_NO_ERROR) {
      audio_error("could not fill OpenAL buffer object with data");
      delete sd;
      return NULL;
    }
    _sample_cache.insert(SampleCache::value_type(path, sd));
  } else {
    audio_debug(path.get_basename() << ": loading as stream");
    sd->_stream = stream;
  }
  
  return sd;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_sound
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) OpenALAudioManager::
get_sound(MovieAudio *sound, bool positional, int mode) {
  ReMutexHolder holder(_lock);
  if(!is_valid()) {
    return get_null_sound();
  }
  PT(OpenALAudioSound) oas = 
    new OpenALAudioSound(this, sound, positional, mode);
  
  _all_sounds.insert(oas);
  PT(AudioSound) res = (AudioSound*)(OpenALAudioSound*)oas;
  return res;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_sound
//       Access: Public
//  Description: This is what creates a sound instance.
////////////////////////////////////////////////////////////////////
PT(AudioSound) OpenALAudioManager::
get_sound(const string &file_name, bool positional, int mode) {
  ReMutexHolder holder(_lock);
  if(!is_valid()) {
    return get_null_sound();
  }
  
  Filename path = file_name;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());
  
  if (path.empty()) {
    audio_error("get_sound - invalid filename");
    return NULL;
  }

  PT(MovieAudio) mva = MovieAudio::get(path);
  
  PT(OpenALAudioSound) oas = 
    new OpenALAudioSound(this, mva, positional, mode);
  
  _all_sounds.insert(oas);
  PT(AudioSound) res = (AudioSound*)(OpenALAudioSound*)oas;
  return res;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::uncache_sound
//       Access: Public
//  Description: Deletes a sample from the expiration queues.
//               If the sound is actively in use, then the sound
//               cannot be deleted, and this function has no effect.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
uncache_sound(const string& file_name) {
  ReMutexHolder holder(_lock);
  assert(is_valid());
  Filename path = file_name;
  
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  SampleCache::iterator sci = _sample_cache.find(path);
  if (sci != _sample_cache.end()) {
    SoundData *sd = (*sci).second;
    if (sd->_client_count == 0) {
      _expiring_samples.erase(sd->_expire);
      _sample_cache.erase(sci);
      delete sd;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::clear_cache
//       Access: Public
//  Description: Clear out the sound cache.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
clear_cache() {
  ReMutexHolder holder(_lock);
  discard_excess_cache(0);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_cache_limit
//       Access: Public
//  Description: Set the number of sounds that the cache can hold.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_cache_limit(unsigned int count) {
  ReMutexHolder holder(_lock);
  _cache_limit=count;
  discard_excess_cache(count);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_cache_limit
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
unsigned int OpenALAudioManager::
get_cache_limit() const {
  return _cache_limit;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::release_sound
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
release_sound(OpenALAudioSound* audioSound) {
  ReMutexHolder holder(_lock);
  AllSounds::iterator ai = _all_sounds.find(audioSound);
  if (ai != _all_sounds.end()) {
    _all_sounds.erase(ai);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_volume(PN_stdfloat volume)
//       Access: Public
//  Description: 
//        Sets listener gain
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(_lock);
  if (_volume!=volume) {
    _volume = volume;

    // Tell our AudioSounds to adjust:
    AllSounds::iterator i=_all_sounds.begin();
    for (; i!=_all_sounds.end(); ++i) {
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
PN_stdfloat OpenALAudioManager::
get_volume() const {
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_play_rate
//       Access: Public
//  Description: set the overall play rate
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_play_rate(PN_stdfloat play_rate) {
  ReMutexHolder holder(_lock);
  if (_play_rate!=play_rate) {
    _play_rate = play_rate;
    // Tell our AudioSounds to adjust:
    AllSounds::iterator i=_all_sounds.begin();
    for (; i!=_all_sounds.end(); ++i) {
      (**i).set_play_rate((**i).get_play_rate());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::get_play_rate
//       Access: Public
//  Description: get the overall speed/pitch/play rate
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioManager::
get_play_rate() const {
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
  ReMutexHolder holder(_lock);
  if (_active!=active) {
    _active=active;
    // Tell our AudioSounds to adjust:
    AllSounds::iterator i=_all_sounds.begin();
    for (; i!=_all_sounds.end(); ++i) {
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
audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  ReMutexHolder holder(_lock);
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
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
  ReMutexHolder holder(_lock);
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
audio_3d_set_distance_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
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

  AllSounds::iterator i=_all_sounds.begin();
  for (; i!=_all_sounds.end(); ++i) {
    (**i).set_3d_min_distance((**i).get_3d_min_distance());
    (**i).set_3d_max_distance((**i).get_3d_max_distance());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: Sets units per foot
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioManager::
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
audio_3d_set_doppler_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
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
PN_stdfloat OpenALAudioManager::
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
audio_3d_set_drop_off_factor(PN_stdfloat factor) {
  ReMutexHolder holder(_lock);
  _drop_off_factor = factor;

  AllSounds::iterator i=_all_sounds.begin();
  for (; i!=_all_sounds.end(); ++i) {
    (**i).set_3d_drop_off_factor((**i).get_3d_drop_off_factor());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioManager::
audio_3d_get_drop_off_factor() const {
  return _drop_off_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::starting_sound
//       Access: 
//  Description: Inform the manager that a sound is about to play.
//               The manager will add this sound to the table of
//               sounds that are playing, and will allocate a source
//               to this sound.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
starting_sound(OpenALAudioSound* audio) {
  ReMutexHolder holder(_lock);
  ALuint source=0;
  
  // If the sound already has a source, we don't need to do anything.
  if (audio->_source) {
    return;
  }

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
  ReMutexHolder holder(_lock);
  if (audio->_source) {
    _al_sources->insert(audio->_source);
    audio->_source = 0;
  }
  _sounds_playing.erase(audio); // This could cause the sound to destruct.
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  ReMutexHolder holder(_lock);
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
  ReMutexHolder holder(_lock);
  // first give all sounds that have finished a chance to stop, so that these get stopped first
  update();

  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    assert(sound != _sounds_playing.end());
    // When the user stops a sound, there is still a PT in the
    // user's hand.  When we stop a sound here, however, 
    // this can remove the last PT.  This can cause an ugly
    // recursion where stop calls the destructor, and the
    // destructor calls stop.  To avoid this, we create
    // a temporary PT, stop the sound, and then release the PT.
    PT(OpenALAudioSound) s = (*sound);
    s->stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::stop_all_sounds()
//       Access: Public
//  Description: Stop playback on all sounds managed by this manager.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
stop_all_sounds() {
  ReMutexHolder holder(_lock);
  reduce_sounds_playing_to(0);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::update
//       Access: Public
//  Description: Perform all per-frame update functions.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
update() {
  ReMutexHolder holder(_lock);

  // See if any of our playing sounds have ended
  // we must first collect a seperate list of finished sounds and then
  // iterated over those again calling their finished method. We 
  // can't call finished() within a loop iterating over _sounds_playing
  // since finished() modifies _sounds_playing
  SoundsPlaying sounds_finished;

  double rtc = TrueClock::get_global_ptr()->get_short_time();
  SoundsPlaying::iterator i=_sounds_playing.begin();
  for (; i!=_sounds_playing.end(); ++i) {
    OpenALAudioSound *sound = (*i);
    sound->pull_used_buffers();
    sound->push_fresh_buffers();
    sound->restart_stalled_audio();
    sound->cache_time(rtc);
    if ((sound->_source == 0)||
        ((sound->_stream_queued.size() == 0)&&
         (sound->_loops_completed >= sound->_playing_loops))) {
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
  ReMutexHolder holder(_lock);
  if (!_cleanup_required) {
    return;
  }

  stop_all_sounds();
  
  AllSounds sounds(_all_sounds);
  AllSounds::iterator ai;
  for (ai = sounds.begin(); ai != sounds.end(); ++ai) {
    (*ai)->cleanup();
  }
  
  clear_cache();
  
  nassertv(_active_managers > 0);
  --_active_managers;

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
        audio_debug("Going to try to close openAL");
        alcCloseDevice(_device);
        //alc_audio_errcheck("alcCloseDevice(_device)",_device);
        _device = NULL;
        audio_debug("openAL Closed");
      }

      _openal_active = false;
    }
  }
  _cleanup_required = false;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::SoundData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::SoundData::
SoundData() :
  _manager(0),
  _movie(0),
  _sample(0),
  _stream(NULL),
  _length(0.0),
  _rate(0),
  _channels(0),
  _client_count(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::SoundData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioManager::SoundData::
~SoundData() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_sample != 0) {
    if (_manager->_is_valid) {
      _manager->make_current();
      alDeleteBuffers(1,&_sample);
    }
    _sample = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::increment_client_count
//       Access: Public
//  Description: Increments the SoundData's client count.  Any
//               SoundData that is actively in use (ie, has a client)
//               is removed entirely from the expiration queue.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
increment_client_count(SoundData *sd) {
  ReMutexHolder holder(_lock);
  sd->_client_count += 1;
  audio_debug("Incrementing: " << sd->_movie->get_filename().get_basename() << " " << sd->_client_count);
  if (sd->_client_count == 1) {
    if (sd->_sample) {
      _expiring_samples.erase(sd->_expire);
    } else {
      _expiring_streams.erase(sd->_expire);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::decrement_client_count
//       Access: Public
//  Description: Decrements the SoundData's client count.  Sounds
//               that are no longer in use (ie, have no clients)
//               go into the expiration queue.  When the expiration
//               queue reaches the cache limit, the first item on
//               the queue is freed.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
decrement_client_count(SoundData *sd) {
  ReMutexHolder holder(_lock);
  sd->_client_count -= 1;
  audio_debug("Decrementing: " << sd->_movie->get_filename().get_basename() << " " << sd->_client_count);
  if (sd->_client_count == 0) {
    if (sd->_sample) {
      _expiring_samples.push_back(sd);
      sd->_expire = _expiring_samples.end();
      sd->_expire--;
    } else {
      _expiring_streams.push_back(sd);
      sd->_expire = _expiring_streams.end();
      sd->_expire--;
    }
    discard_excess_cache(_cache_limit);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioManager::discard_excess_cache
//       Access: Public
//  Description: Discards sounds from the sound cache until the
//               number of sounds remaining is under the limit.
////////////////////////////////////////////////////////////////////
void OpenALAudioManager::
discard_excess_cache(int sample_limit) {
  ReMutexHolder holder(_lock);
  int stream_limit = 5;

  while (((int)_expiring_samples.size()) > sample_limit) {
    SoundData *sd = (SoundData*)(_expiring_samples.front());
    assert(sd->_client_count == 0);
    assert(sd->_expire == _expiring_samples.begin());
    _expiring_samples.pop_front();
    _sample_cache.erase(_sample_cache.find(sd->_movie->get_filename()));
    audio_debug("Expiring: " << sd->_movie->get_filename().get_basename());
    delete sd;
  }

  while (((int)_expiring_streams.size()) > stream_limit) {
    SoundData *sd = (SoundData*)(_expiring_streams.front());
    assert(sd->_client_count == 0);
    assert(sd->_expire == _expiring_streams.begin());
    _expiring_streams.pop_front();
    audio_debug("Expiring: " << sd->_movie->get_filename().get_basename());
    delete sd;
  }
}
