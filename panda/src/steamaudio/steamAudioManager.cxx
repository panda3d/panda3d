/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioManager.cxx
 * @author Jackson Sutherland
 */

#include "pandabase.h"

 // Panda headers.
#include "config_audio.h"
#include "config_putil.h"
#include "config_express.h"
#include "config_openalAudio.h"
#include "steamAudioManager.h"
#include "openalAudioSound.h"
#include "virtualFileSystem.h"
#include "movieAudio.h"
#include "reMutexHolder.h"

#include <algorithm>

//Since this implementation also uses OpenAl
#ifndef ALC_DEFAULT_ALL_DEVICES_SPECIFIER
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER 0x1012
#endif

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER 0x1013
#endif

using std::endl;
using std::string;

TypeHandle SteamAudioManager::_type_handle;

ReMutex SteamAudioManager::_steamLock;

// This is the list of all SteamAudioManager objects in the world.  It must
// be a pointer rather than a concrete object, so it won't be destructed at
// exit time before we're done removing things from it.
SteamAudioManager::Managers* SteamAudioManager::_managers = nullptr;

/**
 *
 */
SteamAudioManager::
SteamAudioManager() {
  ReMutexHolder holder(_lock);
  if (_managers == nullptr) {
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

  // Init 3D attributes
  _distance_factor = 1;
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
  audio_cat.init();
  if (_active_managers == 0 || !_openal_active) {
    _device = nullptr;
    string dev_name = select_audio_device();

    if (!dev_name.empty()) {
      // Open a specific device by name.
      audio_cat.info() << "Using OpenAL device " << dev_name << "\n";
      _device = alcOpenDevice(dev_name.c_str());

      if (_device == nullptr) {
        audio_cat.error()
          << "Couldn't open OpenAL device \"" << dev_name << "\", falling back to default device\n";
      }
    }
    else {
      audio_cat.info() << "Using default OpenAL device\n";
    }

    if (_device == nullptr) {
      // Open the default device.
      _device = alcOpenDevice(nullptr);

      if (_device == nullptr && dev_name != "OpenAL Soft") {
        // Try the OpenAL Soft driver instead, which is fairly reliable.
        _device = alcOpenDevice("OpenAL Soft");

        if (_device == nullptr) {
          audio_cat.error()
            << "Couldn't open default OpenAL device\n";
        }
      }
    }

    if (_device != nullptr) {
      // We managed to get a device open.
      alcGetError(_device); // clear errors
      _context = alcCreateContext(_device, nullptr);
      alc_audio_errcheck("alcCreateContext(_device, NULL)", _device);
      if (_context != nullptr) {
        _openal_active = true;
      }
    }
  }

  // We increment _active_managers regardless of possible errors above.  The
  // shutdown call will do the right thing when it's called, either way.
  ++_active_managers;
  nassertv(_active_managers > 0);

  if (!_device || !_context) {
    audio_error("SteamAudioManager: No open device or context");
    _is_valid = false;
  }
  else {
    alcGetError(_device); // clear errors
    alcMakeContextCurrent(_context);
    alc_audio_errcheck("alcMakeContextCurrent(_context)", _device);

    // set 3D sound characteristics as they are given in the configrc
    audio_3d_set_doppler_factor(audio_doppler_factor);
    audio_3d_set_distance_factor(audio_distance_factor);
    audio_3d_set_drop_off_factor(audio_drop_off_factor);

    if (audio_cat.is_debug()) {
      audio_cat.debug()
        << "ALC_DEVICE_SPECIFIER:" << alcGetString(_device, ALC_DEVICE_SPECIFIER) << endl;
    }
  }

  if (audio_cat.is_debug()) {
    audio_cat.debug() << "AL_RENDERER:" << alGetString(AL_RENDERER) << endl;
    audio_cat.debug() << "AL_VENDOR:" << alGetString(AL_VENDOR) << endl;
    audio_cat.debug() << "AL_VERSION:" << alGetString(AL_VERSION) << endl;
  }
}

/**
 *
 */
SteamAudioManager::
~SteamAudioManager() {
  ReMutexHolder holder(_lock);
  nassertv(_managers != nullptr);
  Managers::iterator mi = _managers->find(this);
  nassertv(mi != _managers->end());
  _managers->erase(mi);
  cleanup();
}

/**
 * Call this at exit time to shut down the audio system.  This will invalidate
 * all currently-active AudioManagers and AudioSounds in the system.  If you
 * change your mind and want to play sounds again, you will have to recreate
 * all of these objects.
 */
void SteamAudioManager::
shutdown() {
  ReMutexHolder holder(_lock);
  if (_managers != nullptr) {
    Managers::iterator mi;
    for (mi = _managers->begin(); mi != _managers->end(); ++mi) {
      (*mi)->cleanup();
    }
  }

  nassertv(_active_managers == 0);
}


/**
 * This is mostly for debugging, but it it could be used to detect errors in a
 * release build if you don't mind the cpu cost.
 */
bool SteamAudioManager::
is_valid() {
  return _is_valid;
}

/**
 * Enumerate the audio devices, selecting the one that is most appropriate or
 * has been selected by the user.
 */
string SteamAudioManager::
select_audio_device() {
  string selected_device = openal_device;

  const char* devices = nullptr;

  // This extension gives us all audio paths on all drivers.
  if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE) {
    string default_device = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
    devices = (const char*)alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

    if (devices) {
      if (audio_cat.is_debug()) {
        audio_cat.debug() << "All OpenAL devices:\n";
      }

      while (*devices) {
        string device(devices);
        devices += device.size() + 1;

        if (audio_cat.is_debug()) {
          if (device == selected_device) {
            audio_cat.debug() << "  " << device << " [selected]\n";
          }
          else if (device == default_device) {
            audio_cat.debug() << "  " << device << " [default]\n";
          }
          else {
            audio_cat.debug() << "  " << device << "\n";
          }
        }
      }
    }
  }
  else {
    audio_cat.debug() << "ALC_ENUMERATE_ALL_EXT not supported\n";
  }

  // This extension just gives us generic driver names, like "OpenAL Soft" and
  // "Generic Software", rather than individual outputs.
  if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT") == AL_TRUE) {
    string default_device = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
    devices = (const char*)alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

    if (devices) {
      if (audio_cat.is_debug()) {
        audio_cat.debug() << "OpenAL drivers:\n";
      }

      while (*devices) {
        string device(devices);
        devices += device.size() + 1;

        if (selected_device.empty() && device == "OpenAL Soft" &&
          default_device == "Generic Software") {
          // Prefer OpenAL Soft over the buggy Generic Software driver.
          selected_device = "OpenAL Soft";
        }

        if (audio_cat.is_debug()) {
          if (device == selected_device) {
            audio_cat.debug() << "  " << device << " [selected]\n";
          }
          else if (device == default_device) {
            audio_cat.debug() << "  " << device << " [default]\n";
          }
          else {
            audio_cat.debug() << "  " << device << "\n";
          }
        }
      }
    }
  }
  else {
    audio_cat.debug() << "ALC_ENUMERATION_EXT not supported\n";
  }

  return selected_device;
}

