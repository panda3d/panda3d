// Filename: matrixAttribute.h
// Created by:  drose (24Mar00)
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

#ifndef MATRIXATTRIBUTE_H
#define MATRIXATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

template<class Matrix>
class MatrixTransition;

////////////////////////////////////////////////////////////////////
//       Class : MatrixAttribute
// Description :
////////////////////////////////////////////////////////////////////
template<class Matrix>
class MatrixAttribute : public NodeAttribute {
protected:
  INLINE_GRAPH MatrixAttribute();
  INLINE_GRAPH MatrixAttribute(const MatrixAttribute &copy);
  INLINE_GRAPH void operator = (const MatrixAttribute &copy);

public:
  INLINE_GRAPH void set_matrix(const Matrix &mat);
  INLINE_GRAPH const Matrix &get_matrix() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

private:
  Matrix _matrix;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    Matrix::init_type();
    register_type(_type_handle,
                  string("MatrixAttribute<") +
                  Matrix::get_class_type().get_name() + ">",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class MatrixTransition<Matrix>;
};

#include "matrixAttribute.T"

#endif
