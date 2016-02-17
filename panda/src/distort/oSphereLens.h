/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file oSphereLens.h
 * @author drose
 * @date 2011-02-25
 */

#ifndef OSPHERELENS_H
#define OSPHERELENS_H

#include "pandabase.h"

#include "lens.h"

/**
 * A OSphereLens is a special nonlinear lens that doesn't correspond to any
 * real physical lenses.  It's primarily useful for generating 360-degree
 * wraparound images while avoiding the distortion associated with fisheye
 * images.
 *
 * A OSphereLens is similar to a Cylindrical lens and PSphereLens, except that
 * it is orthographic in the vertical direction.
 */
class EXPCL_PANDAFX OSphereLens : public Lens {
PUBLISHED:
  INLINE OSphereLens();

public:
  INLINE OSphereLens(const OSphereLens &copy);
  INLINE void operator = (const OSphereLens &copy);

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
    register_type(_type_handle, "OSphereLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "oSphereLens.I"

#endif
