/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cylindricalLens.h
 * @author drose
 * @date 2001-12-12
 */

#ifndef CYLINDRICALLENS_H
#define CYLINDRICALLENS_H

#include "pandabase.h"

#include "lens.h"

/**
 * A cylindrical lens.  This is the kind of lens generally used for extremely
 * wide panoramic shots.  It behaves like a normal perspective lens in the
 * vertical direction, but it is non-linear in the horizontal dimension: a
 * point on the film corresponds to a point in space in linear proportion to
 * its angle to the camera, not to its straight-line distance from the center.
 *
 * This allows up to 360 degree lenses in the horizontal dimension, with
 * relatively little distortion.  The distortion is not very apparent between
 * two relatively nearby points on the film, but it becomes increasingly
 * evident as you compare points widely spaced on the film.
 */
class EXPCL_PANDAFX CylindricalLens : public Lens {
PUBLISHED:
  INLINE CylindricalLens();

public:
  INLINE CylindricalLens(const CylindricalLens &copy);
  INLINE void operator = (const CylindricalLens &copy);

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
    register_type(_type_handle, "CylindricalLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "cylindricalLens.I"

#endif
