// Filename: waitInterval.h
// Created by:  drose (12Sep02)
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

#ifndef WAITINTERVAL_H
#define WAITINTERVAL_H

#include "directbase.h"
#include "cInterval.h"

////////////////////////////////////////////////////////////////////
//       Class : WaitInterval
// Description : This interval does absolutely nothing, and is mainly
//               useful for marking time between other intervals
//               within a sequence.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT WaitInterval : public CInterval {
PUBLISHED:
  INLINE WaitInterval(double duration);

  virtual void priv_step(double t);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "WaitInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "waitInterval.I"

#endif

