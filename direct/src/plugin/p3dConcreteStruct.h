// Filename: p3dConcreteStruct.h
// Created by:  drose (14Jul09)
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

#ifndef P3DCONCRETESTRUCT_H
#define P3DCONCRETESTRUCT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DConcreteStruct
// Description : A simple object that contains a standard mapping of
//               string -> element.  It is passed by value between
//               Python and Javascript, so it may be more optimal for
//               relatively small objects.
//
//               Methods are not supported, other than built-in
//               methods like toString().
////////////////////////////////////////////////////////////////////
class P3DConcreteStruct : public P3DObject {
public:
  P3DConcreteStruct();
  virtual ~P3DConcreteStruct();

  virtual P3D_object_type get_type();
  virtual bool get_bool();

  virtual void make_string(string &value);

  virtual P3D_object *get_property(const string &property);
  virtual bool set_property(const string &property, P3D_object *value);

  virtual bool has_method(const string &method_name);
  virtual P3D_object *call(const string &method_name, bool needs_response,
                           P3D_object *params[], int num_params);

  virtual bool fill_xml(TiXmlElement *xvalue, P3DSession *session);

private:
  typedef map<string, P3D_object *> Elements;
  Elements _elements;
};

#endif

