// Filename: transformTransition.h
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

#ifndef TRANSFORMTRANSITION_H
#define TRANSFORMTRANSITION_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
//       Class : TransformTransition
// Description : This defines a new coordinate system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformTransition : public LMatrix4fTransition {
public:
  INLINE TransformTransition();
  INLINE TransformTransition(const LMatrix4f &matrix);

  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual MatrixTransition<LMatrix4f> *
  make_with_matrix(const LMatrix4f &matrix) const;

private:
  static PT(NodeTransition) _initial;

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_TransformTransition(const FactoryParams &params);

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
    register_type(_type_handle, "TransformTransition",
                  LMatrix4fTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "transformTransition.I"

#endif


