// Filename: matrixTransition.h
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

#ifndef MATRIXTRANSITION_H
#define MATRIXTRANSITION_H

#include "pandabase.h"

#include "nodeTransition.h"

#include "indent.h"

class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : MatrixTransition
// Description : This is an abstract template class that encapsulates
//               all transitions that involve some kind of matrix,
//               either a 4x4 or a 3x3, either float or double.  It
//               templates on the matrix type.
//
//               It's the base class for a number of scene graph
//               transitions like TransformTransition and
//               TexMatrixTransition.
////////////////////////////////////////////////////////////////////
template<class Matrix>
class MatrixTransition : public NodeTransition {
protected:
  INLINE_GRAPH MatrixTransition();
  INLINE_GRAPH MatrixTransition(const Matrix &matrix);
  INLINE_GRAPH MatrixTransition(const MatrixTransition &copy);
  INLINE_GRAPH void operator = (const MatrixTransition &copy);

public:
  INLINE_GRAPH void set_matrix(const Matrix &mat);
  INLINE_GRAPH const Matrix &get_matrix() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;
  virtual void internal_generate_hash(GraphHashGenerator &hash) const;

protected:
  virtual MatrixTransition<Matrix> *
  make_with_matrix(const Matrix &matrix) const=0;

private:
  Matrix _matrix;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  //Matrix has no factory method as it is a virtual template
  //class
protected:
  virtual void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    Matrix::init_type();
    register_type(_type_handle,
                  string("MatrixTransition<") +
                  Matrix::get_class_type().get_name() + ">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "matrixTransition.T"

#endif
