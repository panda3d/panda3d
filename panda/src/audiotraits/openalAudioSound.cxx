// Filename: openalAudioSound.cxx
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

//Panda Headers
#include "throw_event.h"
#include "openalAudioSound.h"
#include "openalAudioManager.h"

TypeHandle OpenALAudioSound::_type_handle;


#ifndef NDEBUG //[
  #define openal_audio_debug(x) \
      audio_debug("OpenALAudioSound \""<<get_name() \
      <<"\" "<< x )
#else //][
#define openal_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////

OpenALAudioSound::
OpenALAudioSound(OpenALAudioManager* manager,
                 MovieAudio *movie,
                 bool positional,
                 int mode) :
  _movie(movie),
  _sd(NULL),
  _playing_loops(0),
  _playing_rate(0.0),
  _loops_completed(0),
  _source(0),
  _manager(manager),
  _volume(1.0f),
  _balance(0),
  _play_rate(1.0),
  _positional(positional),
  _min_dist(3.28f),
  _max_dist(1000000000.0f),
  _drop_off_factor(1.0f),
  _length(0.0),
  _loop_count(1),
  _desired_mode(mode),
  _start_time(0.0),
  _current_time(0.0),
  _basename(movie->get_filename().get_basename()),
  _active(manager->get_active()),
  _paused(false)
{
  _location[0] = 0.0f;
  _location[1] = 0.0f;
  _location[2] = 0.0f;
  _velocity[0] = 0.0f;
  _velocity[1] = 0.0f;
  _velocity[2] = 0.0f;

  ReMutexHolder holder(OpenALAudioManager::_lock);

  require_sound_data();
  if (_manager == NULL) {
    return;
  }

  _length = _sd->_length;
  if (positional) {
    if (_sd->_channels != 1) {
      audio_warning("stereo sound " << movie->get_filename() << " will not be spatialized");
    }
  }
  release_sound_data();
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
OpenALAudioSound::
~OpenALAudioSound() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::cleanup
//       Access: Private
//  Description: Disables the sound forever.  Releases resources and
//               detaches the sound from its audio manager.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
cleanup() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_manager == 0) {
    return;
  }
  if (_source) {
    stop();
  }
  if (_sd) {
    _manager->decrement_client_count(_sd);
    _sd = 0;
  }
  _manager->release_sound(this);
  _manager = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::play
//       Access: public
//  Description: Plays a sound.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
play() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_manager == 0) return;

  PN_stdfloat px,py,pz,vx,vy,vz;
  
  if (!_active) {
    _paused = true;
    return;
  }
  
  stop();

  require_sound_data();
  if (_manager == 0) return;
  _manager->starting_sound(this);

  if (!_source) {
    return;
  }
  
  _manager->make_current();
  
  alGetError(); // clear errors
  
  // nonpositional sources are made relative to the listener so they don't move
  alSourcei(_source,AL_SOURCE_RELATIVE,_positional?AL_FALSE:AL_TRUE);
  al_audio_errcheck("alSourcei(_source,AL_SOURCE_RELATIVE)");
  
  // set source properties that we have stored
  set_volume(_volume);
  //set_balance(_balance);

  set_3d_min_distance(_min_dist);
  set_3d_max_distance(_max_dist);
  set_3d_drop_off_factor(_drop_off_factor);
  get_3d_attributes(&px,&py,&pz,&vx,&vy,&vz);
  set_3d_attributes(px, py, pz, vx, vy, vz);
  
  _playing_loops = _loop_count;
  if (_playing_loops == 0) {
    _playing_loops = 1000000000;
  }
  _loops_completed = 0;

  double play_rate = _play_rate * _manager->get_play_rate();
  audio_debug("playing. Rate=" << play_rate);
  alSourcef(_source, AL_PITCH, play_rate);
  _playing_rate = play_rate;
  
  if (_sd->_sample) {
    push_fresh_buffers();
    alSourcef(_source, AL_SEC_OFFSET, _start_time);
    _stream_queued[0]._time_offset = _start_time;
    restart_stalled_audio();
  } else {
    audio_debug("Play: stream tell = " << _sd->_stream->tell() << " seeking " << _start_time); 
    if (_sd->_stream->tell() != _start_time) {
      _sd->_stream->seek(_start_time);
    }
    push_fresh_buffers();
    restart_stalled_audio();
  }
  double rtc = TrueClock::get_global_ptr()->get_short_time();
  set_calibrated_clock(rtc, _start_time, 1.0);
  _current_time = _start_time;
  _start_time = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::stop
