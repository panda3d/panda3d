// Filename: rocketRegion_ext.I
// Created by:  rdb (06Dec11)
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

#include "rocketRegion.h"

#ifndef CPPPARSER
#undef this
#include <Rocket/Core/Python/Utilities.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: RocketRegion::get_context
//       Access: Published
//  Description: Returns a pointer to the Rocket context associated
//               with this region.  Will only be valid as long as this
//               region still exists, so be sure to toss it when you
//               toss the region itself.
//               It's best to call this method just once and store
//               the context in a Python variable, to avoid overhead.
////////////////////////////////////////////////////////////////////
PyObject* EXT_CONST_METHOD(RocketRegion,
get_context) {
  python::object py_context;

  try {
    Rocket::Core::Context* context = _ext_this->get_context();
    py_context = Rocket::Core::Python::Utilities::MakeObject(context);
  } catch (const python::error_already_set& e) {
    return NULL;
  }

  return py_context.ptr();
}
