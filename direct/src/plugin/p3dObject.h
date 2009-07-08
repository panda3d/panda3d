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

  virtual P3DObject *make_copy() const; 
  virtual P3D_object_type get_type() const=0;
  virtual bool get_bool() const=0;
  virtual int get_int() const;
  virtual double get_float() const;

  int get_string(char *buffer, int buffer_length) const;
  int get_repr(char *buffer, int buffer_length) const;
  virtual void make_string(string &value) const=0;

  virtual P3D_object *get_property(const string &property) const;
  virtual bool set_property(const string &property, P3D_object *value);

  virtual P3D_object *call(const string &method_name, 
                           P3D_object *params[], int num_params) const;
  virtual P3D_object *eval(const string &expression) const;

  virtual void output(ostream &out) const;

  inline void ref() const;
  inline int unref() const;
  static inline void unref_delete(P3DObject *obj);

private:
  int _ref_count;

public:
  static P3D_class_definition _object_class;
  static P3D_class_definition _generic_class;
};

#include "p3dObject.I"

// For classes that inherit from P3DObject, above, we can use the
// virtual method to write the output simply.  (For classes that
// inherit only from P3D_object, we have to use the generic C method
// defined in p3d_plugin_common.h, a little clumsier.)
inline ostream &operator << (ostream &out, const P3DObject &value) {
  value.output(out);
  return out;
}

#endif

