// Filename: nullAudioManager.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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

#include "nullAudioManager.h"

NullAudioManager::NullAudioManager() {
  audio_info("NullAudioManager");
}

NullAudioManager::~NullAudioManager() {
  // intentionally blank.
}
  
AudioSound* NullAudioManager::get_sound(const string&) {
  return new NullAudioSound();
}

void NullAudioManager::drop_sound(const string&) {
  // intentionally blank.
}

void NullAudioManager::set_volume(float) {
  // intentionally blank.
}

float NullAudioManager::get_volume() {
  return 0;
}
  
void NullAudioManager::set_active(bool) {
  // intentionally blank.
}

bool NullAudioManager::get_active() {
  return 0;
}
