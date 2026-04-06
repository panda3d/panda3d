/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pSphereLens.h
 * @author drose
 * @date 2001-12-12
 */

#ifndef PSPHERELENS_H
#define PSPHERELENS_H

#include "pandabase.h"

#include "lens.h"

/**
 * A PSphereLens is a special nonlinear lens that doesn't correspond to any
 * real physical lenses.  It's primarily useful for generating 360-degree
 * wraparound images while avoiding the distortion associated with fisheye
 * images.
 *
 * A PSphereLens is similar to a cylindrical lens, except it is also curved in
 * the vertical direction.  This allows it to extend to both poles in the
 * vertical direction.  The mapping is similar to what many modeling packages
 * call a sphere mapping: the x coordinate is proportional to azimuth, while
 * the y coordinate is proportional to altitude.
 */
class EXPCL_PANDAFX PSphereLens : public Lens {
PUBLISHED:
  INLINE PSphereLens();

public:
  INLINE PSphereLens(const PSphereLens &copy);
  INLINE void operator = (const PSphereLens &copy);

public:
  virtual PT(Lens) make_copy() const;

protected:
  virtual bool do_extrude(const Lens::CData *lens_cdata, const LPoint3 &point2d,
                          LPoint3 &near_point, LPoint3 &far_point) const;
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
    register_type(_type_handle, "PSphereLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "pSphereLens.I"

#endif
