// Filename: geometricBoundingVolume.h
// Created by:  drose (07Oct99)
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

#ifndef GEOMETRICBOUNDINGVOLUME_H
#define GEOMETRICBOUNDINGVOLUME_H

#include "pandabase.h"

#include "boundingVolume.h"

#include "luse.h"
#include "lmatrix.h"

////////////////////////////////////////////////////////////////////
//       Class : GeometricBoundingVolume
// Description : This is another abstract class, for a general class
//               of bounding volumes that actually enclose points in
//               3-d space, such as BSP's and bounding spheres.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeometricBoundingVolume : public BoundingVolume {
public:
  INLINE_MATHUTIL GeometricBoundingVolume();

PUBLISHED:
  INLINE_MATHUTIL bool extend_by(const GeometricBoundingVolume *vol);
  INLINE_MATHUTIL bool extend_by(const LPoint3f &point);

  // It might be nice to make these template member functions so we
  // could have true STL-style first/last iterators, but that's
  // impossible for virtual functions.
  INLINE_MATHUTIL bool around(const GeometricBoundingVolume **first,
                     const GeometricBoundingVolume **last);
  INLINE_MATHUTIL bool around(const LPoint3f *first,
                     const LPoint3f *last);

  INLINE_MATHUTIL int contains(const GeometricBoundingVolume *vol) const;
  INLINE_MATHUTIL int contains(const LPoint3f &point) const;
  INLINE_MATHUTIL int contains(const LPoint3f &a, const LPoint3f &b) const;

  virtual LPoint3f get_approx_center() const=0;
  virtual void xform(const LMatrix4f &mat)=0;

protected:
  // Some virtual functions to implement fundamental bounding
  // operations on points in 3-d space.

  virtual bool extend_by_point(const LPoint3f &point);
  virtual bool around_points(const LPoint3f *first,
                             const LPoint3f *last);
  virtual int contains_point(const LPoint3f &point) const;
  virtual int contains_lineseg(const LPoint3f &a, const LPoint3f &b) const;


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

#include "geometricBoundingVolume.I"

#endif
