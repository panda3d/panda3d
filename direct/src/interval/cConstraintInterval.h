// Filename: cConstraintInterval.h
// Created by:  pratt (29Sep06)
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

#ifndef CCONSTRAINTINTERVAL_H
#define CCONSTRAINTINTERVAL_H

#include "directbase.h"
#include "cInterval.h"

////////////////////////////////////////////////////////////////////
//       Class : CConstraintInterval
// Description : The base class for a family of intervals that
//               constrain some property to a value over time.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CConstraintInterval : public CInterval {
PUBLISHED:
 bool bogus_variable;

public:
  CConstraintInterval(const string &name, double duration);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "CConstraintInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cConstraintInterval.I"

#endif

