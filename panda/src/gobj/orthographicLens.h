/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orthographicLens.h
 * @author mike
 * @date 1999-02-18
 */

#ifndef ORTHOGRAPHICLENS_H
#define ORTHOGRAPHICLENS_H

#include "pandabase.h"

#include "lens.h"


/**
 * An orthographic lens.  Although this kind of lens is linear, like a
 * PerspectiveLens, it doesn't respect field-of-view or focal length
 * parameters, and adjusting these will have no effect.  Instead, its field of
 * view is controlled by adjusting the film_size; the orthographic lens
 * represents a planar projection onto its imaginary film of the specified
 * size, hanging in space.
 */
class EXPCL_PANDA_GOBJ OrthographicLens : public Lens {
PUBLISHED:
  INLINE OrthographicLens();

public:
  INLINE OrthographicLens(const OrthographicLens &copy);
  INLINE void operator = (const OrthographicLens &copy);

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;
  virtual bool is_orthographic() const;

  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  virtual bool do_extrude_depth(const CData *cdata, const LPoint3 &point2d,
                                LPoint3 &point3d) const;
  virtual void do_compute_projection_mat(Lens::CData *lens_cdata);

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
    register_type(_type_handle, "OrthographicLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "orthographicLens.I"

#endif
