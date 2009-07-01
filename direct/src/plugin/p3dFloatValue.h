// Filename: p3dFloatValue.h
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

#ifndef P3DFLOATVALUE_H
#define P3DFLOATVALUE_H

#include "p3d_plugin_common.h"
#include "p3dValue.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DFloatValue
// Description : A value type that contains a floating-point value.
////////////////////////////////////////////////////////////////////
class P3DFloatValue : public P3DValue {
public:
  P3DFloatValue(double value);
  P3DFloatValue(const P3DFloatValue &copy);

public:
  virtual P3DValue *make_copy(); 
  virtual bool get_bool() const;
  virtual int get_int() const;
  virtual double get_float() const;
  virtual void make_string(string &value) const;

  virtual TiXmlElement *make_xml() const;

private:
  double _value;
};

#endif

