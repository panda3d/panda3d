/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioSound.cxx
 * @author cort
 * @date 2003-01-22
 * @author ben
 * @date 2003-10-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 */

#include "pandabase.h"
#include "dcast.h"

// Panda Headers
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "fmodAudioSound.h"
#include "string_utils.h"
#include "subfileInfo.h"
#include "reMutexHolder.h"
#include "virtualFileSystem.h"
#include "vector_uchar.h"

TypeHandle FMODAudioSound::_type_handle;

/**
 * Constructor All sound will DEFAULT load as a 2D sound unless otherwise
 * specified.
 */
FMODAudioSound::
FMODAudioSound(AudioManager *manager, VirtualFile *file, bool positional) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::FMODAudioSound() Creating new sound, filename: "
              << file->get_original_filename());

  _active = manager->get_active();
  _paused = false;
  _start_time = 0.0;
  _balance = 0.0;
  _volume = 1.0;
  _playrate = 1.0;
  _is_midi = false;
  _length = 0;

  // 3D attributes of the sound.
  _location.x = 0;
  _location.y = 0;
  _location.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  _min_dist = 1.0;
  _max_dist = 1000000000.0;

  // These set the speaker levels to a default if you are using a multichannel
  // setup.
  for (int i = 0; i < AudioManager::SPK_COUNT; i++) {
    _mix[i] = 1.0;
  }

  FMOD_RESULT result;

  // Assign the values we need
  FMODAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  _channel = nullptr;
  _file_name = file->get_original_filename();
  _file_name.set_binary();

  // Get the Speaker Mode [Important for later on.]
  result = _manager->get_speaker_mode(_speakermode);
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  {
    bool preload = (fmod_audio_preload_threshold < 0) || (file->get_file_size() < fmod_audio_preload_threshold);
    int flags = FMOD_DEFAULT;
    flags |= positional ? FMOD_3D : FMOD_2D;

    FMOD_CREATESOUNDEXINFO sound_info;
    memset(&sound_info, 0, sizeof(sound_info));
    sound_info.cbsize = sizeof(sound_info);

    std::string ext = downcase(_file_name.get_extension());
    if (ext == "mid") {
      // Get the MIDI parameters.
      memcpy(&sound_info, &_manager->_midi_info, sizeof(sound_info));
      if (sound_info.dlsname != nullptr) {
        audio_debug("Using DLS file " << sound_info.dlsname);
      }
      _is_midi = true;
      // Need this flag so we can correctly query the length of MIDIs.
      flags |= FMOD_ACCURATETIME;
    }

    const char *name_or_data = _file_name.c_str();
    std::string os_filename;

    vector_uchar mem_buffer;
    SubfileInfo info;
    if (preload) {
      // Pre-read the file right now, and pass it in as a memory buffer.  This
      // avoids threading issues completely, because all of the reading
      // happens right here.
      file->read_file(mem_buffer, true);
      sound_info.length = mem_buffer.size();
      if (mem_buffer.size() != 0) {
        name_or_data = (const char *)&mem_buffer[0];
      }
      flags |= FMOD_OPENMEMORY;
      if (fmodAudio_cat.is_debug()) {
        fmodAudio_cat.debug()
          << "Reading " << _file_name << " into memory (" << sound_info.length
          << " bytes)\n";
      }
      result =
        _manager->_system->createSound(name_or_data, flags, &sound_info, &_sound);
    }
    else {
      result = FMOD_ERR_FILE_BAD;

      if (file->get_system_info(info)) {
        // The file exists on disk (or it's part of a multifile that exists on
        // disk), so we can have FMod read the file directly.  This is also
        // safe, because FMod uses its own IO operations that don't involve
        // Panda, so this can safely happen in an FMod thread.
        os_filename = info.get_filename().to_os_specific();
        name_or_data = os_filename.c_str();
        sound_info.fileoffset = (unsigned int)info.get_start();
        sound_info.length = (unsigned int)info.get_size();
        flags |= FMOD_CREATESTREAM;
        if (fmodAudio_cat.is_debug()) {
          fmodAudio_cat.debug()
            << "Streaming " << _file_name << " from disk (" << name_or_data
            << ", " << sound_info.fileoffset << ", " << sound_info.length << ")\n";
        }

        result =
          _manager->_system->createSound(name_or_data, flags, &sound_info, &_sound);
      }

      // If FMOD can't directly read the file (eg. if Panda is locking it for
      // write, or it's compressed) we have to use the callback interface.
      if (result == FMOD_ERR_FILE_BAD) {
  #if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
        // Otherwise, if the Panda threading system is compiled in, we can
        // assign callbacks to read the file through the VFS.
        name_or_data = (const char *)file;
        sound_info.fileoffset = 0;
        sound_info.length = (unsigned int)info.get_size();
        sound_info.fileuseropen = open_callback;
        sound_info.fileuserdata = file;
        sound_info.fileuserclose = close_callback;
        sound_info.fileuserread = read_callback;
        sound_info.fileuserseek = seek_callback;
        flags |= FMOD_CREATESTREAM;
        if (fmodAudio_cat.is_debug()) {
          fmodAudio_cat.debug()
            << "Streaming " << _file_name << " from disk using callbacks\n";
        }
        result =
          _manager->_system->createSound(name_or_data, flags, &sound_info, &_sound);

  #else  // HAVE_THREADS && !SIMPLE_THREADS
        // Without threads, we can't safely read this file.
        name_or_data = "";

        fmodAudio_cat.warning()
          << "Cannot stream " << _file_name << "; file is not literally on disk.\n";
  #endif
      }
    }
  }

  if (result != FMOD_OK) {
    audio_error("createSound(" << _file_name << "): " << FMOD_ErrorString(result));

    // We couldn't load the sound file.  Create a blank sound record instead.
    FMOD_CREATESOUNDEXINFO sound_info;
    memset(&sound_info, 0, sizeof(sound_info));
    char blank_data[100];
    memset(blank_data, 0, sizeof(blank_data));
    sound_info.cbsize = sizeof(sound_info);
    sound_info.length = sizeof(blank_data);
    sound_info.numchannels = 1;
    sound_info.defaultfrequency = 8000;
    sound_info.format = FMOD_SOUND_FORMAT_PCM16;
    int flags = FMOD_OPENMEMORY | FMOD_OPENRAW;

    result = _manager->_system->createSound(blank_data, flags, &sound_info, &_sound);
    fmod_audio_errcheck("createSound (blank)", result);
  }

  // Some WAV files contain a loop bit.  This is not handled consistently.
  // Override it.
  _sound->setLoopCount(1);
  _sound->setMode(FMOD_LOOP_OFF);

  // This is just to collect the defaults of the sound, so we don't Have to
  // query FMOD everytime for the info.  It is also important we get the
  // '_sample_frequency' variable here, for the 'set_play_rate()' and
  // 'get_play_rate()' methods later;

  result = _sound->getDefaults(&_sample_frequency, &_priority);
  fmod_audio_errcheck("_sound->getDefaults()", result);
  result = _channel->getVolume(&_volume);

  // Store off the original length of the sound without any play rate changes
  // applied.  We need this to figure out the loop points of MIDIs that have
  // been sped up.
  result = _sound->getLength(&_length, FMOD_TIMEUNIT_MS);
  fmod_audio_errcheck("_sound->getLength()", result);
}


