// Filename: alphaTransformTransition.h
// Created by:  jason (01Aug00)
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

#ifndef ALPHA_TRANSFORM_TRANSTION_H
#define ALPHA_TRANSFORM_TRANSTION_H

#include <pandabase.h>

#include "alphaTransformProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : AlphaTransformTransition
// Description : This allows for scaling and offseting of current
//               alpha values
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AlphaTransformTransition : public OnTransition {
public:
  INLINE AlphaTransformTransition(float offset = 0, float scale = 1);

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;
  INLINE void set_scale(float scale);
  INLINE float get_scale() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  AlphaTransformProperty _state;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    register_type(_type_handle, "AlphaTransformTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


#include "alphaTransformTransition.I"

#endif
