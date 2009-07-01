// Filename: p3dFloatVariant.h
// Created by:  drose (30Jun09)
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

#ifndef P3DFLOATVARIANT_H
#define P3DFLOATVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DFloatVariant
// Description : A variant type that contains a floating-point value.
////////////////////////////////////////////////////////////////////
class P3DFloatVariant : public P3DVariant {
public:
  P3DFloatVariant(double value);
  P3DFloatVariant(const P3DFloatVariant &copy);

public:
  virtual P3DVariant *make_copy(); 
  virtual bool get_bool() const;
  virtual int get_int() const;
  virtual double get_float() const;
  virtual void make_string(string &value) const;

private:
  double _value;
};

#endif