/**
 * DESTRUCTOR!!!
 */
FMODAudioSound::
~FMODAudioSound() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  // Release the sound.  There is no need to release the channel, it will be
  // reused for future sounds.
  result = _sound->release();
  fmod_audio_errcheck("_sound->release()", result);

  audio_debug("Released FMODAudioSound\n");
}

/**
 * Plays a sound.
 */
void FMODAudioSound::
play() {
  start_playing();
}

/**
 * Stop a sound
 */
void FMODAudioSound::
stop() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (_channel) {
    result =_channel->stop();
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->stop()", result);
    }
  }

  _manager->stopping_sound(this);
}

/**
 * Turns looping on or off.
 */
void FMODAudioSound::
set_loop(bool loop) {
  set_loop_count(loop ? 0 : 1);
}

/**
 * Returns whether looping is on or off.
 */
bool FMODAudioSound::
get_loop() const {
  return get_loop_count() != 1;
}

/**
 * Panda uses 0 to mean loop forever.  Fmod uses negative numbers to mean loop
 * forever.  (0 means don't loop, 1 means play twice, etc.  We must convert!
 */
void FMODAudioSound::
set_loop_count(unsigned long loop_count) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::set_loop_count()   Setting the sound's loop count to: " << loop_count);

  // LOCALS
  FMOD_RESULT result;

  if (loop_count == 0) {
    result = _sound->setLoopCount(-1);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);
    fmod_audio_errcheck("_sound->setMode()", result);
  } else if (loop_count == 1) {
    result = _sound->setLoopCount(1);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_OFF);
    fmod_audio_errcheck("_sound->setMode()", result);
  } else {
    result = _sound->setLoopCount(loop_count);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);
    fmod_audio_errcheck("_sound->setMode()", result);
  }

  audio_debug("FMODAudioSound::set_loop_count()   Sound's loop count should be set to: " << loop_count);
}

