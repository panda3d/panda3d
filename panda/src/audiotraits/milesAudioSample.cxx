// Filename: milesAudioSample.cxx
// Created by:  skyler (June 6, 2001)
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

#include "milesAudioSample.h"

#ifdef HAVE_RAD_MSS //[

#include "milesAudioManager.h"


TypeHandle MilesAudioSample::_type_handle;

#undef miles_audio_debug

#ifndef NDEBUG //[
#define miles_audio_debug(x) \
    audio_debug("MilesAudioSample \""<<get_name()<<"\" "<< x )
#else //][
#define miles_audio_debug(x) ((void)0)
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::Constructor
//       Access: Private
//  Description: This constructor is called only by the
//               MilesAudioManager.
////////////////////////////////////////////////////////////////////
MilesAudioSample::
MilesAudioSample(MilesAudioManager *manager, MilesAudioManager::SoundData *sd, 
                 const string &file_name) :
  MilesAudioSound(manager, file_name),
  _sd(sd)
{
  nassertv(sd != NULL);
  audio_debug("MilesAudioSample(manager=0x"<<(void*)&manager
              <<", sd=0x"<<(void*)sd<<", file_name="<<file_name<<")");

  _sample = 0;
  _sample_index = 0;
  _original_playback_rate = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MilesAudioSample::
~MilesAudioSample() {
  miles_audio_debug("~MilesAudioSample()");
  cleanup();
  miles_audio_debug("~MilesAudioSample() done");
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::play
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
play() {
  miles_audio_debug("play()");
  if (_active) {
    if (_sd->_raw_data.empty()) {
      milesAudio_cat.warning()
        << "Could not play " << _file_name << ": no data\n";
    } else {
      stop();
      _manager->starting_sound(this);

      nassertv(_sample == 0);

      GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
      if (!mgr->get_sample(_sample, _sample_index, this)){ 
        milesAudio_cat.warning()
          << "Could not play " << _file_name << ": too many open samples\n";
        _sample = 0;
      } else {
        AIL_set_named_sample_file(_sample, _sd->_basename.c_str(), 
                                  &_sd->_raw_data[0], _sd->_raw_data.size(),
                                  0);
        _original_playback_rate = AIL_sample_playback_rate(_sample);
        AIL_set_sample_user_data(_sample, 0, (SINTa)this);
        AIL_register_EOS_callback(_sample, finish_callback);

        set_volume(_volume);
        set_play_rate(_play_rate);
        AIL_set_sample_loop_count(_sample, _loop_count);

        if (_got_start_time) {
          do_set_time(_start_time);
          AIL_resume_sample(_sample);
        } else {
          AIL_start_sample(_sample);
        }
      }
      
      _got_start_time = false;
    }
  } else {
    // In case _loop_count gets set to forever (zero):
    audio_debug("  paused "<<_file_name );
    _paused = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::stop
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
stop() {
  if (_manager == (MilesAudioManager *)NULL) {
    return;
  }

  miles_audio_debug("stop()");
  _manager->stopping_sound(this);
  // The _paused flag should not be cleared here.  _paused is not like
  // the Pause button on a cd/dvd player.  It is used as a flag to say
  // that it was looping when it was set inactive.  There is no need to
  // make this symmetrical with play().  set_active() is the 'owner' of
  // _paused.  play() accesses _paused to help in the situation where
  // someone calls play on an inactive sound().

  // it fixes audio bug, I don't understand the reasoning of the above comment
  _paused = false; 

  if (_sample != 0) {
    AIL_end_sample(_sample);

    GlobalMilesManager *mgr = GlobalMilesManager::get_global_ptr();
    mgr->release_sample(_sample_index, this);

    _sample = 0;
    _sample_index = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_time
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSample::
get_time() const {
  if (_sample == 0) {
    if (_got_start_time) {
      return _start_time;
    }
    return 0.0f;
  }

  S32 current_ms;
  AIL_sample_ms_position(_sample, NULL, &current_ms);
  PN_stdfloat time = PN_stdfloat(current_ms * 0.001f);

  return time;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_volume
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_volume(PN_stdfloat volume) {
  miles_audio_debug("set_volume(volume="<<volume<<")");

  // Set the volume even if our volume is not changing, because the
  // MilesAudioManager will call set_volume() when *its* volume
  // changes.

  // Set the volume:
  _volume = volume;

  if (_sample != 0) {
    volume *= _manager->get_volume();
    
    // Change to Miles volume, range 0 to 1.0:
    F32 milesVolume = volume;
    milesVolume = min(milesVolume, 1.0f);
    milesVolume = max(milesVolume, 0.0f);
    
    // Convert balance of -1.0..1.0 to 0-1.0:
    F32 milesBalance = (F32)((_balance + 1.0f) * 0.5f);
    
    AIL_set_sample_volume_pan(_sample, milesVolume, milesBalance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_balance
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_balance(PN_stdfloat balance_right) {
  miles_audio_debug("set_balance(balance_right="<<balance_right<<")");
  _balance = balance_right;

  // Call set_volume to effect the change:
  set_volume(_volume);
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_play_rate
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_play_rate(PN_stdfloat play_rate) {
  miles_audio_debug("set_play_rate(play_rate="<<play_rate<<")");

  // Set the play_rate:
  _play_rate = play_rate;

  if (_sample != 0) {
    play_rate *= _manager->get_play_rate();

    // wave and mp3 use sample rate (e.g. 44100)
    S32 speed = (S32)(play_rate * (PN_stdfloat)_original_playback_rate);
    AIL_set_sample_playback_rate(_sample, speed);
    audio_debug("  play_rate for this wav or mp3 is now " << speed);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::length
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSample::
length() const {
  return _sd->get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::status
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus MilesAudioSample::
status() const {
  if (_sample == 0) {
    return AudioSound::READY;
  }
  switch (AIL_sample_status(_sample)) {
  case SMP_DONE:
  case SMP_STOPPED:
  case SMP_FREE:
    return AudioSound::READY;

  case SMP_PLAYING:
  case SMP_PLAYINGBUTRELEASED:
    return AudioSound::PLAYING;

  default:
    return AudioSound::BAD;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::cleanup
//       Access: Public, Virtual
//  Description: Stops the sound from playing and releases any
//               associated resources, in preparation for releasing
//               the sound or shutting down the sound system.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
cleanup() {
  stop();
  set_active(false);
  nassertv(_sample == 0);

  if (_manager != (MilesAudioManager *)NULL) {
    _manager->release_sound(this);
    _manager = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " " << status();
  if (!_sd.is_null()) {
    out << " " << (_sd->_raw_data.size() + 1023) / 1024 << "K";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound.  Note that
//               Y and Z are switched to translate from Miles's
//               coordinate system.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz) {
  audio_debug("MilesAudioSample::set_3d_attributes()  Setting a sound's 3D Coordinates.");

  if(_sample != 0) {
    AIL_set_sample_3D_position(_sample, px, pz, py);
    AIL_set_sample_3D_velocity_vector(_sample, vx, vz, vy);
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::set_3d_attributes().");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz) {
  audio_debug("MilesAudioSample::get_3d_attributes().");

  if(_sample != 0) {
    float lpx, lpy, lpz, lvx, lvy, lvz;
    AIL_sample_3D_position(_sample, &lpx, &lpz, &lpy);
    AIL_sample_3D_velocity(_sample, &lvx, &lvz, &lvy);
    *px = lpx;
    *py = lpy;
    *pz = lpz;
    *vx = lvx;
    *vy = lvy;
    *vz = lvz;
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::get_3d_attributes().");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_3d_min_distance
//       Access: public
//  Description: Set the distance that this sound begins to fall
//               off.  With Miles's default falloff behavior, when
//               the distance between the sound and the listener is
//               doubled, the volume is halved, and vice versa.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::set_3d_min_distance(PN_stdfloat dist) {
  audio_debug("MilesAudioSample::set_3d_min_distance() Setting the sound's 3D min distance ( min= " << dist << " ) ");

  if(_sample != 0) {
    // Implementation is awkward, since Miles gets and sets min and max distances
    // in a single operation.
    float max_dist;
    int auto_3D_wet_atten;
    AIL_sample_3D_distances(_sample, &max_dist, NULL, &auto_3D_wet_atten);
  
    AIL_set_sample_3D_distances(_sample, max_dist, dist, auto_3D_wet_atten);
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::set_3d_min_distance().");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_3d_min_distance
//       Access: public
//  Description: Get the distance that this sound begins to fall off.
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSample::get_3d_min_distance() const {
  audio_debug("MilesAudioSample::get_3d_min_distance() ");

  if(_sample != 0) {
    float min_dist;
    AIL_sample_3D_distances(_sample, NULL, &min_dist, NULL);
    return (PN_stdfloat)min_dist;
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::get_3d_min_distance().");
    return -1.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_3d_max_distance
//       Access: public
//  Description: Set the distance at which this sound is clipped to
//               silence.  Note that this value does not affect
//               the rate at which the sound falls off, but only
//               the distance at which it gets clipped.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::set_3d_max_distance(PN_stdfloat dist) {
  audio_debug("MilesAudioSample::set_3d_max_distance() Setting the sound's 3D max distance ( max= " << dist << " ) ");

  if(_sample != 0) {
    // Implementation is awkward, since Miles gets and sets min and max distances
    // in a single operation.
    float min_dist;
    int auto_3D_wet_atten;
    AIL_sample_3D_distances(_sample, NULL, &min_dist, &auto_3D_wet_atten);
  
    AIL_set_sample_3D_distances(_sample, dist, min_dist, auto_3D_wet_atten);
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::set_3d_max_distance().");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_3d_max_distance
//       Access: public
//  Description: Get the distance at which this sound is clipped to
//               silence.
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSample::get_3d_max_distance() const {
  audio_debug("MilesAudioSample::get_3d_max_distance() ");

  if(_sample != 0) {
    float max_dist;
    AIL_sample_3D_distances(_sample, &max_dist, NULL, NULL);
    return (PN_stdfloat)max_dist;
  } else {
    audio_warning("_sample == 0 in MilesAudioSample::get_3d_max_distance().");
    return -1.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::get_speaker_level
//       Access: Published
//  Description: Get the level of a particular logical channel (speaker).
//               "index" specifies which speaker in an array of all the
//               logical channels currently in use to retrieve the level
//               of.
//
//               For instance, in a standard 4.0 channel setup, speakers
//               are setup as [frontLeft, frontRight, backLeft, backRight].
//               Thus, get_speaker_level(2) will retrieve the level of the
//               backLeft speaker.
//
//               The order in which speakers appear in the array for
//               standard speaker setups is defined to be:
//
//                  FRONT_LEFT 
//                  FRONT_RIGHT 
//                  FRONT_CENTER 
//                  LOW_FREQUENCY (sub woofer)
//                  BACK_LEFT 
//                  BACK_RIGHT 
//                  FRONT_LEFT_OF_CENTER 
//                  FRONT_RIGHT_OF_CENTER 
//                  BACK_CENTER 
//                  SIDE_LEFT 
//                  SIDE_RIGHT 
//                  TOP_CENTER 
//                  TOP_FRONT_LEFT 
//                  TOP_FRONT_CENTER 
//                  TOP_FRONT_RIGHT 
//                  TOP_BACK_LEFT 
//                  TOP_BACK_CENTER 
//                  TOP_BACK_RIGHT 
//               
////////////////////////////////////////////////////////////////////
PN_stdfloat MilesAudioSample::
get_speaker_level(int index) {
  audio_debug("MilesAudioSample::get_speaker_level(" << index << ")");

  if(_sample != 0) {
    int numLevels;
    float *levels = AIL_sample_channel_levels(_sample, &numLevels);

    if(index < numLevels) {
      return (PN_stdfloat)levels[index];
    } else {
      audio_error("index out of range in MilesAudioSample::get_speaker_level.  numLevels: " << numLevels);
      return -1.0;
    }
  } else {
    audio_warning("Warning: MilesAudioSample::get_speaker_level only works for sounds that are currently playing");
    return -1.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::set_speaker_levels
//       Access: Published
//  Description: Set the output levels on the logical channels (speakers)
//               for this sound.  Values should be in the range 0.0 to 1.0.
//               Levels for up to nine channels may be specified. As soon
//               as a level is reached that falls outside the range 0.0 to
//               1.0, the levels specified up to that point will be sent
//               and all other levels will be ignored.
//
//               The user must know what the current speaker setup is in order
//               to know which level corresponds to which speaker.
//
//               This method will have no effect if 3D attributes have been
//               set for this sound.
//
//               The order in which speakers appear in the array for
//               standard speaker setups is defined to be:
//
//                  FRONT_LEFT 
//                  FRONT_RIGHT 
//                  FRONT_CENTER 
//                  LOW_FREQUENCY (sub woofer)
//                  BACK_LEFT 
//                  BACK_RIGHT 
//                  FRONT_LEFT_OF_CENTER 
//                  FRONT_RIGHT_OF_CENTER 
//                  BACK_CENTER 
//                  SIDE_LEFT 
//                  SIDE_RIGHT 
//                  TOP_CENTER 
//                  TOP_FRONT_LEFT 
//                  TOP_FRONT_CENTER 
//                  TOP_FRONT_RIGHT 
//                  TOP_BACK_LEFT 
//                  TOP_BACK_CENTER 
//                  TOP_BACK_RIGHT 
//               
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
set_speaker_levels(PN_stdfloat level1, PN_stdfloat level2, PN_stdfloat level3, PN_stdfloat level4, PN_stdfloat level5, PN_stdfloat level6, PN_stdfloat level7, PN_stdfloat level8, PN_stdfloat level9) {
  audio_debug("MilesAudioSample::set_speaker_levels()");

  if(_sample != 0) {
    float levels[9] = {level1, level2, level3, level4, level5, level6, level7, level8, level9};

    if((level1 < 0.0) || (level1 > 1.0)) {
      audio_error("No valid levels specified in MilesAudioSample::set_speaker_levels().");
    } else if((level2 < 0.0) || (level2 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 1);
    } else if((level3 < 0.0) || (level3 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 2);
    } else if((level4 < 0.0) || (level4 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 3);
    } else if((level5 < 0.0) || (level5 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 4);
    } else if((level6 < 0.0) || (level6 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 5);
    } else if((level7 < 0.0) || (level7 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 6);
    } else if((level8 < 0.0) || (level8 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 7);
    } else if((level9 < 0.0) || (level9 > 1.0)) {
      AIL_set_sample_channel_levels(_sample, levels, 8);
    } else {
      AIL_set_sample_channel_levels(_sample, levels, 9);
    }
  } else {
    audio_warning("Warning: MilesAudioSample::set_speaker_levels only works for sounds that are currently playing");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::internal_stop
//       Access: Private
//  Description: Called by the GlobalMilesManager when it is detected
//               that this particular sound has already stopped, and
//               its sample handle will be recycled.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
internal_stop() {
  _sample = 0;
  _sample_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::finish_callback
//       Access: Private, Static
//  Description: This callback is made by Miles (possibly in a
//               sub-thread) when the sample finishes.
////////////////////////////////////////////////////////////////////
void AILCALLBACK MilesAudioSample::
finish_callback(HSAMPLE sample) {
  MilesAudioSample *self = (MilesAudioSample *)AIL_sample_user_data(sample, 0);
  if (milesAudio_cat.is_debug()) {
    milesAudio_cat.debug()
      << "finished " << *self << "\n";
  }
  if (self->_manager == (MilesAudioManager *)NULL) {
    return;
  }
  self->_manager->_sounds_finished = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MilesAudioSample::do_set_time
//       Access: Private
//  Description: Sets the start time of an already allocated sample.
////////////////////////////////////////////////////////////////////
void MilesAudioSample::
do_set_time(PN_stdfloat time) {
  miles_audio_debug("do_set_time(time="<<time<<")");
  nassertv(_sample != 0);

  // Ensure we don't inadvertently run off the end of the sound.
  PN_stdfloat max_time = length();
  if (time > max_time) {
    milesAudio_cat.warning()
      << "set_time(" << time << ") requested for sound of length " 
      << max_time << "\n";
    time = max_time;
  }
  
  S32 time_ms = (S32)(1000.0f * time);
  AIL_set_sample_ms_position(_sample, time_ms);
}

#endif //]
