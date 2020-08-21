/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCoordinateSystem.h
 * @author drose
 * @date 1999-01-20
 */

#ifndef EGGCOORDINATESYSTEM_H
#define EGGCOORDINATESYSTEM_H

#include "pandabase.h"

#include "eggNode.h"
#include "eggData.h"
#include "coordinateSystem.h"


/**
 * The <CoordinateSystem> entry at the top of an egg file.  Don't confuse this
 * with the enum EggData::CoordinateSystem, which is the value contained by
 * this entry.
 */
class EXPCL_PANDA_EGG EggCoordinateSystem : public EggNode {
PUBLISHED:
  INLINE EggCoordinateSystem(CoordinateSystem value = CS_default);
  INLINE EggCoordinateSystem(const EggCoordinateSystem &copy);

  INLINE void set_value(CoordinateSystem value);
  INLINE CoordinateSystem get_value() const;

  virtual void write(std::ostream &out, int indent_level) const;

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
