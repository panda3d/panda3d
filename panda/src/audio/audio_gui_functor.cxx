// Filename: audio_gui_functor.cxx
// Created by:  cary (19Apr01)
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
