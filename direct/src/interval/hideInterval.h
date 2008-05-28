// Filename: hideInterval.h
// Created by:  drose (27Aug02)
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

#ifndef HIDEINTERVAL_H
#define HIDEINTERVAL_H

#include "directbase.h"
#include "cInterval.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : HideInterval
// Description : An interval that calls NodePath::hide().
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT HideInterval : public CInterval {
PUBLISHED:
  HideInterval(const NodePath &node, const string &name = string());

  virtual void priv_instant();
  virtual void priv_reverse_instant();

private:
  NodePath _node;
  static int _unique_index;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "HideInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "hideInterval.I"

#endif

