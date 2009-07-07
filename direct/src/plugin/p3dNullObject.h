// Filename: p3dNullObject.h
// Created by:  drose (07Jul09)
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

#ifndef P3DNULLOBJECT_H
#define P3DNULLOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DNullObject
// Description : An object type that represents a NULL pointer.
//               Python doesn't have such a concept, but C and
//               JavaScript do, and it is sometimes an important
//               return value.
////////////////////////////////////////////////////////////////////
class P3DNullObject : public P3DObject {
public:
  P3DNullObject();

public:
  virtual P3D_object_type get_type() const;
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;
};

#endif

