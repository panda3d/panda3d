// Filename: audio_null_traits.I
// Created by:  cary (25Sep00)
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

INLINE NullSound::NullSound(void) : AudioTraits::SoundClass() {
}

INLINE NullPlaying::NullPlaying(AudioTraits::SoundClass* sound)
  : AudioTraits::PlayingClass(sound) {
}

INLINE NullPlayer::NullPlayer(void) : AudioTraits::PlayerClass() {
}
