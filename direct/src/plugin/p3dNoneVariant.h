// Filename: p3dNoneVariant.h
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

#ifndef P3DNONEVARIANT_H
#define P3DNONEVARIANT_H

#include "p3d_plugin_common.h"
#include "p3dVariant.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DNoneVariant
// Description : A variant type that contains no value, similar to
//               Python's None type.
////////////////////////////////////////////////////////////////////
class P3DNoneVariant : public P3DVariant {
public:
  P3DNoneVariant();

public:
  virtual P3DVariant *make_copy();
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;
};

#endif

