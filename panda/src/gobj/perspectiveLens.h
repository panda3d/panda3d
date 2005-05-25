// Filename: perspectiveLens.h
// Created by:  drose (18Feb99)
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

#ifndef PERSPECTIVELENS_H
#define PERSPECTIVELENS_H

#include "pandabase.h"

#include "lens.h"


////////////////////////////////////////////////////////////////////
//       Class : PerspectiveLens
// Description : A perspective-type lens: a normal camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PerspectiveLens : public Lens {
PUBLISHED:
  INLINE PerspectiveLens();

public:
  INLINE PerspectiveLens(const PerspectiveLens &copy);
  INLINE void operator = (const PerspectiveLens &copy);

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;
  virtual bool is_perspective() const;

protected:
  virtual void compute_projection_mat();

  virtual float fov_to_film(float fov, float focal_length, bool horiz) const;
  virtual float fov_to_focal_length(float fov, float film_size, bool horiz) const;
  virtual float film_to_fov(float film_size, float focal_length, bool horiz) const;

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
