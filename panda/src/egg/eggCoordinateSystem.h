// Filename: eggCoordinateSystem.h
// Created by:  drose (20Jan99)
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

#ifndef EGGCOORDINATESYSTEM_H
#define EGGCOORDINATESYSTEM_H

#include "pandabase.h"

#include "eggNode.h"
#include "eggData.h"
#include "coordinateSystem.h"


////////////////////////////////////////////////////////////////////
//       Class : EggCoordinateSystem
// Description : The <CoordinateSystem> entry at the top of an egg
//               file.  Don't confuse this with the enum
//               EggData::CoordinateSystem, which is the value
//               contained by this entry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggCoordinateSystem : public EggNode {
public:
  INLINE EggCoordinateSystem(CoordinateSystem value = CS_default);
  INLINE EggCoordinateSystem(const EggCoordinateSystem &copy);

  INLINE void set_value(CoordinateSystem value);
  INLINE CoordinateSystem get_value() const;

  virtual void write(ostream &out, int indent_level) const;

private:
  CoordinateSystem _value;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggCoordinateSystem",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggCoordinateSystem.I"

#endif
