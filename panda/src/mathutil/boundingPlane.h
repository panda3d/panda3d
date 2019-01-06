/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingPlane.h
 * @author drose
 * @date 2005-08-19
 */

#ifndef BOUNDINGPLANE_H
#define BOUNDINGPLANE_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"
#include "plane.h"

/**
 * This funny bounding volume is an infinite plane that divides space into two
 * regions: the part behind the normal, which is "inside" the bounding volume,
 * and the part in front of the normal, which is "outside" the bounding
 * volume.
 */
class EXPCL_PANDA_MATHUTIL BoundingPlane : public GeometricBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingPlane();
  INLINE_MATHUTIL BoundingPlane(const LPlane &plane);
  ALLOC_DELETED_CHAIN(BoundingPlane);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL const LPlane &get_plane() const;

  MAKE_PROPERTY(plane, get_plane);

public:
  virtual const BoundingPlane *as_bounding_plane() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual bool extend_by_plane(const BoundingPlane *plane);

  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;

private:
  LPlane _plane;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "BoundingPlane",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingSphere;
  friend class BoundingBox;
  friend class BoundingHexahedron;
};

#include "boundingPlane.I"

#endif
