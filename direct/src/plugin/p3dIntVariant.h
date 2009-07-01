// Filename: p3dIntVariant.h
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

#ifndef P3DINTVARIANT_H
#define P3DINTVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DIntVariant
// Description : A variant type that contains an integer value.
////////////////////////////////////////////////////////////////////
class P3DIntVariant : public P3DVariant {
public:
  P3DIntVariant(int value);
  P3DIntVariant(const P3DIntVariant &copy);

public:
  virtual P3DVariant *make_copy(); 
  virtual bool get_bool() const;
  virtual int get_int() const;
  virtual void make_string(string &value) const;

private:
  int _value;
};

#endif

