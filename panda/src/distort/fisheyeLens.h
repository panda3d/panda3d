// Filename: fisheyeLens.h
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

#ifndef FISHEYELENS_H
#define FISHEYELENS_H

#include "pandabase.h"

#include "lens.h"

////////////////////////////////////////////////////////////////////
//       Class : FisheyeLens
// Description : A fisheye lens.  This nonlinear lens introduces a
//               spherical distortion to the image, which is minimal
//               at small angles from the lens, and increases at
//               larger angles from the lens.  The field of view may
//               extend to 360 degrees.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX FisheyeLens : public Lens {
PUBLISHED:
  INLINE FisheyeLens();

public:
  INLINE FisheyeLens(const FisheyeLens &copy);
  INLINE void operator = (const FisheyeLens &copy);

public:
  virtual PT(Lens) make_copy() const;

protected:
  virtual bool extrude_impl(const LPoint3f &point2d,
                            LPoint3f &near_point, LPoint3f &far_point) const;
  virtual bool extrude_vec_impl(const LPoint3f &point2d, LVector3f &vec) const;
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
    register_type(_type_handle, "FisheyeLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fisheyeLens.I"

#endif
