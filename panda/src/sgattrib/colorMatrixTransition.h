// Filename: colorMatrixTransition.h
// Created by:  jason (01Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef COLOR_MATRIX_TRANSITION_H
#define COLOR_MATRIX_TRANSITION_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorMatrixTransition
// Description : This defines a transformation for color. I.E. to
//               make something blue, or rotate the colors, or whatever
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMatrixTransition : public LMatrix4fTransition {
public:
  INLINE ColorMatrixTransition();
  INLINE ColorMatrixTransition(const LMatrix4f &matrix);

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual MatrixTransition<LMatrix4f> *
  make_with_matrix(const LMatrix4f &matrix) const;

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_ColorMatrixTransition(const FactoryParams &params);

protected:

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LMatrix4fTransition::init_type();
    register_type(_type_handle, "ColorMatrixTransition",
                  LMatrix4fTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "colorMatrixTransition.I"

#endif
