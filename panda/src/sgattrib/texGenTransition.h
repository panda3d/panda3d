// Filename: texGenTransition.h
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

#ifndef TEXGENTRANSITION_H
#define TEXGENTRANSITION_H

#include <pandabase.h>

#include "texGenProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : TexGenTransition
// Description : This controls generation of texture coordinates in
//               the graphics HW, for instance for projected
//               textures.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenTransition : public OnTransition {
public:
  INLINE TexGenTransition();
  INLINE static TexGenTransition texture_projector();
  INLINE static TexGenTransition sphere_map();

  INLINE void set_none();
  INLINE void set_texture_projector();
  INLINE void set_sphere_map();

  INLINE TexGenProperty::Mode get_mode() const;
  INLINE LMatrix4f get_plane() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TexGenProperty _value;

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
    register_type(_type_handle, "TexGenTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TexGenAttribute;
};

#include "texGenTransition.I"

#endif
