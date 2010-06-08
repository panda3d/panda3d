// Filename: p3dPythonObject.h
// Created by:  drose (02Jul09)
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

#ifndef P3DPYTHONOBJECT_H
#define P3DPYTHONOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

class P3DSession;

////////////////////////////////////////////////////////////////////
//       Class : P3DPythonObject
// Description : An object type that references a PyObject in the
//               subordinate process.  It allows querying and/or
//               modifying the state of the referenced PyObject, via
//               clever XML communication in
//               P3DSession::command_and_response().
////////////////////////////////////////////////////////////////////
class P3DPythonObject : public P3DObject {
public:
  P3DPythonObject(P3DSession *session, int object_id);
  virtual ~P3DPythonObject();

public:
  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual int get_int();
  virtual double get_float();

  virtual void make_string(string &value);

  virtual P3D_object *get_property(const string &property);
  virtual bool set_property(const string &property, bool needs_response, P3D_object *value);
  bool set_property_insecure(const string &property, bool needs_response,
                             P3D_object *value);

  virtual bool has_method(const string &method_name);
  virtual P3D_object *call(const string &method_name, bool needs_response,
                           P3D_object *params[], int num_params);
  P3D_object *call_insecure(const string &method_name, bool needs_response,
                            P3D_object *params[], int num_params);

  virtual void output(ostream &out);
  virtual bool fill_xml(TiXmlElement *xvalue, P3DSession *session);

  virtual P3DPythonObject *as_python_object();

  P3DSession *get_session();
  int get_object_id();

private:
  P3DSession *_session;
  int _object_id;

  typedef map<string, bool> HasMethod;
  HasMethod _has_method;
};

#endif

