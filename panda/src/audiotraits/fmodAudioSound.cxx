// Filename: fmodAudioSound.cxx
// Created by:  cort (January 22, 2003)
// Extended by: ben  (October 22, 2003)
// Prior system by: cary
// Rewrite [for new Version of FMOD-EX] by: Stan Rosenbaum "Staque" - Spring 2006
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
#include "dcast.h"

#ifdef HAVE_FMODEX //[

//Panda Headers
#include "config_audio.h"
#include "fmodAudioSound.h"
#include "fmodAudioDSP.h"

TypeHandle FmodAudioSound::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::FmodAudioSound
//       Access: public
//  Description: CONSTRUCTOR
//               All sound will DEFAULT load as a 2D sound unless
//         otherwise specified.
////////////////////////////////////////////////////////////////////

FmodAudioSound::
FmodAudioSound(AudioManager *manager, string file_name, bool positional) { 
  audio_debug("FmodAudioSound::FmodAudioSound() Creating new sound, filename: " << file_name  );

  //Local Variables that are needed.
  FMOD_RESULT result;

  //Inits 3D Attributes
  _location.x = 0;
  _location.y = 0;
  _location.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  //Play Rate Variable
  _playrate = 1;

  // These set the Speaker Levels to a default if you are using a MultiChannel Setup.
  _frontleft = 1;
  _frontright = 1;
  _center = 1;
  _sub = 1;
  _backleft = 1;
  _backright = 1;
  _sideleft = 1;
  _sideright = 1;

  //Assign the values we need
  FmodAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  //_channel = 0;
  _file_name = file_name;

  //Get the Speaker Mode [Important for later on.]
  result = _manager->_system->getSpeakerMode( &_speakermode );
  ERRCHECK(result);

  if (positional == true) {
  
    result = _manager->_system->createSound( file_name.c_str(), FMOD_SOFTWARE | FMOD_3D , 0, &_sound);
    ERRCHECK(result);

    //This is just to collect the defaults of the sound, so we don't
    //Have to query FMOD everytime for the info.
    //It is also important we get the '_sampleFrequency' variable here, for the
    //'set_play_rate()' and 'get_play_rate()' methods later;
    
    result = _sound->getDefaults( &_sampleFrequency, &_volume , &_balance, &_priority);
    ERRCHECK(result);

    audio_debug("Sound loaded as 3D");

  } else {

    result = _manager->_system->createSound( file_name.c_str(), FMOD_SOFTWARE | FMOD_2D , 0, &_sound);
    ERRCHECK(result);
  
    //This is just to collect the defaults of the sound, so we don't
    //Have to query FMOD everytime for the info.
    //It is also important we get the '_sampleFrequency' variable here, for the
    //'set_play_rate()' and 'get_play_rate()' methods later;

    result = _sound->getDefaults( &_sampleFrequency, &_volume , &_balance, &_priority);
    ERRCHECK(result);

    audio_debug("Sound loaded as 2D");

  }
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::~FmodAudioSound
//       Access: public
//  Description: DESTRUCTOR!!!
////////////////////////////////////////////////////////////////////
FmodAudioSound::
~FmodAudioSound() {

  audio_debug("FmodAudioSound::~FmodAudioSound()  Closing this Sound Instance Down.");

  FMOD_RESULT result;

  //Remove me from table of all sounds.
  _manager->_all_sounds.erase(this);

  //Release DSPs First
  _sound_dsp.clear();

  //The Release Sound
  result = _sound->release();
  ERRCHECK(result);

  audio_debug("FmodAudioSound::~FmodAudioSound()  FMOD Sound Released and Closed.");

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound:: play
//       Access: public
//  Description: Plays a sound.
//        OK when you play a Sound a couple of things are going
//        to happen.
//        First Panda is going to check if the sound is in a pause
//        State because of if a 'set_time()' has been called on the sound.
//        Second, remember when I said channels don't exist unless a sound,
//        is playing on them?  Well they don't  and therefore any associated
//        DSP, looping, playback rate, pan, volume, or 3D information  associated with
//        that sound is not going to be known by that channel either.
//        So all we do, is load a sound on a channel PAUSED.
//        While the sound is paused, it sets up all the sound attributes, and 
//        then plays the sound.  This all happens really fast, so you should
//        notice a hit.
//        And yes this is the FMOD way of doing things.
//        
//        The biggest problem with this comes when you are 'looping' sounds.
//        In the 'prepare2DSound()' and prepare3DSound' [private methods
//        which set up their respected type of sounds], I had to impliment the
//        'FMOD_CHANNEL_REUSE' flag.
//        Remember back in the header file, where I said there is only one dedicated,
//        Channel for each sound, where there is a reason for that.
//        If you were to use 'FMOD_CHANNEL_FREE' each time you played a sound,
//        FMOD would grab the first available channel and play the sound on that.
//        This is bad for Panda because the FmodAudioSound class is only set up to 
//        remember only channel at a time.   If we play a sound that is set to loop, and
//        then trigger that sound to loop again, the reference to the Channel Object for a particular
//        sound is replaced with the new, instance of the loop.  Thus the previous sound keeps playing
//        forever because we have lost the reference to it.
//        But using the 'FMOD_CHANNEL_REUSE' flag, we avoid this problem, because the new call to play
//        the looping sound will, play the sound on the previous allocated channel.
//        The only to downside to this, is that playing a looping sound a second time, will cause the
//        Previous instance to be cut off.
//
//        Honestly, it should be easy to create a work around for this.  So you could 
//        use the 'FMOD_CHANNEL_FREE' which will prevent the cutoff program, by creating 
//        a set, that keeps track of channels playing the same looping sound, when the sound is looping 
//        [or is really long, another possible scenerio for this problem].
//        
//        But honestly the cutoff shouldn't be a big problem, in most cases.
//        I just wanted to explain my reasoning here.
//        
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
play() {
  
  audio_debug("FmodAudioSound::play()   Going to play a sound." );

  FMOD_RESULT result;
  bool paused = 0;

  result = _channel->getPaused(&paused);
  ERRCHECK(result); 


  if ( paused ) {

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();
    add_dsp_on_channel();

    result = _channel->setPaused(false);
    ERRCHECK(result);

  } else {

    prepareSound();

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();;
    add_dsp_on_channel();

    result = _channel->setPaused(false);
    ERRCHECK(result);

  }

  audio_debug("FmodAudioSound::play()  Sound should be playing (or played if it is really short)." );

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::prepareSound
//       Access: Private
//  Description: Prepares a sound [GENERAL]
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
prepareSound() {
  
  audio_debug("FmodAudioSound::prepareSound()" );

  FMOD_RESULT result;
  bool paused = 0;
  FMOD_MODE soundMode;

  audio_debug("FmodAudioSound::play()   Going to perpare a sound." );

  result = _sound->getMode(&soundMode);
  ERRCHECK(result);

  if ( ( soundMode & FMOD_3D ) > 0 ) {

    prepare3DSound();
    
  } else {
  
    prepare2DSound();

  }

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::prepare2DSound
//       Access: Private
//  Description: Prepares a 2D sound.
//        We need seperate 3D and 2D play commands, because of
//        the way FMOD sets the 3D Attributes to the 'channel' a
//        sound is playing on.
//        Since a 'channel' is initialize when a sound is set to 
//        play to one, and then subsiquenctly destroyed once the
//        sound is finished playing.
//        
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
prepare2DSound() {
  
  audio_debug("FmodAudioSound::prepare2DSound()" );

  FMOD_RESULT result;

  result = _manager->_system->playSound(FMOD_CHANNEL_REUSE, _sound, true, &_channel);
  ERRCHECK(result);

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::prepare3DSound
//       Access: Private
//  Description: Prepares a 3D sound
//        We need seperate 3D and 2D play commands, because of
//        the way FMOD sets the 3D Attributes to the 'channel' a
//        sound is playing on.
//        Since a 'channel' is initialize when a sound is set to 
//        play to one, and then subsiquenctly destroyed once the
//        sound is finished playing.
//        
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
prepare3DSound() {
  
  audio_debug("FmodAudioSound::prepare2DSound()" );

  FMOD_RESULT result;

  result = _manager->_system->playSound(FMOD_CHANNEL_REUSE, _sound, true, &_channel);
  ERRCHECK(result);

  result = _channel->set3DAttributes( &_location, &_velocity );
  ERRCHECK(result);

}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::stop
//       Access: public
//  Description: Stop a sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
stop() {
  audio_debug("FmodAudioSound::stop()  Going to stop a sound." );

  //LOCALS
  FMOD_RESULT result;

  result = _channel->stop();
  ERRCHECK(result);

  audio_debug("FmodAudioSound::stop()  Sound should be stopped.");
}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop
//       Access: public
//  Description: Turns looping on and off
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_loop(bool loop) {
  audio_debug("FmodAudioSound::set_loop()  Setting loop state to " << loop);
  FMOD_RESULT result;
  if ( loop ) {
    result = _sound->setMode(FMOD_LOOP_NORMAL);
    ERRCHECK(result);
    audio_debug("This sound is set to loop." );

  } else {
    result = _sound->setMode(FMOD_LOOP_OFF);  
    ERRCHECK(result);
    audio_debug("FmodAudioSound::set_loop()  This sound is set to one-shot." );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop
//       Access: public
//  Description: Returns whether looping is on or off
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
get_loop() const {
  
  audio_debug( "FmodAudioSound::get_loop()   Retreiving a sound's loop state." );
  
  // 0 means loop forever,
  // >1 means loop that many times
  // So _loop_count != 1 means we're looping

  FMOD_RESULT result;
  FMOD_MODE loopMode;
  bool loopState;

  result = _sound->getMode( &loopMode );
  ERRCHECK(result);

  if ( (loopMode & FMOD_LOOP_NORMAL) != 0 ) {
    loopState = true;
  } else {
    loopState = false;
  }

  return (loopState);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_loop_count
//       Access: public
//  Description: 
//        Panda uses 0 to mean loop forever.
//        Fmod uses negative numbers to mean loop forever.
//        (0 means don't loop, 1 means play twice, etc.
//        We must convert!
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_loop_count(unsigned long loop_count) {

  audio_debug("FmodAudioSound::set_loop_count()   Setting the sound's loop count to: " << loop_count);

  //LOCALS
  FMOD_RESULT result;
  int numberOfLoops;

  numberOfLoops = (int) loop_count;
  numberOfLoops = numberOfLoops - 1;

  if (numberOfLoops == 0) {
    result = _sound->setLoopCount( -1 );
    ERRCHECK(result);
  } else {
    result = _sound->setLoopCount( numberOfLoops );
    ERRCHECK(result);
  }

  audio_debug("FmodAudioSound::set_loop_count()   Sound's loop count should be set to: " << loop_count);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_loop_count
//       Access: public
//  Description: Return how many times a sound will loop.
////////////////////////////////////////////////////////////////////
unsigned long FmodAudioSound::
get_loop_count() const {

  audio_debug("FmodAudioSound::get_loop_count() Getting the sound's loop count. ");

  FMOD_RESULT result;
  int loop_count;
  unsigned long returnedNumber;

  result = _sound->getLoopCount( &loop_count );
  ERRCHECK(result);

  audio_debug("FmodAudioSound::get_loop_count() returning "<< loop_count);

  returnedNumber = (unsigned long) loop_count;

  return loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_time
//       Access: public
//  Description: Sets the play position within the sound
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_time(float start_time) {
  
  FMOD_RESULT result;
  audio_debug("FmodAudioSound::set_time()   Going to set a sounds start position" );

  unsigned int startTime;

  //We must 'prepareSound()' [set it to play on a channel] so
  //we can set its start time.
  prepareSound();

  startTime = start_time * 1000;

  result = _channel->setPosition( startTime , FMOD_TIMEUNIT_MS );
  ERRCHECK(result);

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_time
//       Access: public
//  Description: Gets the play position within the sound
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_time() const {
  audio_debug("FmodAudioSound::get_time()   Going to get a sound's position" );

  FMOD_RESULT result;
  unsigned int current_time;

  result = _channel->getPosition( &current_time , FMOD_TIMEUNIT_MS );
  ERRCHECK(result);

  current_time = current_time / 1000;

  return current_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_volume(float vol)
//       Access: public
//  Description: 0.0 to 1.0 scale of volume converted to Fmod's
//               internal 0.0 to 255.0 scale.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_volume(float vol) {
      
  audio_debug("FmodAudioSound::set_volume() Going to set a sounds volume." );

  FMOD_RESULT result;

  _volume = vol;

  result = _channel->setVolume( _volume );
  ERRCHECK(result);

  audio_debug("FmodAudioSound::set_volume()  Setting volume to " << vol);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_volume_on_channel()
//       Access: Private
//  Description: Set the volume on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_volume_on_channel() {
  audio_debug("FmodAudioSound::set_volume() Going to set a sounds volume." );

  FMOD_RESULT result;

  result = _channel->setVolume( _volume );
  ERRCHECK(result);

  audio_debug("FmodAudioSound::set_volume()  Setting volume to " << _volume );
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_volume
//       Access: public
//  Description: Gets the current volume of a sound.  1 is Max. O is Min.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_volume() const {

  audio_debug("FmodAudioSound::get_volume() Going to get a sound's volume." );

  return _volume;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_balance(float bal)
//       Access: public
//  Description: -1.0 to 1.0 scale
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_balance(float bal) {
  audio_debug("FmodAudioSound::set_balance()  Going to set a sound's balance." );

  FMOD_RESULT result;

  _balance = bal;

  result = _sound->setDefaults( _sampleFrequency, _volume , _balance, _priority);
  ERRCHECK(result);

  result = _channel->setPan( _balance );
  ERRCHECK(result);

  audio_debug("FmodAudioSound::set_balance()    Setting Pan to " << bal);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_balance_on_channel()
//       Access: public
//  Description: -1.0 to 1.0 scale  Set the pan on a prepared Sound channel.
//        -1 should be all the way left.
//        1 is all the way to the right.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_balance_on_channel() {
  
  audio_debug("FmodAudioSound::set_balance()  Going to set a sound's balance to " << _balance );

  FMOD_RESULT result;

  result = _channel->setPan( _balance );
  ERRCHECK(result);

  audio_debug("FmodAudioSound::set_balance()    Setting Pan to " << _balance);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_balance
//       Access: public
//  Description: -1.0 to 1.0 scale 
//        -1 should be all the way left.
//        1 is all the way to the right.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_balance() const {
  
  audio_debug("FmodAudioSound::get_balance()  Going to get a sound's balance." );

  return _balance;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_play_rate(float rate)
//       Access: public
//  Description: Sets the speed at which a sound plays back.
//        The rate is a multiple of the sound, normal playback speed.
//        IE 2 would play back 2 times fast, 3 would play 3 times, and so on.
//        This can also be set to a negative number so a sound plays backwards.
//        But rememeber if the sound is not playing, you must set the 
//        sound's time to its end to hear a song play backwards.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_play_rate(float rate) {
  audio_debug("FmodAudioSound::set_play_rate()  Going to set a sound's play rate to " << rate);

  FMOD_RESULT result;
  float frequencyToSetChannelTo;

  _playrate = rate;

  if (rate == 1) {
    result = _channel->setFrequency( _sampleFrequency );
    ERRCHECK(result);
  } else {
    frequencyToSetChannelTo = _sampleFrequency * rate ;

    result = _channel->setFrequency( frequencyToSetChannelTo );
    ERRCHECK(result);
  
  }
  
  audio_debug("FmodAudioSound::set_play_rate()  Sound's balance set to " << rate);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_play_rate_on_channel()
//       Access: public
//  Description: Set the play rate on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_play_rate_on_channel() {
  audio_debug("FmodAudioSound::set_play_rate()  Going to set a sound's balance to " << _playrate);

  FMOD_RESULT result;
  float frequencyToSetChannelTo;

  if ( _playrate == 1) {
    result = _channel->setFrequency( _sampleFrequency );
    ERRCHECK(result);
  } else {
    frequencyToSetChannelTo = _sampleFrequency * _playrate ;

    result = _channel->setFrequency( frequencyToSetChannelTo );
    ERRCHECK(result);
  
  }
  
  audio_debug("FmodAudioSound::set_play_rate()  Sound's balance set to " << _playrate);

}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_play_rate
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_play_rate() const {

  audio_debug("FmodAudioSound::set_play_rate()  Going to get a sound's balance.");

  return _playrate;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_name
//       Access: public
//  Description: Get name of sound file
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::
get_name() const {
  audio_debug("FmodAudioSound::get_name()  Going to get a sound's file name.");

  return _file_name;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::length
//       Access: public
//  Description: Get length
//        FMOD returns the time in MS  so we have to convert to seconds.
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
length() const {
  
  audio_debug("FmodAudioSound::length()  Going to get a sound's length in second.");

  FMOD_RESULT result;
  unsigned int length;

  result = _sound->getLength( &length, FMOD_TIMEUNIT_MS );
  ERRCHECK(result);

  length = length / 1000;

  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_attributes
//       Access: public
//  Description: Set position and velocity of this sound
//        NOW LISTEN UP!!! THIS IS IMPORTANT!
//        Both Panda3D and FMOD use a left handed coordinate system.
//        But there is a major difference!
//        In Panda3D the Y-Axis is going into the Screen and the Z-Axis is going up.
//        In FMOD the Y-Axis is going up and the Z-Axis is going into the screen.
//        The solution is simple, we just flip the Y and Z axis, as we move coordinates
//        from Panda to FMOD and back.
//        What does did mean to average Panda user?  Nothing, they shouldn't notice anyway.
//        But if you decide to do any 3D audio work in here you have to keep it in mind.
//        I told you, so you can't say I didn't.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
  audio_debug("FmodAudioSound::set_3d_attributes()  Setting a sound's 3D Coordinates.");

  FMOD_RESULT result;
  
  _location.x = px;
  _location.y = pz;
  _location.z = py;
  
  _velocity.x = vx;
  _velocity.y = vz;
  _velocity.z = vy;
  
  result = _channel->set3DAttributes( &_location, &_velocity );
  ERRCHECK(result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_attributes
//       Access: public
//  Description: Get position and velocity of this sound
//         Currently unimplemented. Get the attributes of the attached object.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
  audio_error("get3dAttributes: Currently unimplemented. Get the attributes of the attached object.");

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_min_distance
//       Access: public
//  Description: Set the distance that this sound begins to fall off. Also
//               affects the rate it falls off.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_min_distance(float dist) {
  
  audio_debug("FmodAudioSound::set_3d_min_distance() Setting the sound's 3D min distance ( min= " << dist << " ) ");

  FMOD_RESULT result;

  _min_dist = dist;

  result = _sound->set3DMinMaxDistance( dist, _max_dist );
  ERRCHECK(result);


}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_min_distance
//       Access: public
//  Description: Get the distance that this sound begins to fall off
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_3d_min_distance() const {

  audio_debug("FmodAudioSound::get_3d_min_distance() ");

  return _min_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_3d_max_distance
//       Access: public
//  Description: Set the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_3d_max_distance(float dist) {

  audio_debug("FmodAudioSound::set_3d_max_distance() Setting the sound's 3D max distance ( max= " << dist << " ) ");

  FMOD_RESULT result;

  _max_dist = dist;

  result = _sound->set3DMinMaxDistance( _min_dist, dist );
  ERRCHECK(result);

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_3d_max_distance
//       Access: public
//  Description: Get the distance that this sound stops falling off
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_3d_max_distance() const {
  
  audio_debug("FmodAudioSound::get_3d_max_distance() ");

  return _max_dist;
}




////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::add_dsp
//       Access: Published
//  Description: This adds a DSP effect to a Sound's personal DSP Chain.
//        DSPs set here will only affect it respective sound.
//        
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
add_dsp( PT(AudioDSP) x) {
  audio_debug("FmodAudioManager()::add_dsp");

  FMOD_RESULT result;
  bool playingState;

  FmodAudioDSP *fdsp;
  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {

    audio_debug("This DSP has already been assigned to the system or a sound.");

    return false;

  } else
    {

      _sound_dsp.insert(fdsp);

      if ( _channel != 0 ) {
        result = _channel->isPlaying( &playingState );
        ERRCHECK(result);
        if ( playingState ) {
          result = _channel->addDSP( fdsp->_dsp );
          ERRCHECK( result );
        }
      }

      fdsp->set_in_chain(true);

      return true;

    }

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::add_dsp_on_channel()
//       Access: Published
//  Description: Sets the DSPs on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
add_dsp_on_channel() {
  audio_debug("FmodAudioManager()::add_dsp_on_channel");

  FMOD_RESULT result;

  for (DSPSet::iterator i = _sound_dsp.begin(); i != _sound_dsp.end(); ++i) {

    result = _channel->addDSP( (*i)->_dsp );
    ERRCHECK( result );

  }
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::remove_dsp
//       Access: Published
//  Description: This will remove a DSP from a Sound's DSP Chain, but
//        It will not destroy it!
//        So in theory you could reuse the DSP for something else.
//        In the Global Chain or another sound.
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
remove_dsp(PT(AudioDSP) x) {
  // intentionally blank
  
  audio_debug("FmodAudioManager()::remove_dsp()");

  FMOD_RESULT result;

  FmodAudioDSP *fdsp;
  DCAST_INTO_R(fdsp, x, false);

  if ( fdsp->get_in_chain() ) {

    result = fdsp->_dsp->remove();
    ERRCHECK( result );

    _sound_dsp.erase(fdsp);

    fdsp->set_in_chain(false);

    return true;

  } else {

    audio_debug("FmodAudioManager()::remove_dsp()");
    audio_debug("This DSP doesn't exist in this chain.");
    
    return false;
    
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_speaker_mix
//       Access: Published
//  Description: In Multichannel Speaker systems [like Surround].
//        Can pass in the ID of a specific speaker and get itself Volume level.
//        Currenly that is set to the Following:
//        1 = Front Left
//        2 = Front Right
//        3 = Center
//        4 = Subwoofer
//        5 = Back Left
//        6 = Back Right
//        7 = Side Left
//        8 = Side Right
//        Speakers which don't exist in some systems will simply be ignored.
//        IE So a quadsystem would only return values on 1,2,5,6.
//
//        But I haven't been able to test this yet, so I am jsut letting you know.
//
//        BTW This will also work in Stereo speaker systems, but since
//        PANDA/FMOD has a balance [pan] function what is the point?
////////////////////////////////////////////////////////////////////
float FmodAudioSound::
get_speaker_mix(int speaker) {
  // intentionally blank

  audio_debug("FmodAudioSound::getSpeakerMix()");

  FMOD_RESULT result;
  float frontleft;
  float frontright;
  float center;
  float sub;
  float backleft;
  float backright; 
  float sideleft;
  float sideright;

  float returnValue;

  switch(speaker) {
  
  case 1: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = frontleft;
    break;
  case 2: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = frontright;
    break;
  case 3: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = center;
    break;
  case 4: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = sub;
    break;
  case 5: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = backleft;
    break;
  case 6: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = backright;
    break;
  case 7: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = sideleft;
    break;
  case 8: 
    result = _channel->getSpeakerMix( &frontleft, &frontright, &center, &sub, &backleft, &backright, &sideleft, &sideright );
    ERRCHECK(result);
    returnValue = sideright;
    break;
  default: 
    cerr << "You specified a speaker which doesn't exist.";
  }

  return returnValue;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_speaker_mix
//       Access: Published
//  Description: This sets the speaker mix for Surround Sound sytems.
//        It required 8 parameters which match up to the following:
//        1 = Front Left
//        2 = Front Right
//        3 = Center
//        4 = Subwoofer
//        5 = Back Left
//        6 = Back Right
//        7 = Side Left
//        8 = Side Right
//        Speakers which don't exist in some systems will simply be ignored.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_speaker_mix(float frontleft, float frontright, float center, float sub, float backleft, float backright, float sideleft, float  sideright) {
  // intentionally blank

  audio_debug("FmodAudioSound::setSpeakerMix()");

  FMOD_RESULT result;

  _frontleft = frontleft;
  _frontright = frontright;
  _center = center;
  _sub = sub;
  _backleft = backleft;
  _backright = backright;
  _sideleft = sideleft;
  _sideright = sideright;

  result = _channel->setSpeakerMix( _frontleft, _frontright, _center, _sub, _backleft, _backright, _sideleft, _sideright );
  ERRCHECK(result);;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_speaker_mix_on_channel
//       Access: Published
//  Description: Set the Speaker Mix for a sound on a prepared Sound channel.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_speaker_mix_on_channel() {
  // intentionally blank

  audio_debug("FmodAudioSound::setSpeakerMix()");

  FMOD_RESULT result;

  result = _channel->setSpeakerMix( _frontleft, _frontright, _center, _sub, _backleft, _backright, _sideleft, _sideright );
  ERRCHECK(result);;
}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_speaker_mix_or_balance_on_channel
//       Access: Private
//  Description: This is simply a safety catch.
//        If you are using a Stero speaker setup Panda will only pay attention
//        to 'set_balance()' command when setting speaker balances.
//        Other wise it will use 'set_speaker_mix'.
//        I put this in, because other wise you end up with a sitation,
//        where 'set_speaker_mix()' or 'set_balace()' will override any
//        previous speaker balance setups.  It all depends on which was called last.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_speaker_mix_or_balance_on_channel() {
  // intentionally blank

  audio_debug("FmodAudioSound::set_speaker_mix_or_balance_on_channel()");

  FMOD_RESULT result;
  FMOD_MODE soundMode;


  result = _sound->getMode(&soundMode);
  ERRCHECK(result);


  if ( _speakermode == FMOD_SPEAKERMODE_STEREO ) {

    //FMOD Returns an error is you try and pan a sound in 3D Audio.
    //Which makes sense.
    //It is nothing serious,  but might as well avoid it while we can.
    if ( !( ( soundMode & FMOD_3D ) > 0 ) ) {
      set_balance_on_channel();
    }

  } else {

    set_speaker_mix_on_channel();

  }

}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_priority
//       Access: Published
//  Description: Sets the priority of a sound.
//        This is what FMOD uses to determine is a sound will
//        play if all the other real channels have been used up.
////////////////////////////////////////////////////////////////////
int FmodAudioSound::
get_priority() {
  audio_debug("FmodAudioSound::get_priority()");
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_priority(int priority)
//       Access: Published
//  Description: Sets the Sound Priority [Whether is will be played
//        over other sound when real audio channels become short.
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_priority(int priority) {
  // intentionally blank

  audio_debug("FmodAudioSound::set_priority()");

  FMOD_RESULT result;

  _priority = priority;

  result = _sound->setDefaults( _sampleFrequency, _volume , _balance, _priority);
  ERRCHECK(result);
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::status
//       Access: public
//  Description: Get status of the sound.
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus FmodAudioSound::
status() const {
  // If the stream's channel isn't playing anything, then the stream
  // definitely isn't playing.
  
  audio_debug("FmodAudioSound::status() ");

  FMOD_RESULT result;
  bool playingState;

  if ( _channel != 0 ) {
    result = _channel->isPlaying( &playingState );
    ERRCHECK(result);
  }

  //audio_debug("If you get 'FMOD State: 32 An invalid object handle was used.' ");
  //audio_debug("It doesn't mean there is a problem with FMOD or the sound.");
  //audio_debug( "It just means your sound isn't playing." );


  if (playingState) {
    return PLAYING;
  } else {
    return READY;
  }
  
  //return BAD;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_active
//       Access: public
//  Description: NOT USED ANYMORE!!!
//
//        
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_active(bool active) {
  audio_debug( "set_active(active=" << active << ")" );
  audio_debug("NOT USED ANYMORE in FMOD-EX version of PANDA.");
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_active 
//       Access: public
//  Description: NOT 'REALLY' USED ANYMORE!!!
//        This is actually an layover from the old version of the FMOD code.
//        The old version used a weird cache system to keep track of the sounds' states.
//        I just converted this method, to return if a sound is 'Playing' or not.
//        This is not exactly how the orignal use of this function btu I figured I might
//        as well get some use out of it.
////////////////////////////////////////////////////////////////////
bool FmodAudioSound::
get_active() const {
  audio_debug("FmodAudioSound::get_active()  Going to get a sound's activity.");
  audio_debug("NOT USED ANYMORE in FMOD-EX version of PANDA.");
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::finished
//       Access: public
//  Description: NOT USED ANYMORE!!!
//        Called by finishedCallback function when a sound
//              terminates (but doesn't loop).
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
finished() {
  audio_debug("FmodAudioSound::finished()");
  audio_debug("NOT USED ANYMORE in FMOD-EX version of PANDA.");
  stop();
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::set_finished_event
//       Access: public
//  Description: NOT USED ANYMORE!!!
//        Assign a string for the finished event to be referenced 
//              by in python by an accept method
//
////////////////////////////////////////////////////////////////////
void FmodAudioSound::
set_finished_event(const string& event) {
  audio_debug("FmodAudioSound::set_finished_event(event="<<event<<")");
  audio_debug("NOT USED ANYMORE in FMOD-EX version of PANDA.");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioSound::get_finished_event
//       Access: public
//  Description:NOT USED ANYMORE!!!
//        Return the string the finished event is referenced by
//
//        
////////////////////////////////////////////////////////////////////
const string& FmodAudioSound::
get_finished_event() const {
  audio_debug("FmodAudioSound::get_finished_event() returning " << _finished_event );
  audio_debug("NOT USED ANYMORE in FMOD-EX version of PANDA.");
  return _finished_event;
}

#endif //]