/**
 * Return how many times a sound will loop.
 */
unsigned long FMODAudioSound::
get_loop_count() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  int loop_count;

  result = _sound->getLoopCount(&loop_count);
  fmod_audio_errcheck("_sound->getLoopCount()", result);

  if (loop_count <= 0) {
    return 0;
  } else {
    return (unsigned long)loop_count;
  }
}

/**
 * Sets the time at which the next play() operation will begin.  If we are
 * already playing, skips to that time immediatey.
 */
void FMODAudioSound::
set_time(PN_stdfloat start_time) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _start_time = start_time;

  if (status() == PLAYING) {
    // Already playing; skip to the indicated time.
    start_playing();
  }
}

/**
 * Gets the play position within the sound
 */
PN_stdfloat FMODAudioSound::
get_time() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  unsigned int current_time_ms;

  if (!_channel) {
    return 0.0f;
  }

  result = _channel->getPosition(&current_time_ms, FMOD_TIMEUNIT_MS);
  if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
    return 0.0f;
  }
  fmod_audio_errcheck("_channel->getPosition()", result);

  return current_time_ms * 0.001;
}

/**
 * 0.0 to 1.0 scale of volume converted to Fmod's internal 0.0 to 255.0 scale.
 */
void FMODAudioSound::
set_volume(PN_stdfloat vol) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _volume = vol;
  set_volume_on_channel();
}

/**
 * Gets the current volume of a sound.  1 is Max.  O is Min.
 */
PN_stdfloat FMODAudioSound::
get_volume() const {
  return _volume;
}

/**
 * Starts the sound playing at _start_time.
 */
void FMODAudioSound::
start_playing() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (!_active) {
    _paused = true;
    return;
  }

  _manager->starting_sound(this);

  int start_time_ms = (int)(_start_time * 1000);

  if (_channel) {
    // try backing up current sound.
    result = _channel->setPosition(start_time_ms, FMOD_TIMEUNIT_MS);
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;

    } else {
      fmod_audio_errcheck("_channel->setPosition()", result);

      bool playing;
      result = _channel->isPlaying(&playing);
      fmod_audio_errcheck("_channel->isPlaying()", result);
      if (result != FMOD_OK || !playing) {
        _channel = nullptr;
      }
    }
  }

  if (!_channel) {
    result = _manager->_system->playSound(_sound, _manager->_channelgroup, true, &_channel);
    fmod_audio_errcheck("_system->playSound()", result);
    result = _channel->setPosition(start_time_ms , FMOD_TIMEUNIT_MS);
    fmod_audio_errcheck("_channel->setPosition()", result);

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();
    set_3d_attributes_on_channel();

    result = _channel->setPaused(false);
    fmod_audio_errcheck("_channel->setPaused()", result);
  }

  _start_time = 0.0;
}

/**
 * Set the volume on a prepared Sound channel.
 */
void FMODAudioSound::
set_volume_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (_channel) {
    result = _channel->setVolume(_volume);
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setVolume()", result);
    }
  }
}

/**
 * -1.0 to 1.0 scale
 */
void FMODAudioSound::
set_balance(PN_stdfloat bal) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _balance = bal;
  set_speaker_mix_or_balance_on_channel();
}

/**
 * -1.0 to 1.0 scale -1 should be all the way left.  1 is all the way to the
 * right.
 */
