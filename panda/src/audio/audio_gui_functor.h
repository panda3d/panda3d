// Filename: audio_gui_functor.h
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

#ifndef __AUDIO_GUI_FUNCTOR_H__
#define __AUDIO_GUI_FUNCTOR_H__

#include <guiBehavior.h>
#include "audio_sound.h"

class EXPCL_PANDA AudioGuiFunctor : public GuiBehavior::BehaviorFunctor {
protected:
  PT(GuiBehavior::BehaviorFunctor) _prev;
  PT(AudioSound) _sound;
public:
  virtual ~AudioGuiFunctor(void);
  virtual void doit(GuiBehavior*);
PUBLISHED:
  AudioGuiFunctor(AudioSound* = (AudioSound*)0L,
                  GuiBehavior::BehaviorFunctor* =
                  (GuiBehavior::BehaviorFunctor*)0L);
  INLINE AudioSound* get_sound(void) const { return _sound; }
  INLINE GuiBehavior::BehaviorFunctor* get_prev(void) const { return _prev; }
public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiBehavior::BehaviorFunctor::init_type();
    register_type(_type_handle, "AudioGuiFunctor",
                  GuiBehavior::BehaviorFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif /* __AUDIO_GUI_FUNCTOR_H__ */
