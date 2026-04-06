/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody_ext.h
 * @author rdb
 * @date 2013-12-11
 */

#ifndef ODEBODY_EXT_H
#define ODEBODY_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "config_ode.h"
#include "odeBody.h"
#include "extension.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePathCollection, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<OdeBody> : public ExtensionBase<OdeBody> {
public:
  void set_data(PyObject *);
  INLINE PyObject *get_data() const;

  INLINE PyObject *get_converted_joint(int i) const;
};

#include "odeBody_ext.I"

#endif  // HAVE_PYTHON

#endif  // ODEBODY_EXT_H
