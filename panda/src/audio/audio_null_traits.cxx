// Filename: audio_null_traits.cxx
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

#include "audio_null_traits.h"

#ifdef AUDIO_USE_NULL

#include "audio_manager.h"
#include "config_audio.h"

static bool have_initialized = false;

static void update_null() {
  if (audio_cat.is_debug())
    audio_cat->debug() << "Update in Null audio driver" << endl;
}

static void initialize() {
  if (have_initialized)
    return;
  AudioManager::set_update_func(update_null);
  have_initialized = true;
}

NullSound::~NullSound() {
}

float NullSound::length() const {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in sample length in Null audio driver" << endl;
  return -1.;
}

AudioTraits::PlayingClass* NullSound::get_state() const {
  return new NullPlaying((NullSound*)this);
}

AudioTraits::PlayerClass* NullSound::get_player() const {
  return new NullPlayer();
}


AudioTraits::DeletePlayingFunc* NullSound::get_delstate() const {
  return NullPlaying::destroy;
}

NullPlaying::~NullPlaying() {
}

AudioTraits::PlayingClass::PlayingStatus NullPlaying::status() {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in playing status in Null audio driver" << endl;
  return BAD;
}

void NullPlaying::destroy(AudioTraits::PlayingClass* play) {
  delete play;
}

NullPlayer::~NullPlayer() {
}

void NullPlayer::play_sound(AudioTraits::SoundClass*,
                AudioTraits::PlayingClass*, float) {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in play sound in Null audio driver" << endl;
}

void NullPlayer::stop_sound(AudioTraits::SoundClass*,
                AudioTraits::PlayingClass*) {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in stop sound in Null audio driver" << endl;
}

void NullPlayer::set_volume(AudioTraits::PlayingClass* p, float v) {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in set volume in Null audio driver" << endl;
  p->set_volume(v);
}

bool NullPlayer::adjust_volume(AudioTraits::PlayingClass*) {
  if (audio_cat.is_debug())
    audio_cat->debug() << "in adjust volume in Null audio driver" << endl;
  return false;
}

#endif /* AUDIO_USE_NULL */
