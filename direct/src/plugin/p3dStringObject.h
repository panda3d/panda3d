// Filename: p3dStringObject.h
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

#ifndef P3DSTRINGOBJECT_H
#define P3DSTRINGOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DStringObject
// Description : An object type that contains a string value.
////////////////////////////////////////////////////////////////////
class P3DStringObject : public P3DObject {
public:
  P3DStringObject(const string &value);
  P3DStringObject(const char *data, size_t size);
  P3DStringObject(const P3DStringObject &copy);

public:
  virtual ~P3DStringObject();

  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual void make_string(string &value);

  virtual void output(ostream &out);

private:
  string _value;
};

#endif

