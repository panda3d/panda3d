// Filename: eggPoint.h
// Created by:  drose (15Dec99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGPOINT_H
#define EGGPOINT_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggPoint
// Description : A single point, or a collection of points as defined
//               by a single <PointLight> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPoint : public EggPrimitive {
PUBLISHED:
  INLINE EggPoint(const string &name = "");
  INLINE EggPoint(const EggPoint &copy);
  INLINE EggPoint &operator = (const EggPoint &copy);

  virtual bool cleanup();

  virtual void write(ostream &out, int indent_level) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPoint",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggPoint.I"

#endif
