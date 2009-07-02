// Filename: p3dListObject.h
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

#ifndef P3DLISTOBJECT_H
#define P3DLISTOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DListObject
// Description : An object type that contains a list of objects.
////////////////////////////////////////////////////////////////////
class P3DListObject : public P3DObject {
public:
  P3DListObject();
  P3DListObject(const P3DListObject &copy);

public:
  virtual ~P3DListObject();

  virtual P3DObject *make_copy() const; 
  virtual bool get_bool() const;
  virtual void make_string(string &value) const;
  virtual int get_list_length() const;
  virtual P3D_object *get_element(int n) const;
  virtual bool set_element(int n, P3D_object *value);

  virtual TiXmlElement *make_xml() const;

private:
  typedef vector<P3D_object *> Elements;
  Elements _elements;
};

#endif

