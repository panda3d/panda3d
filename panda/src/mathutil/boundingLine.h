// Filename: boundingLine.h
// Created by:  drose (04Jul00)
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

#ifndef BOUNDINGLINE_H
#define BOUNDINGLINE_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"

///////////////////////////////////////////////////////////////////
//       Class : BoundingLine
// Description : This funny bounding volume is an infinite line with
//               no thickness and extending to infinity in both
//               directions.
//
//               Note that it *always* extends in both directions,
//               despite the fact that you specify two points to the
//               constructor.  These are not endpoints, they are two
//               arbitrary points on the line.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundingLine : public GeometricBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingLine();
  INLINE_MATHUTIL BoundingLine(const LPoint3f &a, const LPoint3f &b);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL const LPoint3f &get_point_a() const;
  INLINE_MATHUTIL LPoint3f get_point_b() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual bool extend_by_line(const BoundingLine *line);

  virtual int contains_sphere(const BoundingSphere *sphere) const;

  float sqr_dist_to_line(const LPoint3f &point) const;

private:
  LPoint3f _origin;
  LVector3f _vector;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "BoundingLine",
                  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BoundingSphere;
};

#include "boundingLine.I"

#endif
