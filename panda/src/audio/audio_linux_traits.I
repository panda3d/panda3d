// Filename: audio_linux_traits.I
// Created by:  cary (02Oct00)
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

INLINE LinuxSample::LinuxSample(byte* b, unsigned long s)
  : AudioTraits::SoundClass(), _data(b), _size(s) {
}

INLINE byte* LinuxSample::get_data(void) const {
  return _data;
}

INLINE unsigned long LinuxSample::get_size(void) const {
  return _size;
}

INLINE LinuxMusic::LinuxMusic(void) : AudioTraits::SoundClass() {
}

INLINE LinuxSamplePlaying::LinuxSamplePlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s), _buff((Buffer*)0L) {
  LinuxSample* smpl = (LinuxSample*)s;
  _buff = new Buffer(smpl->get_data(), smpl->get_size());
}

INLINE Buffer* LinuxSamplePlaying::get_data(void) {
  return _buff;
}

INLINE LinuxMusicPlaying::LinuxMusicPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
}

INLINE LinuxSamplePlayer::LinuxSamplePlayer(void)
  : AudioTraits::PlayerClass() {
}

INLINE LinuxMusicPlayer::LinuxMusicPlayer(void)
  : AudioTraits::PlayerClass() {
}
