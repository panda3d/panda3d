// Filename: eggLine.h
// Created by:  drose (14Oct03)
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

#ifndef EGGLINE_H
#define EGGLINE_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggLine
// Description : A line segment, or a series of connected line
//               segments, defined by a <Line> entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggLine : public EggPrimitive {
PUBLISHED:
  INLINE EggLine(const string &name = "");
  INLINE EggLine(const EggLine &copy);
  INLINE EggLine &operator = (const EggLine &copy);

  virtual void write(ostream &out, int indent_level) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggLine",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggLine.I"

#endif
