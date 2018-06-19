/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUserData.h
 * @author drose
 * @date 2003-06-03
 */

#ifndef EGGUSERDATA_H
#define EGGUSERDATA_H

#include "pandabase.h"

#include "typedReferenceCount.h"

/**
 * This is a base class for a user-defined data type to extend egg structures
 * in processing code.  The user of the egg library may derive from
 * EggUserData to associate any arbitrary data with various egg objects.
 *
 * However, this data will not be written out to the disk when the egg file is
 * written; it is an in-memory object only.
 */
class EXPCL_PANDA_EGG EggUserData : public TypedReferenceCount {
PUBLISHED:
  INLINE EggUserData();
  INLINE EggUserData(const EggUserData &copy);
  INLINE EggUserData &operator = (const EggUserData &copy);

  virtual ~EggUserData();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EggUserData",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggUserData.I"

#endif
