// Filename: audio_win_traits.I
// Created by:  cary (27Sep00)
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

INLINE WinSample::
WinSample() 
    : AudioTraits::SoundClass(), _data(NULL), _len(0) {
}

INLINE DWORD WinSample::
get_length() const {
  return _len;
}

INLINE WAVEFORMATEX WinSample::
get_format() const {
  return _info;
}

INLINE WinMusic::
WinMusic() 
    : AudioTraits::SoundClass(), 
    _performance(NULL), _music(NULL),
    _buffer(NULL), _synth(NULL) {
  init();
}

INLINE IDirectMusicPerformance* WinMusic::
get_performance() const {
  return _performance;
}

INLINE IDirectMusicSegment* WinMusic::
get_music() {
  return _music;
}

INLINE LPDIRECTSOUNDBUFFER WinSamplePlaying::
get_channel() {
  return _channel;
}

INLINE IDirectMusicPerformance* WinMusicPlaying::
get_performance() const {
  WinMusic* wmusic = (WinMusic*)_sound;
  return wmusic->get_performance();
}

INLINE WinSamplePlayer::
WinSamplePlayer() 
    : AudioTraits::PlayerClass() {
}

INLINE WinMusicPlayer::
WinMusicPlayer() 
    : AudioTraits::PlayerClass() {
}
