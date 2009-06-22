// Filename: p3dCInstance.cxx
// Created by:  drose (08Jun09)
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

#include "p3dCInstance.h"


////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Constructor
//       Access: Public
//  Description: Constructs a new Instance from an XML description.
////////////////////////////////////////////////////////////////////
P3DCInstance::
P3DCInstance(TiXmlElement *xinstance) :
  _func(NULL)
{
  xinstance->Attribute("id", &_instance_id);

  const char *p3d_filename = xinstance->Attribute("p3d_filename");
  if (p3d_filename != NULL) {
    _p3d_filename = p3d_filename;
  }

  TiXmlElement *xtoken = xinstance->FirstChildElement("token");
  while (xtoken != NULL) {
    Token token;
    const char *keyword = xtoken->Attribute("keyword");
    if (keyword != NULL) {
      token._keyword = keyword;
    }

    const char *value = xtoken->Attribute("value");
    if (value != NULL) {
      token._value = value;
    }

    _tokens.push_back(token);
    xtoken = xtoken->NextSiblingElement("token");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DCInstance::
~P3DCInstance() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::get_py_tokens
//       Access: Public
//  Description: Returns a Python list object that corresponds to the
//               tokens passed to this instance, expressed as a list
//               of 2-tuples.  New instance.
////////////////////////////////////////////////////////////////////
PyObject *P3DCInstance::
get_py_tokens() const {
  PyObject *list = PyList_New(_tokens.size());

  for (size_t i = 0; i < _tokens.size(); ++i) {
    const Token &token = _tokens[i];
    PyObject *tuple = Py_BuildValue("(ss)", token._keyword.c_str(), 
                                    token._value.c_str());
    PyList_SetItem(list, i, tuple);
  }

  return list;
}