/**
 * Returns true if the specified MovieAudioCursor can be used by this
 * AudioManager.  Mostly, this involves checking whether or not the format is
 * implemented/supported.
 */
bool SteamAudioManager::
can_use_audio(MovieAudioCursor* source) {
  ReMutexHolder holder(_lock);
  int channels = source->audio_channels();
  if ((channels != 1) && (channels != 2)) {
    audio_error("Currently, only mono and stereo are supported.");
    return false;
  }
  //TODO::add (temporary) check that sample rate matches our config value
  return true;
}

/**
 * Returns true if the specified MovieAudio should be cached into RAM.  A lot
 * of conditions have to be met in order to allow caching - if any are not
 * met, the file will be streamed.
 */
bool SteamAudioManager::
should_load_audio(MovieAudioCursor* source, int mode) {
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
  if ((mode == SM_heuristic) && (bytes > audio_preload_threshold)) {
    // In heuristic mode, if file is long, stream it.
    return false;
  }
  return true;
}

/**
 * Obtains a SoundData for the specified sound.
 *
 * When you are done with the SoundData, you need to decrement the client
 * count.
 */
SteamAudioManager::SoundData* SteamAudioManager::
get_sound_data(MovieAudio* movie, int mode) {
  ReMutexHolder holder(_lock);
  const Filename& path = movie->get_filename();

  // Search for an already-cached sample or an already-opened stream.
  if (!path.empty()) {

    if (mode != SM_stream) {
      SampleCache::iterator lsmi = _sample_cache.find(path);
      if (lsmi != _sample_cache.end()) {
        SoundData* sd = (*lsmi).second;
        increment_client_count(sd);
        return sd;
      }
    }

    if (mode != SM_sample) {
      ExpirationQueue::iterator exqi;
      for (exqi = _expiring_streams.begin(); exqi != _expiring_streams.end(); exqi++) {
        SoundData* sd = (SoundData*)(*exqi);
        if (sd->_movie->get_filename() == path) {
          increment_client_count(sd);
          return sd;
        }
      }
    }
  }

  PT(MovieAudioCursor) stream = movie->open();
  if (stream == nullptr) {
    audio_error("Cannot open file: " << path);
    return nullptr;
  }

  if (!can_use_audio(stream)) {
    audio_error("File is not in usable format: " << path);
    return nullptr;
  }

  SoundData* sd = new SoundData();
  sd->_client_count = 1;
  sd->_manager = this;
  sd->_movie = movie;
  sd->_rate = stream->audio_rate();
  sd->_channels = stream->audio_channels();
  sd->_length = stream->length();
  audio_debug("Creating: " << sd->_movie->get_filename().get_basename());
  audio_debug("  - Rate: " << sd->_rate);
  audio_debug("  - Channels: " << sd->_channels);
  audio_debug("  - Length: " << sd->_length);

  if (should_load_audio(stream, mode)) {//Todo:: should I just disable this? Steam Audio is mostely useless without the ability to update audio in real time...
    audio_debug(path.get_basename() << ": loading as sample");
    make_current();
    alGetError(); // clear errors
    sd->_sample = 0;
    alGenBuffers(1, &sd->_sample);
    al_audio_errcheck("alGenBuffers");
    if (sd->_sample == 0) {
      audio_error("Could not create an OpenAL buffer object");
      delete sd;
      return nullptr;
    }
    int channels = stream->audio_channels();
    int samples = (int)(stream->length() * stream->audio_rate());
    int16_t* data = new int16_t[samples * channels];
    samples = stream->read_samples(samples, data);
    alBufferData(sd->_sample,
      (channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
      data, samples * channels * 2, stream->audio_rate());
    delete[] data;
    int err = alGetError();
    if (err != AL_NO_ERROR) {
      audio_error("could not fill OpenAL buffer object with data");
      delete sd;
      return nullptr;
    }
    _sample_cache.insert(SampleCache::value_type(path, sd));
  }
  else {
    audio_debug(path.get_basename() << ": loading as stream");
    sd->_stream = stream;
  }

  return sd;
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) SteamAudioManager::
get_sound(MovieAudio* sound, bool positional, int mode) {
  ReMutexHolder holder(_lock);
  if (!is_valid()) {
    return get_null_sound();
  }
  PT(OpenALAudioSound) oas =
    new OpenALAudioSound(this, sound, positional, mode);

  if (!oas->_manager) {
    // The sound cleaned itself up immediately. It pretty clearly didn't like
    // something, so we should just return a null sound instead.
    return get_null_sound();
  }

  _all_sounds.insert(oas);
  PT(SteamAudioSound) res = (SteamAudioSound*)oas;
  return res;
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) SteamAudioManager::
get_sound(const Filename& file_name, bool positional, int mode) {
  ReMutexHolder holder(_lock);
  if (!is_valid()) {
    return get_null_sound();
  }

  Filename path = file_name;
  VirtualFileSystem* vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  if (path.empty()) {
    audio_error("get_sound - invalid filename");
    return nullptr;
  }

  PT(MovieAudio) mva = MovieAudio::get(path);

  PT(OpenALAudioSound) oas =
    new OpenALAudioSound(this, mva, positional, mode);

  if (!oas->_manager) {
    // The sound cleaned itself up immediately. It pretty clearly didn't like
    // something, so we should just return a null sound instead.
    return get_null_sound();
  }

  _all_sounds.insert(oas);
  PT(SteamAudioSound) res = (SteamAudioSound*)oas;//Since This doesn't work like other kinds of AudioManagers, we return a distinct kind of pointer.
  return res;
}

/**
 * Deletes a sample from the expiration queues.  If the sound is actively in
 * use, then the sound cannot be deleted, and this function has no effect.
 */
void SteamAudioManager::
uncache_sound(const Filename& file_name) {
  ReMutexHolder holder(_lock);
  nassertv(is_valid());
  Filename path = file_name;

  VirtualFileSystem* vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());

  SampleCache::iterator sci = _sample_cache.find(path);
  if (sci == _sample_cache.end()) {
    sci = _sample_cache.find(file_name);
  }
  if (sci != _sample_cache.end()) {
    SoundData* sd = (*sci).second;
    if (sd->_client_count == 0) {
      _expiring_samples.erase(sd->_expire);
      _sample_cache.erase(sci);
      delete sd;
    }
  }

  ExpirationQueue::iterator exqi;
  for (exqi = _expiring_streams.begin(); exqi != _expiring_streams.end();) {
    SoundData* sd = (SoundData*)(*exqi);
    if (sd->_client_count == 0) {
      if (sd->_movie->get_filename() == path ||
        sd->_movie->get_filename() == file_name) {
        exqi = _expiring_streams.erase(exqi);
        delete sd;
        continue;
      }
    }
    ++exqi;
  }
}

