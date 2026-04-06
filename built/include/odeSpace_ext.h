/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef ODESPACE_EXT_H
#define ODESPACE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "config_ode.h"
#include "odeSpace.h"
#include "extension.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePathCollection, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<OdeSpace> : public ExtensionBase<OdeSpace> {
public:
  INLINE PyObject *get_AA_bounds() const;

  PyObject *convert() const;
  INLINE PyObject *get_converted_geom(int index) const;
  INLINE PyObject *get_converted_space() const;

  int collide(PyObject* arg, PyObject* near_callback);

private:
  static void near_callback(void*, dGeomID, dGeomID);

  static PyObject *_python_callback;
};

#include "odeSpace_ext.I"

#endif  // HAVE_PYTHON

#endif  // ODESPACE_EXT_H
