/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file omniBoundingVolume.h
 * @author drose
 * @date 2000-06-22
 */

#ifndef OMNIBOUNDINGVOLUME_H
#define OMNIBOUNDINGVOLUME_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"

/**
 * This is a special kind of GeometricBoundingVolume that fills all of space.
 */
class EXPCL_PANDA_MATHUTIL OmniBoundingVolume : public GeometricBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL OmniBoundingVolume();

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3 get_approx_center() const;
  virtual void xform(const LMatrix4 &mat);

  virtual void output(std::ostream &out) const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;


  virtual bool extend_by_point(const LPoint3 &point);
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_box(const BoundingBox *box);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);

  virtual bool around_points(const LPoint3 *first,
                             const LPoint3 *last);
  virtual bool around_spheres(const BoundingVolume **first,
                              const BoundingVolume **last);
  virtual bool around_boxes(const BoundingVolume **first,
                            const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
                                  const BoundingVolume **last);

  virtual int contains_point(const LPoint3 &point) const;
  virtual int contains_lineseg(const LPoint3 &a, const LPoint3 &b) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "OmniBoundingVolume",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingHexahedron;
};

#include "omniBoundingVolume.I"

#endif
