/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJoint_ext.h
 * @author rdb
 * @date 2013-12-11
 */

#ifndef ODEJOINT_EXT_H
#define ODEJOINT_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "config_ode.h"
#include "odeJoint.h"
#include "extension.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePathCollection, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<OdeJoint> : public ExtensionBase<OdeJoint> {
public:
  void attach(PyObject *body1, PyObject *body2);

  PyObject *convert() const;
};

#endif  // HAVE_PYTHON

#endif  // ODEJOINT_EXT_H