PN_stdfloat FMODAudioSound::
get_balance() const {
  return _balance;
}

/**
 * Sets the speed at which a sound plays back.  The rate is a multiple of the
 * sound, normal playback speed.  IE 2 would play back 2 times fast, 3 would
 * play 3 times, and so on.  This can also be set to a negative number so a
 * sound plays backwards.  But rememeber if the sound is not playing, you must
 * set the sound's time to its end to hear a song play backwards.
 */
void FMODAudioSound::
set_play_rate(PN_stdfloat rate) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _playrate = rate;
  set_play_rate_on_channel();
}

/**
 *
 */
PN_stdfloat FMODAudioSound::
get_play_rate() const {
  return _playrate;
}

/**
 * Set the play rate on a prepared Sound channel.
 */
void FMODAudioSound::
set_play_rate_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (_is_midi) {
    // If this is a MIDI sequence, simply adjust the speed at which the song is
    // played.  This makes the song play faster without increasing the pitch.
    result = _sound->setMusicSpeed(_playrate);
    fmod_audio_errcheck("_sound->setMusicSpeed()", result);

    // We have to manually fix up the loop points when changing the speed of a
    // MIDI because FMOD does not handle this for us.
    result = _sound->setLoopPoints(0, FMOD_TIMEUNIT_MS, _length / _playrate, FMOD_TIMEUNIT_MS);
    fmod_audio_errcheck("_sound->setLoopPoints()", result);

  } else if (_channel) {
    // We have to adjust the frequency for non-sequence sounds.  The sound will
    // play faster, but will also have an increase in pitch.

    PN_stdfloat frequency = _sample_frequency * _playrate;
    result = _channel->setFrequency(frequency);
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setFrequency()", result);
    }
  }
}

/**
 * Get name of sound file
 */
const std::string& FMODAudioSound::
get_name() const {
  return _file_name;
}

/**
 * Get length FMOD returns the time in MS  so we have to convert to seconds.
 */
PN_stdfloat FMODAudioSound::
length() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  unsigned int length;

  result = _sound->getLength(&length, FMOD_TIMEUNIT_MS);
  fmod_audio_errcheck("_sound->getLength()", result);

  return ((double)length) / 1000.0;
}

/**
 * Set position and velocity of this sound NOW LISTEN UP!!! THIS IS IMPORTANT!
 * Both Panda3D and FMOD use a left handed coordinate system.  But there is a
 * major difference!  In Panda3D the Y-Axis is going into the Screen and the
 * Z-Axis is going up.  In FMOD the Y-Axis is going up and the Z-Axis is going
 * into the screen.  The solution is simple, we just flip the Y and Z axis, as
 * we move coordinates from Panda to FMOD and back.  What does did mean to
 * average Panda user?  Nothing, they shouldn't notice anyway.  But if you
 * decide to do any 3D audio work in here you have to keep it in mind.  I told
 * you, so you can't say I didn't.
 */
void FMODAudioSound::
set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _location.x = px;
  _location.y = pz;
  _location.z = py;

  _velocity.x = vx;
  _velocity.y = vz;
  _velocity.z = vy;

  set_3d_attributes_on_channel();
}

/**
 *
 */
void FMODAudioSound::
set_3d_attributes_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);

  if ((_channel) && (soundMode & FMOD_3D)) {
    result = _channel->set3DAttributes(&_location, &_velocity);
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->set3DAttributes()", result);
    }
  }
}

/**
 * Get position and velocity of this sound Currently unimplemented.  Get the
 * attributes of the attached object.
 */
void FMODAudioSound::
get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz) {
  audio_error("get3dAttributes: Currently unimplemented. Get the attributes of the attached object.");
}

/**
 * Set the distance that this sound begins to fall off.  Also affects the rate
 * it falls off.
 */
void FMODAudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  _min_dist = dist;

  result = _sound->set3DMinMaxDistance(dist, _max_dist);
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

/**
 * Get the distance that this sound begins to fall off
 */
PN_stdfloat FMODAudioSound::
get_3d_min_distance() const {
  return _min_dist;
}

/**
 * Set the distance that this sound stops falling off
 */
