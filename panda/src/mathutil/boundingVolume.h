// Filename: boundingVolume.h
// Created by:  drose (01Oct99)
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

#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include "pandabase.h"

#include "typedObject.h"
#include "typedReferenceCount.h"

class BoundingSphere;
class BoundingHexahedron;
class BoundingLine;


///////////////////////////////////////////////////////////////////
//       Class : BoundingVolume
// Description : This is an abstract class for any volume in any sense
//               which can be said to define the locality of reference
//               of a node in a graph, along with all of its
//               descendants.  It is not necessarily a geometric
//               volume (although see GeometricBoundingVolume); this
//               is simply an abstract interface for bounds of any
//               sort.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundingVolume : public TypedReferenceCount {
PUBLISHED:
  INLINE_MATHUTIL BoundingVolume();
  virtual BoundingVolume *make_copy() const=0;

  INLINE_MATHUTIL bool is_empty() const;
  INLINE_MATHUTIL bool is_infinite() const;

  INLINE_MATHUTIL void set_infinite();

  INLINE_MATHUTIL bool extend_by(const BoundingVolume *vol);

  // It might be nice to make these template member functions so we
  // could have true STL-style first/last iterators, but that's
  // impossible for virtual functions.
  bool around(const BoundingVolume **first,
              const BoundingVolume **last);

  // The contains() functions return the union of one or more of these
  // bits.
  enum IntersectionFlags {
    // If no bits are set, it is known that there is no intersection.
    IF_no_intersection = 0,

    // IF_possible is set if there might be an intersection.
    IF_possible        = 0x01,

    // IF_some is set if there is definitely an intersection.  In this
    // case, IF_possible will also be set.
    IF_some            = 0x02,

    // IF_all is set if the other bounding volume is known to be
    // completely within this bounding volume: that is, there is no
    // part of the other bounding volume that does not intersect this
    // one.  It does *not* indicate the inverse; it is possible that
    // some part of this bounding volume does not intersect the other.

    // Also, the converse is not implied: if IF_all is not set, you
    // simply don't know whether the other volume is completely
    // contained within this one or not.

    // When IF_all is set, both IF_possible and IF_some will also be
    // set.
    IF_all             = 0x04,

    // IF_dont_understand is set if the particular volume/volume
    // intersection test has not been implemented.
    IF_dont_understand = 0x08
  };

  INLINE_MATHUTIL int contains(const BoundingVolume *vol) const;

  virtual void output(ostream &out) const=0;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  enum Flags {
    F_empty        = 0x01,
    F_infinite     = 0x02
  };
  int _flags;

protected:
  // The following functions support double-dispatch of virtual
  // methods, so we can easily extend_by() various types of bounding
  // volumes.

  // These functions are the first dispatch point.
  virtual bool extend_other(BoundingVolume *other) const=0;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const=0;
  virtual int contains_other(const BoundingVolume *other) const=0;

  // These functions are the second dispatch point.  They actually do
  // the work.
  virtual bool extend_by_sphere(const BoundingSphere *sphere);
  virtual bool extend_by_hexahedron(const BoundingHexahedron *hexahedron);
  virtual bool extend_by_line(const BoundingLine *line);

  virtual bool around_spheres(const BoundingVolume **first,
                              const BoundingVolume **last);
  virtual bool around_hexahedrons(const BoundingVolume **first,
                                  const BoundingVolume **last);
  virtual bool around_lines(const BoundingVolume **first,
                            const BoundingVolume **last);

  virtual int contains_sphere(const BoundingSphere *sphere) const;
  virtual int contains_hexahedron(const BoundingHexahedron *hexahedron) const;
  virtual int contains_line(const BoundingLine *line) const;


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
  friend class BoundingHexahedron;
  friend class BoundingLine;
};

INLINE_MATHUTIL ostream &operator << (ostream &out, const BoundingVolume &bound);

#include "boundingVolume.I"

#endif
