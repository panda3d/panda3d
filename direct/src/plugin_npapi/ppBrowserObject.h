// Filename: ppBrowserObject.h
// Created by:  drose (05Jul09)
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

#ifndef PPBROWSEROBJECT_H
#define PPBROWSEROBJECT_H

#include "nppanda3d_common.h"

class PPInstance;

////////////////////////////////////////////////////////////////////
//       Class : PPBrowserObject
// Description : This is the interface layer between an NPObject and a
//               P3D_object.  It maps calls from P3D_object into the
//               NPObject system, thus allowing Panda to view and
//               operate on a browser object.
//
//               Also see PPPandaObject, which maps calls the other
//               way.
////////////////////////////////////////////////////////////////////
class PPBrowserObject : public P3D_object {
public:
  PPBrowserObject(PPInstance *inst, NPObject *npobj);
  PPBrowserObject(const PPBrowserObject &copy);
  ~PPBrowserObject();

  int get_repr(char *buffer, int buffer_length) const;
  P3D_object *get_property(const string &property) const;
  bool set_property(const string &property, bool needs_response,
                    P3D_object *value);

  P3D_object *call(const string &method_name, 
                   P3D_object *params[], int num_params) const;
  P3D_object *eval(const string &expression) const;

  static void clear_class_definition();

private:
  static P3D_class_definition *get_class_definition();

private:
  PPInstance *_instance;
  NPObject *_npobj;
  static P3D_class_definition *_browser_object_class;
};

#include "ppBrowserObject.I"

#endif

