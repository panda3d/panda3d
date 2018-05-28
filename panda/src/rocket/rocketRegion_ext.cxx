/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRegion_ext.cxx
 * @author rdb
 * @date 2011-12-06
 */

#include "rocketRegion_ext.h"
#include "extension.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
#define HAVE_LONG_LONG 1
#include <Rocket/Core/Context.h>
#include <Rocket/Core/Python/Utilities.h>
#endif

/**
 * Returns a pointer to the Rocket context associated with this region.  Will
 * only be valid as long as this region still exists, so be sure to toss it
 * when you toss the region itself.  It's best to call this method just once
 * and store the context in a Python variable, to avoid overhead.
 */
PyObject* Extension<RocketRegion>::
get_context() const {
  try {
    Rocket::Core::Context* context = _this->get_context();
    python::object py_context = Rocket::Core::Python::Utilities::MakeObject(context);

    // Make sure the context won't be destroyed before both the Python
    // references and the C++ references to it are completely gone.
    Py_INCREF(py_context.ptr());
    context->AddReference();
    return py_context.ptr();

  } catch (const python::error_already_set &e) {
    (void)e;
    // Return NULL, which will trigger the exception in Python
  }
  return nullptr;
}

#endif
