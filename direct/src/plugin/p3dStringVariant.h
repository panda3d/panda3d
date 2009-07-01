// Filename: p3dStringVariant.h
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

#ifndef P3DSTRINGVARIANT_H
#define P3DSTRINGVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DStringVariant
// Description : A variant type that contains a string value.
////////////////////////////////////////////////////////////////////
class P3DStringVariant : public P3DVariant {
public:
  P3DStringVariant(const char *value, int length);
  P3DStringVariant(const P3DStringVariant &copy);

public:
  virtual ~P3DStringVariant();

  virtual P3DVariant *make_copy(); 
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;

private:
  string _value;
};

#endif

