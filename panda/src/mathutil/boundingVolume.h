/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingVolume.h
 * @author drose
 * @date 1999-10-01
 */

#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include "pandabase.h"

#include "typedObject.h"
#include "typedReferenceCount.h"
#include "deletedChain.h"

class GeometricBoundingVolume;
class FiniteBoundingVolume;
class BoundingSphere;
class BoundingBox;
class BoundingHexahedron;
class BoundingLine;
class BoundingPlane;
class UnionBoundingVolume;
class IntersectionBoundingVolume;


/**
 * This is an abstract class for any volume in any sense which can be said to
 * define the locality of reference of a node in a graph, along with all of
 * its descendants.  It is not necessarily a geometric volume (although see
 * GeometricBoundingVolume); this is simply an abstract interface for bounds
 * of any sort.
 */
class EXPCL_PANDA_MATHUTIL BoundingVolume : public TypedReferenceCount {
public:
  INLINE_MATHUTIL BoundingVolume();

PUBLISHED:
  virtual BoundingVolume *make_copy() const=0;

  INLINE_MATHUTIL bool is_empty() const;
  INLINE_MATHUTIL bool is_infinite() const;

  INLINE_MATHUTIL void set_infinite();

  INLINE_MATHUTIL bool extend_by(const BoundingVolume *vol);

public:
  // It might be nice to make these template member functions so we could have
  // true STL-style firstlast iterators, but that's impossible for virtual
  // functions.
  bool around(const BoundingVolume **first,
              const BoundingVolume **last);

PUBLISHED:
  // The contains() functions return the union of one or more of these bits.
  enum IntersectionFlags {
    // If no bits are set, it is known that there is no intersection.
    IF_no_intersection = 0,

    // IF_possible is set if there might be an intersection.
    IF_possible        = 0x01,

    // IF_some is set if there is definitely an intersection.  In this case,
    // IF_possible will also be set.
    IF_some            = 0x02,

    // IF_all is set if the other bounding volume is known to be completely
    // within this bounding volume: that is, there is no part of the other
    // bounding volume that does not intersect this one.  It does *not*
    // indicate the inverse; it is possible that some part of this bounding
    // volume does not intersect the other.

    // Also, the converse is not implied: if IF_all is not set, you simply
    // don't know whether the other volume is completely contained within this
    // one or not.

    // When IF_all is set, both IF_possible and IF_some will also be set.
    IF_all             = 0x04,

    // IF_dont_understand is set if the particular volumevolume intersection
    // test has not been implemented.
    IF_dont_understand = 0x08
  };

  INLINE_MATHUTIL int contains(const BoundingVolume *vol) const;

  virtual void output(std::ostream &out) const=0;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  // This enum is used to control the automatic generation of bounding
  // volumes.
  enum BoundsType {
    BT_default,
    BT_best,
    BT_sphere,
    BT_box,
    BT_fastest,
  };

public:
  virtual GeometricBoundingVolume *as_geometric_bounding_volume();
  virtual const GeometricBoundingVolume *as_geometric_bounding_volume() const;
  virtual const FiniteBoundingVolume *as_finite_bounding_volume() const;
  virtual const BoundingSphere *as_bounding_sphere() const;
  virtual const BoundingBox *as_bounding_box() const;
  virtual const BoundingHexahedron *as_bounding_hexahedron() const;
  virtual const BoundingLine *as_bounding_line() const;
  virtual const BoundingPlane *as_bounding_plane() const;

  static BoundsType string_bounds_type(const std::string &str);

protected:
  enum Flags {
    F_empty        = 0x01,
    F_infinite     = 0x02
  };
  int _flags;

protected:
  // The following functions support double-dispatch of virtual methods, so we
  // can easily extend_by() various types of bounding volumes.

  // These functions are the first dispatch point.
  virtual bool extend_other(BoundingVolume *other) const=0;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const=0;
  virtual int contains_other(const BoundingVolume *other) const=0;

  // These functions are the second dispatch point.  They actually do the
  // work.
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_box(const BoundingBox *box);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);
  virtual bool extend_by_line(const BoundingLine *line);
  virtual bool extend_by_plane(const BoundingPlane *plane);
  virtual bool extend_by_union(const UnionBoundingVolume *unionv);
  virtual bool extend_by_intersection(const IntersectionBoundingVolume *intersection);
  virtual bool extend_by_finite(const FiniteBoundingVolume *volume);
  virtual bool extend_by_geometric(const GeometricBoundingVolume *volume);

  virtual bool around_spheres(const BoundingVolume **first,
                              const BoundingVolume **last);
  virtual bool around_boxes(const BoundingVolume **first,
                            const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
                                  const BoundingVolume **last);
  virtual bool around_lines(const BoundingVolume **first,
                            const BoundingVolume **last);
  virtual bool around_planes(const BoundingVolume **first,
                             const BoundingVolume **last);
  virtual bool around_unions(const BoundingVolume **first,
                             const BoundingVolume **last);
  virtual bool around_intersections(const BoundingVolume **first,
                                    const BoundingVolume **last);
  virtual bool around_finite(const BoundingVolume **first,
                             const BoundingVolume **last);
  virtual bool around_geometric(const BoundingVolume **first,
                                const BoundingVolume **last);

  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_box(const BoundingBox *box) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_line(const BoundingLine *line) const;
  virtual int contains_plane(const BoundingPlane *plane) const;
  virtual int contains_union(const UnionBoundingVolume *unionv) const;
  virtual int contains_intersection(const IntersectionBoundingVolume *intersection) const;
  virtual int contains_finite(const FiniteBoundingVolume *volume) const;
  virtual int contains_geometric(const GeometricBoundingVolume *volume) const;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BoundingVolume",
                  TypedReferenceCount::get_class_type());
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
  friend class BoundingLine;
  friend class BoundingPlane;
  friend class UnionBoundingVolume;
  friend class IntersectionBoundingVolume;
};

INLINE_MATHUTIL std::ostream &operator << (std::ostream &out, const BoundingVolume &bound);

#include "boundingVolume.I"

EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, BoundingVolume::BoundsType type);
EXPCL_PANDA_MATHUTIL std::istream &operator >> (std::istream &in, BoundingVolume::BoundsType &type);

#endif
