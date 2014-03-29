// Filename: odeGeom_ext.h
// Created by:  rdb (11Dec13)
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

#ifndef ODEGEOM_EXT_H
#define ODEGEOM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "config_ode.h"
#include "odeGeom.h"
#include "extension.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<OdeGeom>
// Description : This class defines the extension methods for
//               NodePathCollection, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<OdeGeom> : public ExtensionBase<OdeGeom> {
public:
  INLINE PyObject *get_AA_bounds() const;

  PyObject *convert() const;
  INLINE PyObject *get_converted_space() const;
};

#include "odeGeom_ext.I"

#endif  // HAVE_PYTHON

#endif  // ODEGEOM_EXT_H
