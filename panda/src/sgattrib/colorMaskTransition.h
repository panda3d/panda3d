// Filename: colorMaskTransition.h
// Created by:  mike (08Feb99)
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

#ifndef COLORMASKTRANSITION_H
#define COLORMASKTRANSITION_H

#include <pandabase.h>

#include "colorMaskProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorMaskTransition
// Description : This is scene graph colorMask (which is not to be
//               confused with geometry colorMask).  By setting a
//               ColorMaskTransition on the scene graph, you can override
//               the colorMask at the object level, but not at the
//               individual primitive or vertex level.  If a scene
//               graph colorMask is set, it overrides the primitive colorMask;
//               otherwise, the primitive colorMask shows through.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMaskTransition : public OnTransition {
public:
  INLINE ColorMaskTransition();
  INLINE ColorMaskTransition(int mask);

  INLINE void set_mask(int mask);
  INLINE int get_mask() const;

  INLINE bool is_write_r() const;
  INLINE bool is_write_g() const;
  INLINE bool is_write_b() const;
  INLINE bool is_write_a() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorMaskProperty _value;

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
    register_type(_type_handle, "ColorMaskTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ColorMaskAttribute;
};

#include "colorMaskTransition.I"

#endif
