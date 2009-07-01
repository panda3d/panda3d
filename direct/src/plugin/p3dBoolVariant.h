// Filename: p3dBoolVariant.h
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

#ifndef P3DBOOLVARIANT_H
#define P3DBOOLVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DBoolVariant
// Description : A variant type that contains a boolean value.
////////////////////////////////////////////////////////////////////
class P3DBoolVariant : public P3DVariant {
public:
  P3DBoolVariant(bool value);
  P3DBoolVariant(const P3DBoolVariant &copy);

public:
  virtual P3DVariant *make_copy(); 
  virtual bool get_bool() const;
  virtual int get_int() const;
  virtual void make_string(string &value) const;

private:
  bool _value;
};

#endif

