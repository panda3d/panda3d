// Filename: p3dBoolValue.h
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

#ifndef P3DBOOLVALUE_H
#define P3DBOOLVALUE_H

#include "p3d_plugin_common.h"
#include "p3dValue.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DBoolValue
// Description : A value type that contains a boolean value.
////////////////////////////////////////////////////////////////////
class P3DBoolValue : public P3DValue {
public:
  P3DBoolValue(bool value);
  P3DBoolValue(const P3DBoolValue &copy);

public:
  virtual P3DValue *make_copy(); 
  virtual bool get_bool() const;
  virtual int get_int() const;
  virtual void make_string(string &value) const;

  virtual TiXmlElement *make_xml() const;

private:
  bool _value;
};

#endif

