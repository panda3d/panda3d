// Filename: parameter_ext.h
// Created by:  rdb (28Jul15)
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

#ifndef PARAMETER_EXT_H
#define PARAMETER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "parameter.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<Parameter>
// Description : Python extensions for Parameter class.
////////////////////////////////////////////////////////////////////
template<>
class EXPCL_PANDA_EVENT Extension<Parameter> : public ExtensionBase<Parameter> {
public:
  void set_value(PyObject *value);
  PyObject *get_value() const;
};

#endif  // HAVE_PYTHON

#endif  // PARAMETER_EXT_H
