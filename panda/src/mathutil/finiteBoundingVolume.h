/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file finiteBoundingVolume.h
 * @author drose
 * @date 1999-10-02
 */

#ifndef FINITEBOUNDINGVOLUME_H
#define FINITEBOUNDINGVOLUME_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"


/**
 * A special kind of GeometricBoundingVolume that is known to be finite.  It
 * is possible to query this kind of volume for its minimum and maximum
 * extents.
 */
class EXPCL_PANDA_MATHUTIL FiniteBoundingVolume : public GeometricBoundingVolume {
PUBLISHED:
  virtual LPoint3 get_min() const=0;
  virtual LPoint3 get_max() const=0;
  virtual PN_stdfloat get_volume() const;

  MAKE_PROPERTY(min, get_min);
  MAKE_PROPERTY(max, get_max);
  MAKE_PROPERTY(volume, get_volume);

public:
  virtual const FiniteBoundingVolume *as_finite_bounding_volume() const;

protected:
  virtual bool around_lines(const BoundingVolume **first,
                            const BoundingVolume **last);
  virtual bool around_planes(const BoundingVolume **first,
                            const BoundingVolume **last);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "FiniteBoundingVolume",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
