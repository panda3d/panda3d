// Filename: colorAttrib.h
// Created by:  drose (22Feb02)
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

#ifndef COLORATTRIB_H
#define COLORATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : ColorAttrib
// Description : Indicates what color should be applied to renderable
//               geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorAttrib : public RenderAttrib {
PUBLISHED:
  enum Type {
    T_vertex, T_flat, T_off
  };

private:
  INLINE ColorAttrib(Type type, const Colorf &color);

PUBLISHED:
  static CPT(RenderAttrib) make_vertex();
  static CPT(RenderAttrib) make_flat(const Colorf &color);
  static CPT(RenderAttrib) make_off();

  INLINE Type get_color_type() const;
  INLINE const Colorf &get_color() const;

public:
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Type _type;
  Colorf _color;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ColorAttrib",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorAttrib.I"

#endif

