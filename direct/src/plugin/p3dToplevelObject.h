// Filename: p3dToplevelObject.h
// Created by:  drose (10Jul09)
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

#ifndef P3DTOPLEVELOBJECT_H
#define P3DTOPLEVELOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"
#include <map>

class P3DSession;

////////////////////////////////////////////////////////////////////
//       Class : P3DToplevelObject
// Description : This corresponds to the toplevel "Python" object
//               owned by a particular instance, as returned by
//               P3DInstance::get_panda_script_object().
//
//               This is mostly a wrapper around a P3DPythonObject
//               pointer, and therefore functions like any other
//               P3DPythonObject; but it also handles the special case
//               of being available before Python has been started;
//               and it furthermore reports properties that are
//               generated directly by the core API (like
//               downloadProgress and such).
////////////////////////////////////////////////////////////////////
class P3DToplevelObject : public P3DObject {
public:
  P3DToplevelObject();
  virtual ~P3DToplevelObject();

public:
  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual int get_int();
  virtual double get_float();

  virtual void make_string(string &value);

  virtual P3D_object *get_property(const string &property);
  virtual bool set_property(const string &property, P3D_object *value);

  virtual bool has_method(const string &method_name);
  virtual P3D_object *call(const string &method_name, 
                           P3D_object *params[], int num_params);

  virtual void output(ostream &out);

  void set_pyobj(P3D_object *pyobj);
  P3D_object *get_pyobj() const;

private:
  P3D_object *_pyobj;

  // This map is used to store properties and retrieve until
  // set_pyobj() is called for the firs ttime.  At that point, the
  // properties stored here are transferred down to the internal
  // PyObject.
  typedef map<string, P3D_object *> Properties;
  Properties _properties;
};

#endif

