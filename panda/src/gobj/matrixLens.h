// Filename: matrixLens.h
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

#ifndef MATRIXLENS_H
#define MATRIXLENS_H

#include "pandabase.h"

#include "lens.h"


////////////////////////////////////////////////////////////////////
//       Class : MatrixLens
// Description : A completely generic linear lens.  This is provided
//               for the benefit of low-level code that wants to
//               specify a perspective or orthographic frustum via an
//               explicit projection matrix, but not mess around with
//               fov's or focal lengths or any of that nonsense.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MatrixLens : public Lens {
PUBLISHED:
  INLINE MatrixLens();

public:
  INLINE MatrixLens(const MatrixLens &copy);
  INLINE void operator = (const MatrixLens &copy);

PUBLISHED:
  INLINE void set_user_mat(const LMatrix4f &user_mat);
  INLINE const LMatrix4f &get_user_mat() const;

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual void compute_projection_mat();

private:
  LMatrix4f _user_mat;

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
    register_type(_type_handle, "MatrixLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "matrixLens.I"

#endif
