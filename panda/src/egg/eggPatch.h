// Filename: eggPatch.h
// Created by:  drose (27Apr12)
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

#ifndef EGGPATCH_H
#define EGGPATCH_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPatch
// Description : A single "patch", a special primitive to be rendered
//               only with a tessellation shader.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPatch : public EggPrimitive {
PUBLISHED:
  INLINE EggPatch(const string &name = "");
  INLINE EggPatch(const EggPatch &copy);
  INLINE EggPatch &operator = (const EggPatch &copy);

  virtual void write(ostream &out, int indent_level) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPatch",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggPatch.I"

#endif