//       Access: public
//  Description: Stop a sound
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
stop() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_manager==0) return;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourceStop(_source);
    al_audio_errcheck("stopping a source");
    alSourcei(_source, AL_BUFFER, 0);
    al_audio_errcheck("clear source buffers");
    for (int i=0; i<((int)(_stream_queued.size())); i++) {
      ALuint buffer = _stream_queued[i]._buffer;
      if (buffer != _sd->_sample) {
        alDeleteBuffers(1, &buffer);
        al_audio_errcheck("deleting a buffer");
      }
    }
    _stream_queued.resize(0);
  }
  
  _manager->stopping_sound(this);
  release_sound_data();
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::finished
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
finished() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  stop();
  _current_time = _length;
  if (!_finished_event.empty()) {
    throw_event(_finished_event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_loop
//       Access: public
//  Description: Turns looping on and off
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_loop(bool loop) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  set_loop_count((loop)?0:1);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_loop
//       Access: public
//  Description: Returns whether looping is on or off
////////////////////////////////////////////////////////////////////
bool OpenALAudioSound::
get_loop() const {
  return (_loop_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_loop_count
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_loop_count(unsigned long loop_count) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_manager==0) return;
  
  if (loop_count >= 1000000000) {
    loop_count = 0;
  }
  _loop_count=loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_loop_count
//       Access: public
//  Description: Return how many times a sound will loop.
////////////////////////////////////////////////////////////////////
unsigned long OpenALAudioSound::
get_loop_count() const {
  return _loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::restart_stalled_audio
//       Access: public
//  Description: When streaming audio, the computer is supposed to 
//               keep OpenAL's queue full.  However, there are times
//               when the computer is running slow and the queue 
//               empties prematurely.  In that case, OpenAL will stop.
//               When the computer finally gets around to refilling
//               the queue, it is necessary to tell OpenAL to resume
//               playing.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
restart_stalled_audio() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  ALenum status;
  if (_stream_queued.size() == 0) {
    return;
  }
  alGetError();
  alGetSourcei(_source, AL_SOURCE_STATE, &status);
  if (status != AL_PLAYING) {
    alSourcePlay(_source);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::queue_buffer
//       Access: public
//  Description: Pushes a buffer into the source queue.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
queue_buffer(ALuint buffer, int samples, int loop_index, double time_offset) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  // Now push the buffer into the stream queue.
  alGetError();
  alSourceQueueBuffers(_source,1,&buffer);
  ALenum err = alGetError();
  if (err != AL_NO_ERROR) {
    audio_error("could not load sample buffer into the queue");
    cleanup();
    return;
  }
  QueuedBuffer buf;
  buf._buffer = buffer;
  buf._samples = samples;
  buf._loop_index = loop_index;
  buf._time_offset = time_offset;
  _stream_queued.push_back(buf);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::make_buffer
//       Access: public
//  Description: Creates an OpenAL buffer object.
////////////////////////////////////////////////////////////////////
ALuint OpenALAudioSound::
make_buffer(int samples, int channels, int rate, unsigned char *data) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  
  // Allocate a buffer to hold the data.
  alGetError();
  ALuint buffer;
  alGenBuffers(1, &buffer);
  if (alGetError() != AL_NO_ERROR) {
    audio_error("could not allocate an OpenAL buffer object");
    cleanup();
    return 0;
  }
  
  // Now fill the buffer with the data provided.
  alBufferData(buffer,
               (channels>1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
               data, samples * channels * 2, rate);
  int err = alGetError();
  if (err != AL_NO_ERROR) {
    audio_error("could not fill OpenAL buffer object with data");
    cleanup();
    return 0;
  }
  
  return buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::read_stream_data
//       Access: public
//  Description: Fills a buffer with data from the stream.
//               Returns the number of samples stored in the buffer.
////////////////////////////////////////////////////////////////////
int OpenALAudioSound::
read_stream_data(int bytelen, unsigned char *buffer) {
  ReMutexHolder holder(OpenALAudioManager::_lock);

  MovieAudioCursor *cursor = _sd->_stream;
  double length = cursor->length();
  int channels = cursor->audio_channels();
  int rate = cursor->audio_rate();
  int space = bytelen / (channels * 2);
  int fill = 0;
  
  while (space && (_loops_completed < _playing_loops)) {
    double t = cursor->tell();
    double remain = length - t;
    if (remain > 60.0) {
      remain = 60.0;
    }
    int samples = (int)(remain * rate);
    if (samples <= 0) {
      _loops_completed += 1;
      cursor->seek(0.0);
      continue;
    }
    if (_sd->_stream->ready() == 0) {
      if (_sd->_stream->aborted()) {
        _loops_completed = _playing_loops;
      }
      return fill;
    }
    if (samples > space) {
      samples = space;
    }
    if (samples > _sd->_stream->ready()) {
      samples = _sd->_stream->ready();
    }
    cursor->read_samples(samples, (PN_int16 *)buffer);
    size_t hval = AddHash::add_hash(0, (PN_uint8*)buffer, samples*channels*2);
    audio_debug("Streaming " << cursor->get_source()->get_name() << " at " << t << " hash " << hval);
    fill += samples;
    space -= samples;
    buffer += (samples * channels * 2);
  }
  return fill; 
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::correct_calibrated_clock
//       Access: public
//  Description: Compares the specified time to the value of the
//               calibrated clock, and adjusts the calibrated
//               clock speed to make it closer to the target value.
//               This routine is quite careful to make sure that
//               the calibrated clock moves in a smooth, monotonic
//               way.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
correct_calibrated_clock(double rtc, double t) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  double cc = (rtc - _calibrated_clock_base) * _calibrated_clock_scale;
  double diff = cc-t;
  _calibrated_clock_decavg = (_calibrated_clock_decavg * 0.95) + (diff * 0.05);
  if (diff > 0.5) {
    set_calibrated_clock(rtc, t, 1.0);
    _calibrated_clock_decavg = 0.0;
  } else {
    double scale = 1.0;
    if ((_calibrated_clock_decavg > 0.01) && (diff > 0.01)) {
      scale = 0.98;
    }
    if ((_calibrated_clock_decavg < -0.01) && (diff < -0.01)) {
      scale = 1.03;
    }
    if ((_calibrated_clock_decavg < -0.05) && (diff < -0.05)) {
      scale = 1.2;
    }
    if ((_calibrated_clock_decavg < -0.15) && (diff < -0.15)) {
      scale = 1.5;
    }
    set_calibrated_clock(rtc, cc, scale);
  }
  cc = (rtc - _calibrated_clock_base) * _calibrated_clock_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::pull_used_buffers
//       Access: public
//  Description: Pulls any used buffers out of OpenAL's queue.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
pull_used_buffers() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  while (_stream_queued.size()) {
    ALuint buffer = 0;
    alGetError();
    alSourceUnqueueBuffers(_source, 1, &buffer);
    int err = alGetError();
    if (err == AL_NO_ERROR) {
      if (_stream_queued[0]._buffer != buffer) {
        audio_error("corruption in stream queue");
        cleanup();
        return;
      }
      _stream_queued.pop_front();
      if (_stream_queued.size()) {
        double al = _stream_queued[0]._time_offset + _stream_queued[0]._loop_index * _length;
        double rtc = TrueClock::get_global_ptr()->get_short_time();
        correct_calibrated_clock(rtc, al);
      }
      if (buffer != _sd->_sample) {
        alDeleteBuffers(1,&buffer);
      }
    } else {
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::push_fresh_buffers
//       Access: public
//  Description: Pushes fresh buffers into OpenAL's queue until
//               the queue is "full" (ie, has plenty of data).
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
push_fresh_buffers() {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  static unsigned char data[65536];
  
  if (_sd->_sample) {
    while ((_loops_completed < _playing_loops) &&
           (_stream_queued.size() < 100)) {
      queue_buffer(_sd->_sample, 0,_loops_completed, 0.0);
      _loops_completed += 1;
    }
  } else {
    MovieAudioCursor *cursor = _sd->_stream;
    int channels = cursor->audio_channels();
    int rate = cursor->audio_rate();
    
    int fill = 0;
    for (size_t i = 0; i < _stream_queued.size(); i++) {
      fill += _stream_queued[i]._samples;
    }
    
    while ((_loops_completed < _playing_loops) &&
           (fill < (int)(audio_buffering_seconds * rate * channels))) {
      int loop_index = _loops_completed;
      double time_offset = cursor->tell();
      int samples = read_stream_data(65536, data);
      if (samples == 0) {
        break;
      }
      ALuint buffer = make_buffer(samples, channels, rate, data);
      if (_manager == 0) return;
      queue_buffer(buffer, samples, loop_index, time_offset);
      if (_manager == 0) return;
      fill += samples;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_time
//       Access: public
//  Description: The next time you call play, the sound will
//               start from the specified offset.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_time(PN_stdfloat time) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _start_time = time;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_time
//       Access: public
//  Description: Gets the play position within the sound
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_time() const {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_manager == 0) {
    return 0.0;
  }
  return _current_time;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::cache_time
//       Access: Private
//  Description: Updates the current_time field of a playing sound.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
cache_time(double rtc) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  assert(_source != 0);
  double t=get_calibrated_clock(rtc);
  double max = _length * _playing_loops;
  if (t >= max) {
    _current_time = _length;
  } else {
    _current_time = fmod(t, _length);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_volume(PN_stdfloat vol)
//       Access: public
//  Description: 0.0 to 1.0 scale of volume converted to Fmod's
//               internal 0.0 to 255.0 scale.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _volume=volume;

  if (_source) {
    volume*=_manager->get_volume();
    _manager->make_current();
    alGetError(); // clear errors
    alSourcef(_source,AL_GAIN,volume);
    al_audio_errcheck("alSourcef(_source,AL_GAIN)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_volume
//       Access: public
//  Description: Gets the current volume of a sound.  1 is Max. O is Min.
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_volume() const {
  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_balance(PN_stdfloat bal)
//       Access: public
//  Description: -1.0 to 1.0 scale
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_balance(PN_stdfloat balance_right) {
  audio_debug("OpenALAudioSound::set_balance() not implemented");
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_balance
//       Access: public
//  Description: -1.0 to 1.0 scale 
//        -1 should be all the way left.
//        1 is all the way to the right.
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_balance() const {
  audio_debug("OpenALAudioSound::get_balance() not implemented");
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_play_rate(PN_stdfloat rate)
//       Access: public
//  Description: Sets the speed at which a sound plays back.
//        The rate is a multiple of the sound, normal playback speed.
//        IE 2 would play back 2 times fast, 3 would play 3 times, and so on.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_play_rate(PN_stdfloat play_rate) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _play_rate = play_rate;
  if (_source) {
    alSourcef(_source, AL_PITCH, play_rate);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_play_rate
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_play_rate() const {
  return _play_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::length
//       Access: public
//  Description: Get length
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound
//
//               Both Panda3D and OpenAL use a right handed
//               coordinate system.  However, in Panda3D the
//               Y-Axis is going into the Screen and the
//               Z-Axis is going up.  In OpenAL the Y-Axis is
//               going up and the Z-Axis is coming out of
//               the screen.
//
//               The solution is simple, we just flip the Y
//               and Z axis and negate the Z, as we move
//               coordinates from Panda to OpenAL and back.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _location[0] = px;
  _location[1] = pz;
  _location[2] = -py; 

  _velocity[0] = vx;
  _velocity[1] = vz;
  _velocity[2] = -vy;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcefv(_source,AL_POSITION,_location);
    al_audio_errcheck("alSourcefv(_source,AL_POSITION)");
    alSourcefv(_source,AL_VELOCITY,_velocity);
    al_audio_errcheck("alSourcefv(_source,AL_VELOCITY)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound
//         Currently unimplemented. Get the attributes of the attached object.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  *px = _location[0];
  *py = -_location[2];
  *pz = _location[1];

  *vx = _velocity[0];
  *vy = -_velocity[2];
  *vz = _velocity[1];
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_min_distance
//       Access: public
//  Description: Set the distance that this sound begins to fall off. Also
//               affects the rate it falls off.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _min_dist = dist;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_REFERENCE_DISTANCE,_min_dist*_manager->audio_3d_get_distance_factor());
    al_audio_errcheck("alSourcefv(_source,AL_REFERENCE_DISTANCE)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_min_distance
//       Access: public
//  Description: Get the distance that this sound begins to fall off
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_3d_min_distance() const {
  return _min_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_max_distance
//       Access: public
//  Description: Set the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_max_distance(PN_stdfloat dist) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _max_dist = dist;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_MAX_DISTANCE,_max_dist*_manager->audio_3d_get_distance_factor());
    al_audio_errcheck("alSourcefv(_source,AL_MAX_DISTANCE)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_max_distance
//       Access: public
//  Description: Get the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_3d_drop_off_factor
//       Access: public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_3d_drop_off_factor(PN_stdfloat factor) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  _drop_off_factor = factor;

  if (_source) {
    _manager->make_current();

    alGetError(); // clear errors
    alSourcef(_source,AL_ROLLOFF_FACTOR,_drop_off_factor*_manager->audio_3d_get_drop_off_factor());
    al_audio_errcheck("alSourcefv(_source,AL_ROLLOFF_FACTOR)");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_3d_drop_off_factor
//       Access: public
//  Description: Control the effect distance has on audability.
//               Defaults to 1.0
////////////////////////////////////////////////////////////////////
PN_stdfloat OpenALAudioSound::
get_3d_drop_off_factor() const {
  return _drop_off_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_active
//       Access: public
//  Description: Sets whether the sound is marked "active".  By
//               default, the active flag true for all sounds.  If the
//               active flag is set to false for any particular sound,
//               the sound will not be heard.
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_active(bool active) {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_active!=active) {
    _active=active;
    if (_active) {
      // ...activate the sound.
      if (_paused && _loop_count==0) {
        // ...this sound was looping when it was paused.
        _paused=false;
        play();
      }
    } else {
      // ...deactivate the sound.
      if (status()==PLAYING) {
        if (_loop_count==0) {
          // ...we're pausing a looping sound.
          _paused=true;
        }
        stop();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_active 
//       Access: public
//  Description: Returns whether the sound has been marked "active".
////////////////////////////////////////////////////////////////////
bool OpenALAudioSound::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::set_finished_event
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
void OpenALAudioSound::
set_finished_event(const string& event) {
  _finished_event = event;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_finished_event
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
const string& OpenALAudioSound::
get_finished_event() const {
  return _finished_event;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::get_name
//       Access: public
//  Description: Get name of sound file
////////////////////////////////////////////////////////////////////
const string& OpenALAudioSound::
get_name() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenALAudioSound::status
//       Access: public
//  Description: Get status of the sound.
//
//               This returns the status as of the
//               last push_fresh_buffers
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus OpenALAudioSound::
status() const {
  ReMutexHolder holder(OpenALAudioManager::_lock);
  if (_source==0) {
    return AudioSound::READY;
  }
  if ((_loops_completed >= _playing_loops)&&(_stream_queued.size()==0)) {
    return AudioSound::READY;
  } else {
    return AudioSound::PLAYING;
  }
}
