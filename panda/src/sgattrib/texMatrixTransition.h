// Filename: texMatrixTransition.h
// Created by:  mike (19Jan99)
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

#ifndef TEXMATRIXTRANSITION_H
#define TEXMATRIXTRANSITION_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
//       Class : TexMatrixTransition
// Description : If present, this modifies the texture coordinates on
//               geometry as it is rendered.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexMatrixTransition : public LMatrix4fTransition {
public:
  INLINE TexMatrixTransition();
  INLINE TexMatrixTransition(const LMatrix4f &matrix);

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual MatrixTransition<LMatrix4f> *
  make_with_matrix(const LMatrix4f &matrix) const;

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
    register_type(_type_handle, "TexMatrixTransition",
                  LMatrix4fTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TexMatrixAttribute;
};

#include "texMatrixTransition.I"

#endif


