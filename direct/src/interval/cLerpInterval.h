// Filename: cLerpInterval.h
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

#ifndef CLERPINTERVAL_H
#define CLERPINTERVAL_H

#include "directbase.h"
#include "cInterval.h"

////////////////////////////////////////////////////////////////////
//       Class : CLerpInterval
// Description : The base class for a family of intervals that
//               linearly interpolate one or more numeric values over
//               time.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CLerpInterval : public CInterval {
PUBLISHED:
  enum BlendType {
    BT_no_blend,
    BT_ease_in,
    BT_ease_out,
    BT_ease_in_out,
    BT_invalid
  };

public:
  INLINE CLerpInterval(const string &name, double duration, 
                       BlendType blend_type);

PUBLISHED:
  INLINE BlendType get_blend_type() const;

  static BlendType string_blend_type(const string &blend_type);

protected:
  double compute_delta(double t) const;

private:
  BlendType _blend_type;

  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "CLerpInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cLerpInterval.I"

#endif

