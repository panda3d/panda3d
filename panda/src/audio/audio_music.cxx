// Filename: audio_music.cxx
// Created by:  cary (26Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_music.h"
#include "config_audio.h"

TypeHandle AudioMusic::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::destructor
//       Access: Public
//  Description: deletes the music data and then lets the system
//               destroy this structure
////////////////////////////////////////////////////////////////////
AudioMusic::~AudioMusic(void) {
  (*_destroy)(_music);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioMusic::status
//       Access: Public
//  Description: return the current play status of this music
////////////////////////////////////////////////////////////////////
AudioMusic::MusicStatus AudioMusic::status(void) {
  //  AudioTraits::PlayingClass::PlayingStatus stat = _state->status();
  AudioTraits::MusicClass::MusicStatus stat = _music->status();
  switch (stat) {
    //  case AudioTraits::PlayingClass::BAD:
  case AudioTraits::MusicClass::BAD:
    return BAD;
    //  case AudioTraits::PlayingClass::READY:
  case AudioTraits::MusicClass::READY:
    return READY;
    //  case AudioTraits::PlayingClass::PLAYING:
  case AudioTraits::MusicClass::PLAYING:
    return PLAYING;
  }
  audio_cat->error() << "unknown status for music" << endl;
  return BAD;
}