void FMODAudioSound::
set_3d_max_distance(PN_stdfloat dist) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  _max_dist = dist;

  result = _sound->set3DMinMaxDistance(_min_dist, dist);
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

/**
 * Get the distance that this sound stops falling off
 */
PN_stdfloat FMODAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

/**
 * In Multichannel Speaker systems [like Surround].
 *
 * Speakers which don't exist in some systems will simply be ignored.  But I
 * haven't been able to test this yet, so I am jsut letting you know.
 *
 * BTW This will also work in Stereo speaker systems, but since PANDA/FMOD has
 * a balance [pan] function what is the point?
 */
PN_stdfloat FMODAudioSound::
get_speaker_mix(int speaker) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  if (!_channel || speaker < 0 || speaker >= AudioManager::SPK_COUNT) {
    return 0.0;
  }

  int in, out;
  float mix[32][32];
  FMOD_RESULT result;
  // First query the number of output speakers and input channels
  result = _channel->getMixMatrix(nullptr, &out, &in, 32);
  fmod_audio_errcheck("_channel->getMixMatrix()", result);
  // Now get the actual mix matrix
  result = _channel->getMixMatrix((float *)mix, &out, &in, 32);
  fmod_audio_errcheck("_channel->getMixMatrix()", result);

  return mix[speaker][0];
}

/**
 * Sets the mix value of a speaker.
 */
void FMODAudioSound::
set_speaker_mix(int speaker, PN_stdfloat mix) {
  nassertv(speaker >= 0 && speaker < AudioManager::SPK_COUNT);

  ReMutexHolder holder(FMODAudioManager::_lock);

  _mix[speaker] = mix;

  set_speaker_mix_or_balance_on_channel();
}

/**
 * Sets the mix values for all speakers.
 */
void FMODAudioSound::
set_speaker_mix(PN_stdfloat frontleft, PN_stdfloat frontright,
                PN_stdfloat center, PN_stdfloat sub,
                PN_stdfloat backleft, PN_stdfloat backright,
                PN_stdfloat sideleft, PN_stdfloat sideright) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  _mix[AudioManager::SPK_front_left] = frontleft;
  _mix[AudioManager::SPK_front_right] = frontright;
  _mix[AudioManager::SPK_front_center] = center;
  _mix[AudioManager::SPK_sub] = sub;
  _mix[AudioManager::SPK_surround_left] = sideleft;
  _mix[AudioManager::SPK_surround_right] = sideright;
  _mix[AudioManager::SPK_back_left] = backleft;
  _mix[AudioManager::SPK_back_right] = backright;

  set_speaker_mix_or_balance_on_channel();
}

/**
 * This is simply a safety catch.  If you are using a Stero speaker setup
 * Panda will only pay attention to 'set_balance()' command when setting
 * speaker balances.  Other wise it will use 'set_speaker_mix'. I put this in,
 * because other wise you end up with a sitation, where 'set_speaker_mix()' or
 * 'set_balace()' will override any previous speaker balance setups.  It all
 * depends on which was called last.
 */
void FMODAudioSound::
set_speaker_mix_or_balance_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);

  if (_channel && (( soundMode & FMOD_3D ) == 0)) {
    if (_speakermode == FMOD_SPEAKERMODE_STEREO) {
      result = _channel->setPan(_balance);
    } else {
      result = _channel->setMixLevelsOutput(
        _mix[AudioManager::SPK_front_left],
        _mix[AudioManager::SPK_front_right],
        _mix[AudioManager::SPK_front_center],
        _mix[AudioManager::SPK_sub],
        _mix[AudioManager::SPK_surround_left],
        _mix[AudioManager::SPK_surround_right],
        _mix[AudioManager::SPK_back_left],
        _mix[AudioManager::SPK_back_right]);
    }
    if (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setSpeakerMix()/setPan()", result);
    }
  }
}

/**
 * Sets the priority of a sound.  This is what FMOD uses to determine is a
 * sound will play if all the other real channels have been used up.
 */
int FMODAudioSound::
get_priority() {
  audio_debug("FMODAudioSound::get_priority()");
  return _priority;
}

/**
 * Sets the Sound Priority [Whether is will be played over other sound when
 * real audio channels become short.
 */
