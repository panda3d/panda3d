/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeUtil_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef ODEUTIL_EXT_H
#define ODEUTIL_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "config_ode.h"
#include "odeUtil.h"
#include "extension.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePathCollection, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<OdeUtil> : public ExtensionBase<OdeUtil> {
public:
  static int collide2(const OdeGeom &geom1, const OdeGeom &geom2,
                      PyObject* arg, PyObject* callback);

private:
  static void near_callback(void*, dGeomID, dGeomID);

  static PyObject *_python_callback;
};

#endif  // HAVE_PYTHON

#endif  // ODEUTIL_EXT_H
