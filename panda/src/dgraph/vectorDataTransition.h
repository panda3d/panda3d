// Filename: vectorDataTransition.h
// Created by:  drose (25Jan99)
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

#ifndef VECTORDATATRANSITION_H
#define VECTORDATATRANSITION_H

#include <pandabase.h>

#include <nodeTransition.h>
#include <luse.h>
#include <indent.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : VectorDataTransition
// Description : A VectorDataAttribute is a special data graph
//               attribute that is used to pass around a complex
//               number like a vector or a matrix.  The Transition
//               isn't often used, but it may contain a matrix that
//               modifies the vector in the attribute.  (Note that a
//               LMatrix4DataAttribute actually derives from
//               VectorDataAttribute, even though its base type is a
//               matrix, not a vector.)
////////////////////////////////////////////////////////////////////
template<class VecType, class MatType>
class VectorDataTransition : public NodeTransition {
public:
  INLINE VectorDataTransition();
  INLINE VectorDataTransition(const MatType &matrix);
  INLINE VectorDataTransition(const VectorDataTransition<VecType, MatType> &copy);
  INLINE void operator = (const VectorDataTransition<VecType, MatType> &copy);

public:

  INLINE bool is_identity() const;

  INLINE void set_matrix(const MatType &matrix);
  INLINE const MatType &get_matrix() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

private:
  MatType _matrix;

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
    do_init_type(VecType);
    do_init_type(MatType);
    register_type(_type_handle,
                  string("VectorDataTransition<") +
                  get_type_handle(VecType).get_name() + "," +
                  get_type_handle(MatType).get_name() + ">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vectorDataTransition.I"

#endif
