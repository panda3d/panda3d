// Filename: stencilTransition.h
// Created by:  mike (18Jan99)
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

#ifndef STENCILTRANSITION_H
#define STENCILTRANSITION_H

#include <pandabase.h>

#include "stencilProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : StencilTransition
// Description : This is scene graph stencil (which is not to be
//               confused with geometry stencil).  By setting a
//               StencilTransition on the scene graph, you can override
//               the stencil at the object level, but not at the
//               individual primitive or vertex level.  If a scene
//               graph stencil is set, it overrides the primitive stencil;
//               otherwise, the primitive stencil shows through.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StencilTransition : public OnTransition {
public:
  INLINE StencilTransition(StencilProperty::Mode mode = StencilProperty::M_none,
                           StencilProperty::Action action = StencilProperty::A_keep);

  INLINE void set_mode(StencilProperty::Mode mode);
  INLINE StencilProperty::Mode get_mode() const;

  INLINE void set_action(StencilProperty::Action action);
  INLINE StencilProperty::Action get_action() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  StencilProperty _value;

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
    register_type(_type_handle, "StencilTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "stencilTransition.I"

#endif
