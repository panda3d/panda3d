// Filename: p3dConcreteSequence.h
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

#ifndef P3DCONCRETESEQUENCE_H
#define P3DCONCRETESEQUENCE_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DConcreteSequence
// Description : An object type that contains a sequence of objects,
//               which is passed by value between Python and
//               JavaScript, so may be more optimal for small lists
//               that are accessed repeatedly.
//
//               This is converted from a Python "tuple" object.
////////////////////////////////////////////////////////////////////
class P3DConcreteSequence : public P3DObject {
public:
  P3DConcreteSequence();
  virtual ~P3DConcreteSequence();

  virtual bool is_sequence_object();

  virtual P3D_object_type get_type();
  virtual bool get_bool();

  virtual void make_string(string &value);

  virtual P3D_object *get_property(const string &property);
  virtual bool set_property(const string &property, P3D_object *value);

  virtual bool fill_xml(TiXmlElement *xvalue, P3DSession *session);
  virtual P3D_object **get_object_array();
  virtual int get_object_array_size();

  int get_length() const;
  P3D_object *get_element(int n) const;
  bool set_element(int n, P3D_object *value);
  void append(P3D_object *value);

private:
  typedef vector<P3D_object *> Elements;
  Elements _elements;
};

#endif

