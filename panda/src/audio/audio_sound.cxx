// Filename: audio_sound.cxx
// Created by:  cary (17Oct00)
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
AudioSound::~AudioSound(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "AudioSound destructor" << endl;
  //AudioManager::stop(this);
  (*_delstate)(_state);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::length
//       Access: Public
//  Description: return the length (in seconds) of the sound
////////////////////////////////////////////////////////////////////
float AudioSound::length(void) const {
  return _sound->length();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::status
//       Access: Public
//  Description: return the current play status of this sound
////////////////////////////////////////////////////////////////////
AudioSound::SoundStatus AudioSound::status(void) const {
  AudioTraits::PlayingClass::PlayingStatus stat = _state->status();
  switch (stat) {
  case AudioTraits::PlayingClass::BAD:
    return BAD;
  case AudioTraits::PlayingClass::READY:
    return READY;
  case AudioTraits::PlayingClass::PLAYING:
    return PLAYING;
  }
  audio_cat->error() << "unknown status for sound" << endl;
  return BAD;
}