/**
 * Clear out the sound cache.
 */
void SteamAudioManager::
clear_cache() {
  ReMutexHolder holder(_lock);
  discard_excess_cache(0);
}

/**
 *
 */
void SteamAudioManager::
release_sound(OpenALAudioSound* audioSound) {
  ReMutexHolder holder(_lock);
  AllSounds::iterator ai = _all_sounds.find(audioSound);
  if (ai != _all_sounds.end()) {
    _all_sounds.erase(ai);
  }
}

/**
 * Inform the manager that a sound is about to play.  The manager will add
 * this sound to the table of sounds that are playing, and will allocate a
 * source to this sound.
 */
void SteamAudioManager::
starting_sound(OpenALAudioSound* audio) {
  ReMutexHolder holder(_lock);
  ALuint source = 0;

  // If the sound already has a source, we don't need to do anything.
  if (audio->_source) {
    return;
  }

  // first give all sounds that have finished a chance to stop, so that these
  // get stopped first
  update();

  if (_concurrent_sound_limit) {
    reduce_sounds_playing_to(_concurrent_sound_limit - 1); // because we're about to add one
  }

  // get a source from the source pool or create a new source
  if (_al_sources->empty()) {
    make_current();
    alGetError(); // clear errors
    alGenSources(1, &source);
    ALenum result = alGetError();
    if (result != AL_NO_ERROR) {
      audio_error("alGenSources(): " << alGetString(result));
      // if we can't create any more sources, set stop a sound to free a
      // source
      reduce_sounds_playing_to(_sounds_playing.size() - 1);
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

/**
 * Inform the manager that a sound is finished or someone called stop on the
 * sound (this should not be called if a sound is only paused).
 */
void SteamAudioManager::
stopping_sound(OpenALAudioSound* audio) {
  ReMutexHolder holder(_lock);
  if (audio->_source) {
    _al_sources->insert(audio->_source);
    audio->_source = 0;
  }
  _sounds_playing.erase(audio); // This could cause the sound to destruct.
}

/**
 * Perform all per-frame update functions.
 */
void SteamAudioManager::
update() {
  ReMutexHolder const holder(_lock);

  // See if any of our playing sounds have ended.
  double const rtc = TrueClock::get_global_ptr()->get_short_time();
  SoundsPlaying::iterator i = _sounds_playing.begin();
  while (i != _sounds_playing.end()) {
    // The post-increment syntax is *very* important here.
    // As both OpenALAudioSound::pull_used_buffers and OpenALAudioSound::finished can modify the list of sounds playing
    // by erasing 'sound' from the list, thus invaliding the iterator.
    PT(OpenALAudioSound) sound = *(i++);
    sound->pull_used_buffers();

    // If pull_used_buffers() encountered an error, the sound was cleaned up.
    // No need to do anymore work.
    if (!sound->is_valid()) {
      continue;
    }

    sound->push_fresh_buffers();
    sound->restart_stalled_audio();
    sound->cache_time(rtc);
    if (sound->_source == 0 ||
      (sound->_stream_queued.empty() &&
        (sound->_loops_completed >= sound->_playing_loops))) {
      sound->finished();
    }
  }
}


/**
 * Shuts down the audio manager and releases any resources associated with it.
 * Also cleans up all AudioSounds created via the manager.
 */
void SteamAudioManager::
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
      int i = 0;
      ALuint* sources;
      sources = new ALuint[_al_sources->size()];
      for (SourceCache::iterator si = _al_sources->begin(); si != _al_sources->end(); ++si) {
        sources[i++] = *si;
      }
      make_current();
      alGetError(); // clear errors
      alDeleteSources(_al_sources->size(), sources);
      al_audio_errcheck("alDeleteSources()");
      delete[] sources;
      _al_sources->clear();

      // make sure that the context is not current when it is destroyed
      alcGetError(_device); // clear errors
      alcMakeContextCurrent(nullptr);
      alc_audio_errcheck("alcMakeContextCurrent(NULL)", _device);

      alcDestroyContext(_context);
      alc_audio_errcheck("alcDestroyContext(_context)", _device);
      _context = nullptr;

      if (_device) {
        audio_debug("Going to try to close openAL");
        alcCloseDevice(_device);
        // alc_audio_errcheck("alcCloseDevice(_device)",_device);
        _device = nullptr;
        audio_debug("openAL Closed");
      }

      _openal_active = false;
    }
  }
  _cleanup_required = false;
}

