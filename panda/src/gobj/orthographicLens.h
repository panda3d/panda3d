// Filename: orthographicLens.h
// Created by:  mike (18Feb99)
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

#ifndef ORTHOGRAPHICLENS_H
#define ORTHOGRAPHICLENS_H

#include "pandabase.h"

#include "lens.h"


////////////////////////////////////////////////////////////////////
//       Class : OrthographicLens
// Description : An orthographic lens.  Although this kind of lens is
//               linear, like a PerspectiveLens, it doesn't respect
//               field-of-view or focal length parameters, and
//               adjusting these will have no effect.  Instead, its
//               field of view is controlled by adjusting the
//               film_size; the orthographic lens represents a planar
//               projection onto its imaginary film of the specified
//               size, hanging in space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OrthographicLens : public Lens {
PUBLISHED:
  INLINE OrthographicLens();

public:
  INLINE OrthographicLens(const OrthographicLens &copy);
  INLINE void operator = (const OrthographicLens &copy);

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual void compute_projection_mat();

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