void FMODAudioSound::
set_priority(int priority) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  audio_debug("FMODAudioSound::set_priority()");

  FMOD_RESULT result;

  _priority = priority;

  result = _sound->setDefaults(_sample_frequency, _priority);
  fmod_audio_errcheck("_sound->setDefaults()", result);
}

/**
 * Get status of the sound.
 */
AudioSound::SoundStatus FMODAudioSound::
status() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  bool playingState;

  if (!_channel) {
    return READY;
  }

  result = _channel->isPlaying(&playingState);
  if ((result == FMOD_OK) && (playingState == true)) {
    return PLAYING;
  } else {
    return READY;
  }
}

/**
 * Sets whether the sound is marked "active".  By default, the active flag
 * true for all sounds.  If the active flag is set to false for any particular
 * sound, the sound will not be heard.
 */
void FMODAudioSound::
set_active(bool active) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  if (_active != active) {
    _active = active;
    if (_active) {
      // ...activate the sound.
      if (_paused && get_loop_count()==0) {
        // ...this sound was looping when it was paused.
        _paused = false;
        play();
      }

    } else {
      // ...deactivate the sound.
      if (status() == PLAYING) {
        if (get_loop_count() == 0) {
          // ...we're pausing a looping sound.
          _paused = true;
        }
        // Store off the current time so we can resume from where we paused.
        _start_time = get_time();
        stop();
      }
    }
  }
}

/**
 * Returns whether the sound has been marked "active".
 */
bool FMODAudioSound::
get_active() const {
  return _active;
}

/**
 * Not implemented.
 */
void FMODAudioSound::
finished() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  stop();
}

/**
 * NOT USED ANYMORE!!! Assign a string for the finished event to be referenced
 * by in python by an accept method
 *
 */
void FMODAudioSound::
set_finished_event(const std::string& event) {
  audio_error("set_finished_event: not implemented under FMOD");
}

/**
 * NOT USED ANYMORE!!! Return the string the finished event is referenced by
 */
const std::string& FMODAudioSound::
get_finished_event() const {
  audio_error("get_finished_event: not implemented under FMOD");
  return _finished_event;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODAudioSound::
open_callback(const char *name, unsigned int *file_size,
              void **handle, void *user_data) {
  // We actually pass in the VirtualFile pointer as the "name".
  VirtualFile *file = (VirtualFile *)name;
  if (file == nullptr) {
    return FMOD_ERR_FILE_NOTFOUND;
  }
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "open_callback(" << *file << ")\n";
  }

  std::istream *str = file->open_read_file(true);

  (*file_size) = file->get_file_size(str);
  (*handle) = (void *)str;

  // Explicitly ref the VirtualFile since we're storing it in a void pointer
  // instead of a PT(VirtualFile).
  file->ref();

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODAudioSound::
close_callback(void *handle, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "close_callback(" << *file << ")\n";
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  std::istream *str = (std::istream *)handle;
  vfs->close_read_file(str);

  // Explicitly unref the VirtualFile pointer.
  unref_delete(file);

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODAudioSound::
read_callback(void *handle, void *buffer, unsigned int size_bytes,
              unsigned int *bytes_read, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "read_callback(" << *file << ", " << size_bytes << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->read((char *)buffer, size_bytes);
  (*bytes_read) = str->gcount();

  // We can't yield here, since this callback is made within a sub-thread--an
  // OS-level sub-thread spawned by FMod, not a Panda thread.  But we will
  // only execute this code in the true-threads case anyway.
  // thread_consider_yield();

  if (str->eof()) {
    if ((*bytes_read) == 0) {
      return FMOD_ERR_FILE_EOF;
    } else {
      // Report the EOF next time.
      return FMOD_OK;
    }
  } if (str->fail()) {
    return FMOD_ERR_FILE_BAD;
  } else {
    return FMOD_OK;
  }
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODAudioSound::
seek_callback(void *handle, unsigned int pos, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "seek_callback(" << *file << ", " << pos << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->clear();
  str->seekg(pos);

  if (str->fail() && !str->eof()) {
    return FMOD_ERR_FILE_COULDNOTSEEK;
  } else {
    return FMOD_OK;
  }
}
