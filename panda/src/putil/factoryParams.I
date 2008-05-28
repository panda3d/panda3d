// Filename: factoryParams.I
// Created by:  drose (08May00)
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

#include "pnotify.h"


////////////////////////////////////////////////////////////////////
//     Function: get_param_into
//  Description: A handy convenience template function that extracts a
//               parameter of the indicated type from the
//               FactoryParams list.  If the parameter type is found,
//               it fills the pointer and returns true; otherwise, it
//               sets the pointer to NULL and returns false.
////////////////////////////////////////////////////////////////////
template<class ParamType>
bool get_param_into(ParamType *&pointer, const FactoryParams &params) {
  FactoryParam *param =
    params.get_param_of_type(ParamType::get_class_type());
  if (param == (FactoryParam *)NULL) {
    pointer = NULL;
    return false;
  }
  DCAST_INTO_R(pointer, param, false);
  return true;
}
