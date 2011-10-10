// Filename: cConstrainPosHprInterval.h
// Created by:  pratt (10Mar08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CCONSTRAINPOSHPRINTERVAL_H
#define CCONSTRAINPOSHPRINTERVAL_H

#include "directbase.h"
#include "cConstraintInterval.h"
#include "nodePath.h"
#include "lvecBase3.h"
#include "lquaternion.h"

////////////////////////////////////////////////////////////////////
//       Class : CConstrainPosHprInterval
// Description : A constraint interval that will constrain the
//               position and orientation of one node to the
//               position and orientation of another.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CConstrainPosHprInterval : public CConstraintInterval {
PUBLISHED:
  CConstrainPosHprInterval(const string &name, double duration,
                           const NodePath &node, const NodePath &target,
                           bool wrt, const LVecBase3 posOffset=LVector3::zero(),
                           const LVecBase3 hprOffset=LVector3::zero());

  INLINE const NodePath &get_node() const;
  INLINE const NodePath &get_target() const;

  virtual void priv_step(double t);
  virtual void output(ostream &out) const;

private:
  NodePath _node;
  NodePath _target;
  bool _wrt;
  LVecBase3 _posOffset;
  LQuaternion _quatOffset;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CConstraintInterval::init_type();
    register_type(_type_handle, "CConstrainPosHprInterval",
                  CConstraintInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cConstrainPosHprInterval.I"

#endif

