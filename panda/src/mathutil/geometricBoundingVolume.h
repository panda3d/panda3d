/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geometricBoundingVolume.h
 * @author drose
 * @date 1999-10-07
 */

#ifndef GEOMETRICBOUNDINGVOLUME_H
#define GEOMETRICBOUNDINGVOLUME_H

#include "pandabase.h"

#include "boundingVolume.h"

#include "luse.h"
#include "lmatrix.h"

/**
 * This is another abstract class, for a general class of bounding volumes
 * that actually enclose points in 3-d space, such as BSP's and bounding
 * spheres.
 */
class EXPCL_PANDA_MATHUTIL GeometricBoundingVolume : public BoundingVolume {
public:
  INLINE_MATHUTIL GeometricBoundingVolume();

PUBLISHED:
  INLINE_MATHUTIL bool extend_by(const GeometricBoundingVolume *vol);
  INLINE_MATHUTIL bool extend_by(const LPoint3 &point);

public:
  // It might be nice to make these template member functions so we could have
  // true STL-style firstlast iterators, but that's impossible for virtual
  // functions.
  INLINE_MATHUTIL bool around(const GeometricBoundingVolume **first,
                              const GeometricBoundingVolume **last);
  INLINE_MATHUTIL bool around(const LPoint3 *first, const LPoint3 *last);

PUBLISHED:
  INLINE_MATHUTIL int contains(const GeometricBoundingVolume *vol) const;
  INLINE_MATHUTIL int contains(const LPoint3 &point) const;
  INLINE_MATHUTIL int contains(const LPoint3 &a, const LPoint3 &b) const;

  virtual LPoint3 get_approx_center() const=0;
  virtual void xform(const LMatrix4 &mat)=0;

public:
  virtual GeometricBoundingVolume *as_geometric_bounding_volume() final;
  virtual const GeometricBoundingVolume *as_geometric_bounding_volume() const final;

protected:
  // Some virtual functions to implement fundamental bounding operations on
  // points in 3-d space.

  virtual bool extend_by_point(const LPoint3 &point);
  virtual bool around_points(const LPoint3 *first,
                             const LPoint3 *last);
  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BoundingVolume::init_type();
    register_type(_type_handle, "GeometricBoundingVolume",
                  BoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<GeometricBoundingVolume>::update_type(To *ptr) {}

#include "geometricBoundingVolume.I"

#endif
