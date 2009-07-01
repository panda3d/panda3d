// Filename: p3dNoneValue.h
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

#ifndef P3DNONEVALUE_H
#define P3DNONEVALUE_H

#include "p3d_plugin_common.h"
#include "p3dValue.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DNoneValue
// Description : A value type that contains no value, similar to
//               Python's None type.
////////////////////////////////////////////////////////////////////
class P3DNoneValue : public P3DValue {
public:
  P3DNoneValue();

public:
  virtual P3DValue *make_copy();
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;

  virtual TiXmlElement *make_xml() const;
};

#endif

