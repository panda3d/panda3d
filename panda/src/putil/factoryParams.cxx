// Filename: factoryParams.cxx
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

#include "factoryParams.h"

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FactoryParams::
FactoryParams() {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FactoryParams::
~FactoryParams() {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::add_param
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FactoryParams::
add_param(FactoryParam *param) {
  nassertv(param != (FactoryParam *)NULL);
  _params.push_back(param);
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::clear
//       Access: Public
//  Description: Removes all parameters from the set.
////////////////////////////////////////////////////////////////////
void FactoryParams::
clear() {
  _params.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::get_num_params
//       Access: Public
//  Description: Returns the number of parameters that have been added
//               to the set.
////////////////////////////////////////////////////////////////////
int FactoryParams::
get_num_params() const {
  return _params.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::get_param
//       Access: Public
//  Description: Returns the nth parameter that has been added to the
//               set.
////////////////////////////////////////////////////////////////////
FactoryParam *FactoryParams::
get_param(int n) const {
  nassertr(n >= 0 && n < (int)_params.size(), NULL);
  return DCAST(FactoryParam, _params[n]);
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryParams::get_param_of_type
//       Access: Public
//  Description: Returns the first parameter that matches exactly the
//               indicated type, or if there are no exact matches,
//               returns the first one that derives from the indicated
//               type.  If no parameters match at all, returns NULL.
////////////////////////////////////////////////////////////////////
FactoryParam *FactoryParams::
get_param_of_type(TypeHandle type) const {
  Params::const_iterator pi;

  // First, search for the exact match.
  for (pi = _params.begin(); pi != _params.end(); ++pi) {
    FactoryParam *param;
    DCAST_INTO_R(param, *pi, NULL);
    nassertr(param != (FactoryParam *)NULL, NULL);

    if (param->is_exact_type(type)) {
      return param;
    }
  }

  // Now, search for a derived match.
  for (pi = _params.begin(); pi != _params.end(); ++pi) {
    FactoryParam *param;
    DCAST_INTO_R(param, *pi, NULL);
    nassertr(param != (FactoryParam *)NULL, NULL);

    if (param->is_of_type(type)) {
      return param;
    }
  }

  return NULL;
}
