// Filename: boundingPlane.h
// Created by:  drose (19Aug05)
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

#ifndef BOUNDINGPLANE_H
#define BOUNDINGPLANE_H

#include "pandabase.h"

#include "geometricBoundingVolume.h"
#include "plane.h"

////////////////////////////////////////////////////////////////////
//       Class : BoundingPlane
// Description : This funny bounding volume is an infinite plane that
//               divides space into two regions: the part behind the
//               normal, which is "inside" the bounding volume, and
//               the part in front of the normal, which is "outside"
//               the bounding volume.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundingPlane : public GeometricBoundingVolume {
PUBLISHED:
  INLINE_MATHUTIL BoundingPlane();
  INLINE_MATHUTIL BoundingPlane(const Planef &plane);

public:
  virtual BoundingVolume *make_copy() const;

  virtual LPoint3f get_approx_center() const;
  virtual void xform(const LMatrix4f &mat);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE_MATHUTIL const Planef &get_plane() const;

protected:
  virtual bool extend_other(BoundingVolume *other) const;
  virtual bool around_other(BoundingVolume *other,
                            const BoundingVolume **first,
                            const BoundingVolume **last) const;
  virtual int contains_other(const BoundingVolume *other) const;

  virtual bool extend_by_plane(const BoundingPlane *plane);

  virtual int contains_sphere(const BoundingSphere *sphere) const;

private:
  Planef _plane;


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
};

#include "boundingPlane.I"

#endif
