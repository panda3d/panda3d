// Filename: audio_sound.cxx
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

#include "audio_sound.h"
#include "config_audio.h"
#include "audio_manager.h"

TypeHandle AudioSound::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::destructor
//       Access: Public
//  Description: deletes the sound data and then lets the system
//               destroy this structure
////////////////////////////////////////////////////////////////////
AudioSound::
~AudioSound() {
  audio_debug("AudioSound destructor (" << get_name() << ")");
  AudioManager::stop(this);
  (*_delstate)(_state);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::length
//       Access: Public
//  Description: return the length (in seconds) of the sound
////////////////////////////////////////////////////////////////////
float AudioSound::
length() const {
  return _sound->length();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::status
//       Access: Public
//  Description: return the current play status of this sound
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus AudioSound::
status() const {
  AudioTraits::PlayingClass::PlayingStatus stat = _state->status();
  switch (stat) {
  case AudioTraits::PlayingClass::BAD:
    return BAD;
  case AudioTraits::PlayingClass::READY:
    return READY;
  case AudioTraits::PlayingClass::PLAYING:
    return PLAYING;
  }
  audio_error("unknown status for sound");
  return BAD;
}
