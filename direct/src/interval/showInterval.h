// Filename: showInterval.h
// Created by:  drose (27Aug02)
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

#ifndef SHOWINTERVAL_H
#define SHOWINTERVAL_H

#include "directbase.h"
#include "cInterval.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : ShowInterval
// Description : An interval that calls NodePath::show().
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT ShowInterval : public CInterval {
PUBLISHED:
  ShowInterval(const NodePath &node, const string &name = string());

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
    register_type(_type_handle, "ShowInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "showInterval.I"

#endif

