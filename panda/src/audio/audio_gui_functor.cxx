// Filename: audio_gui_functor.cxx
// Created by:  cary (19Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "audio_gui_functor.h"

AudioGuiFunctor::AudioGuiFunctor(AudioSound* sound,
				 GuiBehavior::BehaviorFunctor* prev)
  : _prev(prev), _sound(sound) {}

AudioGuiFunctor::~AudioGuiFunctor(void) {
}
