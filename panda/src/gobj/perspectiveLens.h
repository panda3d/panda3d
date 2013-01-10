// Filename: perspectiveLens.h
// Created by:  drose (18Feb99)
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

#ifndef PERSPECTIVELENS_H
#define PERSPECTIVELENS_H

#include "pandabase.h"

#include "lens.h"


////////////////////////////////////////////////////////////////////
//       Class : PerspectiveLens
// Description : A perspective-type lens: a normal camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ PerspectiveLens : public Lens {
PUBLISHED:
  INLINE PerspectiveLens();
  INLINE PerspectiveLens(PN_stdfloat hfov, PN_stdfloat vfov);

public:
  INLINE PerspectiveLens(const PerspectiveLens &copy);
  INLINE void operator = (const PerspectiveLens &copy);

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;
  virtual bool is_perspective() const;

protected:
  virtual bool do_extrude_depth(const CData *cdata, const LPoint3 &point2d,
                                LPoint3 &point3d) const;
  virtual void do_compute_projection_mat(Lens::CData *lens_cdata);

  virtual PN_stdfloat fov_to_film(PN_stdfloat fov, PN_stdfloat focal_length, bool horiz) const;
  virtual PN_stdfloat fov_to_focal_length(PN_stdfloat fov, PN_stdfloat film_size, bool horiz) const;
  virtual PN_stdfloat film_to_fov(PN_stdfloat film_size, PN_stdfloat focal_length, bool horiz) const;

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

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
    register_type(_type_handle, "PerspectiveLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "perspectiveLens.I"

#endif
