// Filename: audio_trait.cxx
// Created by:  cary (23Sep00)
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

#include "audio_trait.h"
#include "config_audio.h"

AudioTraits::SoundClass::~SoundClass() {
}

float AudioTraits::SoundClass::length() const {
  audio_cat->error() << "In abstract SoundClass::length!" << endl;
  return -1.;
}

AudioTraits::PlayingClass* AudioTraits::SoundClass::get_state() const {
  audio_cat->error() << "In abstract SoundClass::get_state!" << endl;
  return (AudioTraits::PlayingClass*)0L;
}

AudioTraits::PlayerClass* AudioTraits::SoundClass::get_player() const {
  audio_cat->error() << "In abstract SoundClass::get_player!" << endl;
  return (AudioTraits::PlayerClass*)0L;
}

AudioTraits::DeletePlayingFunc*
AudioTraits::SoundClass::get_delstate() const {
  audio_cat->error() << "In abstract SoundClass::get_delstate!" << endl;
  return (AudioTraits::DeletePlayingFunc*)0L;
}

AudioTraits::PlayingClass::~PlayingClass() {
}

AudioTraits::PlayingClass::PlayingStatus
AudioTraits::PlayingClass::status() {
  audio_cat->error() << "In abstract PlayingClass::status!" << endl;
  return BAD;
}

AudioTraits::PlayerClass::~PlayerClass() {
}

void AudioTraits::PlayerClass::play_sound(AudioTraits::SoundClass*,
                                          AudioTraits::PlayingClass*, float) {
  audio_cat->error() << "In abstract PlayerClass::play_sound!" << endl;
}

void AudioTraits::PlayerClass::stop_sound(AudioTraits::SoundClass*,
                                          AudioTraits::PlayingClass*) {
  audio_cat->error() << "In abstract PlayerClass::stop_sound!" << endl;
}

void AudioTraits::PlayerClass::set_volume(AudioTraits::PlayingClass*, float) {
  audio_cat->error() << "In abstract PlayerClass::set_volume!" << endl;
}

bool AudioTraits::PlayerClass::adjust_volume(AudioTraits::PlayingClass*) {
  audio_cat->error() << "In abstract PlayerClass::adjust_volume!" << endl;
  return false;
}
