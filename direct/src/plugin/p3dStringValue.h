// Filename: p3dStringValue.h
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

#ifndef P3DSTRINGVALUE_H
#define P3DSTRINGVALUE_H

#include "p3d_plugin_common.h"
#include "p3dValue.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DStringValue
// Description : A value type that contains a string value.
////////////////////////////////////////////////////////////////////
class P3DStringValue : public P3DValue {
public:
  P3DStringValue(const string &value);
  P3DStringValue(const P3DStringValue &copy);

public:
  virtual ~P3DStringValue();

  virtual P3DValue *make_copy(); 
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;

  virtual TiXmlElement *make_xml() const;

  virtual void output(ostream &out) const;

private:
  string _value;
};

#endif

