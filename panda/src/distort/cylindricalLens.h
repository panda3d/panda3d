// Filename: cylindricalLens.h
// Created by:  drose (12Dec01)
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

#ifndef CYLINDRICALLENS_H
#define CYLINDRICALLENS_H

#include "pandabase.h"

#include "lens.h"

////////////////////////////////////////////////////////////////////
//       Class : CylindricalLens
// Description : A cylindrical lens.  This is the kind of lens
//               generally used for extremely wide panoramic shots.
//               It behaves like a normal perspective lens in the
//               vertical direction, but it is non-linear in the
//               horizontal dimension: a point on the film corresponds
//               to a point in space in linear proportion to its angle
//               to the camera, not to its straight-line distance from
//               the center.
//
//               This allows up to 360 degree lenses in the horizontal
//               dimension, with relatively little distortion.  The
//               distortion is not very apparent between two
//               relatively nearby points on the film, but it becomes
//               increasingly evident as you compare points widely
//               spaced on the film.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX CylindricalLens : public Lens {
PUBLISHED:
  INLINE CylindricalLens();

public:
  INLINE CylindricalLens(const CylindricalLens &copy);
  INLINE void operator = (const CylindricalLens &copy);

public:
  virtual PT(Lens) make_copy() const;

protected:
  virtual bool extrude_impl(const LPoint3f &point2d,
                            LPoint3f &near_point, LPoint3f &far_point) const;
  virtual bool project_impl(const LPoint3f &point3d, LPoint3f &point2d) const;

  virtual float fov_to_film(float fov, float focal_length, bool horiz) const;
  virtual float fov_to_focal_length(float fov, float film_size, bool horiz) const;
  virtual float film_to_fov(float film_size, float focal_length, bool horiz) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Lens::init_type();
    register_type(_type_handle, "CylindricalLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "cylindricalLens.I"

#endif
