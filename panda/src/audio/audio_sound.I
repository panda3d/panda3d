// Filename: audio_sound.I
// Created by:  cary (17Oct00)
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

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::constructor
//       Access: Protected
//  Description: initialize a new sound
////////////////////////////////////////////////////////////////////
INLINE AudioSound::AudioSound(AudioTraits::SoundClass* sound,
                              AudioTraits::PlayingClass* state,
                              AudioTraits::PlayerClass* player,
                              AudioTraits::DeletePlayingFunc* destroy,
                              const string& filename) : Namable(filename),
                                                        _sound(sound),
                                                        _state(state),
                                                        _player(player),
                                                        _delstate(destroy) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::copy constructor
//       Access: Protected
//  Description: copy a sound, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioSound::AudioSound(const AudioSound& c) : Namable(c.get_name()),
                                                     _sound(c._sound),
                                                     _state(c._state),
                                                     _player(c._player),
                                                     _delstate(c._delstate) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::assignment operator
//       Access: Protected
//  Description: copy a sound, but we don't really want to allow this
////////////////////////////////////////////////////////////////////
INLINE AudioSound& AudioSound::operator=(const AudioSound& c) {
  this->set_name(c.get_name());
  this->_sound = c._sound;
  this->_state = c._state;
  this->_player = c._player;
  this->_delstate = c._delstate;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_player
//       Access: Protected
//  Description: return the player for this sound
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::PlayerClass* AudioSound::get_player(void) const {
  return _player;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_sound
//       Access: Protected
//  Description: return the trait sound class for this sound
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::SoundClass* AudioSound::get_sound(void) const {
  return _sound;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_state
//       Access: Protected
//  Description: return the trait playing class for this sound
////////////////////////////////////////////////////////////////////
INLINE AudioTraits::PlayingClass* AudioSound::get_state(void) const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::equality operator
//       Access: Public
//  Description: test to see if two sounds are the same
////////////////////////////////////////////////////////////////////
INLINE bool AudioSound::operator==(const AudioSound& c) const {
  return ((_sound == c._sound) && (_state == c._state));
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::inequality operator
//       Access: Public
//  Description: test to see if two sounds are not the same
////////////////////////////////////////////////////////////////////
INLINE bool AudioSound::operator!=(const AudioSound& c) const {
  return ((_sound != c._sound) || (_state != c._state));
}