/**
 * Discards sounds from the sound cache until the number of sounds remaining
 * is under the limit.
 */
void SteamAudioManager::
discard_excess_cache(int sample_limit) {
  ReMutexHolder holder(_lock);
  int stream_limit = 5;

  while (((int)_expiring_samples.size()) > sample_limit) {
    SoundData* sd = (SoundData*)(_expiring_samples.front());
    nassertv(sd->_client_count == 0);
    nassertv(sd->_expire == _expiring_samples.begin());
    _expiring_samples.pop_front();
    _sample_cache.erase(_sample_cache.find(sd->_movie->get_filename()));
    audio_debug("Expiring: " << sd->_movie->get_filename().get_basename());
    delete sd;
  }

  while (((int)_expiring_streams.size()) > stream_limit) {
    SoundData* sd = (SoundData*)(_expiring_streams.front());
    nassertv(sd->_client_count == 0);
    nassertv(sd->_expire == _expiring_streams.begin());
    _expiring_streams.pop_front();
    audio_debug("Expiring: " << sd->_movie->get_filename().get_basename());
    delete sd;
  }
}

/**
 * Deletes an OpenAL buffer.  This is a special function because some
 * implementations of OpenAL (e.g. Apple's) don't unlock the buffers
 * immediately, due to needing to coordinate with another thread.  If this is
 * the case, the alDeleteBuffers call will error back with AL_INVALID_OPERATION
 * as if trying to delete an actively-used buffer, which will tell us to wait a
 * bit and try again.
 */
void SteamAudioManager::
delete_buffer(ALuint buffer) {
  ReMutexHolder holder(_lock);
  int tries = 0;
  ALuint error;

  // Keep trying until we succeed (or give up).
  while (true) {
    alDeleteBuffers(1, &buffer);
    error = alGetError();

    if (error == AL_NO_ERROR) {
      // Success!  This will happen right away 99% of the time.
      return;
    }
    else if (error != AL_INVALID_OPERATION) {
      // We weren't expecting that.  This should be reported.
      break;
    }
    else if (tries >= openal_buffer_delete_retries.get_value()) {
      // We ran out of retries.  Give up.
      break;
    }
    else {
      // Make another try after (delay * 2^n) seconds.
      Thread::sleep(openal_buffer_delete_delay.get_value() * (1 << tries));
      tries++;
    }
  }

  // If we got here, one of the breaks above happened, indicating an error.
  audio_error("failed to delete a buffer: " << alGetString(error));
}
