// Filename: p3dIntObject.h
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

#ifndef P3DINTOBJECT_H
#define P3DINTOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DIntObject
// Description : An object type that contains an integer value.
////////////////////////////////////////////////////////////////////
class P3DIntObject : public P3DObject {
public:
  P3DIntObject(int value);
  P3DIntObject(const P3DIntObject &copy);

public:
  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual int get_int();
  virtual void make_string(string &value);

private:
  int _value;
};

#endif

