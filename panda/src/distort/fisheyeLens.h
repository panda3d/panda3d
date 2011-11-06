// Filename: fisheyeLens.h
// Created by:  drose (12Dec01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
  virtual bool do_extrude(const Lens::CData *lens_cdata, const LPoint3 &point2d, 
                          LPoint3 &near_point, LPoint3 &far_point) const;
  virtual bool do_extrude_vec(const Lens::CData *lens_cdata, const LPoint3 &point2d, 
                              LVector3 &vec) const;
  virtual bool do_project(const Lens::CData *lens_cdata, 
                          const LPoint3 &point3d, LPoint3 &point2d) const;

  virtual PN_stdfloat fov_to_film(PN_stdfloat fov, PN_stdfloat focal_length, bool horiz) const;
  virtual PN_stdfloat fov_to_focal_length(PN_stdfloat fov, PN_stdfloat film_size, bool horiz) const;
  virtual PN_stdfloat film_to_fov(PN_stdfloat film_size, PN_stdfloat focal_length, bool horiz) const;

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
