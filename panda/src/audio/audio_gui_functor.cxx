// Filename: audio_gui_functor.cxx
// Created by:  cary (19Apr01)
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

#include "audio_gui_functor.h"

TypeHandle AudioGuiFunctor::_type_handle;

AudioGuiFunctor::AudioGuiFunctor(AudioSound* sound,
                                 GuiBehavior::BehaviorFunctor* prev)
  : GuiBehavior::BehaviorFunctor(), _prev(prev), _sound(sound) {}

AudioGuiFunctor::~AudioGuiFunctor(void) {
  _prev.clear();
}

#include "audio_manager.h"

void AudioGuiFunctor::doit(GuiBehavior* b) {
  if (_sound != (AudioSound*)0L)
    AudioManager::play(_sound);
  if (_prev != (GuiBehavior::BehaviorFunctor*)0L)
    _prev->doit(b);
}
