// Filename: factoryParams.I
// Created by:  drose (08May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "notify.h"


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
