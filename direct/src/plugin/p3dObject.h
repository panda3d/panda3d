// Filename: p3dObject.h
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

#ifndef P3DOBJECT_H
#define P3DOBJECT_H

#include "p3d_plugin_common.h"

class P3DPythonObject;

////////////////////////////////////////////////////////////////////
//       Class : P3DObject
// Description : The C++ implementation of P3D_value, corresponding
//               to a single atomic value that is passed around
//               between scripting languages.  This is an abstract
//               base class; the actual implementations are provided
//               by the various specialized classes.
////////////////////////////////////////////////////////////////////
class P3DObject : public P3D_object {
protected:
  inline P3DObject();
  inline P3DObject(const P3DObject &copy);

public:
  virtual ~P3DObject();

  virtual P3D_object_type get_type()=0;
  virtual bool get_bool()=0;
  virtual int get_int();
  virtual double get_float();

  int get_string(char *buffer, int buffer_length);
  int get_repr(char *buffer, int buffer_length);
  virtual void make_string(string &value)=0;

  virtual P3D_object *get_property(const string &property);
  virtual bool set_property(const string &property, bool needs_response,
                            P3D_object *value);

  virtual bool has_method(const string &method_name);
  virtual P3D_object *call(const string &method_name, bool needs_response,
                           P3D_object *params[], int num_params);
  virtual P3D_object *eval(const string &expression);

  virtual void output(ostream &out);
  virtual bool fill_xml(TiXmlElement *xvalue, P3DSession *session);
  virtual P3D_object **get_object_array();
  virtual int get_object_array_size();

  virtual P3DPythonObject *as_python_object();

  // Convenience functions.
  bool get_bool_property(const string &property);
  void set_bool_property(const string &property, bool value);

  int get_int_property(const string &property);
  void set_int_property(const string &property, int value);

  double get_float_property(const string &property);
  void set_float_property(const string &property, double value);

  string get_string_property(const string &property);
  void set_string_property(const string &property, const string &value);

  void set_undefined_property(const string &property);

public:
  static P3D_class_definition _object_class;
  static P3D_class_definition _generic_class;
};

#include "p3dObject.I"

// For classes that inherit from P3DObject, above, we can use the
// virtual method to write the output simply.  (For classes that
// inherit only from P3D_object, we have to use the generic C method
// defined in p3d_plugin_common.h, a little clumsier.)
inline ostream &operator << (ostream &out, P3DObject &value) {
  value.output(out);
  return out;
}

#endif

