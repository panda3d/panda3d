// Filename: cLerpAnimEffectInterval.h
// Created by:  drose (27Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CLERPANIMEFFECTINTERVAL_H
#define CLERPANIMEFFECTINTERVAL_H

#include "directbase.h"
#include "cLerpInterval.h"
#include "animControl.h"
#include "pointerTo.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : CLerpAnimEffectInterval
// Description : This interval lerps between different amounts of
//               control effects for various AnimControls that might
//               be playing on an actor.  It's used to change the
//               blending amount between multiple animations.
//
//               The idea is to start all the animations playing
//               first, then use a CLerpAnimEffectInterval to adjust
//               the degree to which each animation affects the actor.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CLerpAnimEffectInterval : public CLerpInterval {
PUBLISHED:
  INLINE CLerpAnimEffectInterval(const string &name, double duration, 
                                 BlendType blend_type);

  INLINE void add_control(AnimControl *control, const string &name,
                          float begin_effect, float end_effect);

  virtual void priv_step(double t);

  virtual void output(ostream &out) const;

private:
  class ControlDef {
  public:
    INLINE ControlDef(AnimControl *control, const string &name,
                      float begin_effect, float end_effect);
    PT(AnimControl) _control;
    string _name;
    float _begin_effect;
    float _end_effect;
  };
    
  typedef pvector<ControlDef> Controls;
  Controls _controls;
  

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CLerpInterval::init_type();
    register_type(_type_handle, "CLerpAnimEffectInterval",
                  CLerpInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cLerpAnimEffectInterval.I"

#endif

