/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file factoryParams.cxx
 * @author drose
 * @date 2000-05-08
 */

#include "factoryParams.h"

/**
 *
 */
void FactoryParams::
add_param(FactoryParam *param) {
  nassertv(param != nullptr);
  _params.push_back(param);
}

/**
 * Removes all parameters from the set.
 */
void FactoryParams::
clear() {
  _params.clear();
}

/**
 * Returns the number of parameters that have been added to the set.
 */
int FactoryParams::
get_num_params() const {
  return _params.size();
}

/**
 * Returns the nth parameter that has been added to the set.
 */
FactoryParam *FactoryParams::
get_param(int n) const {
  nassertr(n >= 0 && n < (int)_params.size(), nullptr);
  return DCAST(FactoryParam, _params[n]);
}

/**
 * Returns the first parameter that matches exactly the indicated type, or if
 * there are no exact matches, returns the first one that derives from the
 * indicated type.  If no parameters match at all, returns NULL.
 */
FactoryParam *FactoryParams::
get_param_of_type(TypeHandle type) const {
  Params::const_iterator pi;

  // First, search for the exact match.
  for (pi = _params.begin(); pi != _params.end(); ++pi) {
    FactoryParam *param;
    DCAST_INTO_R(param, *pi, nullptr);
    nassertr(param != nullptr, nullptr);

    if (param->is_exact_type(type)) {
      return param;
    }
  }

  // Now, search for a derived match.
  for (pi = _params.begin(); pi != _params.end(); ++pi) {
    FactoryParam *param;
    DCAST_INTO_R(param, *pi, nullptr);
    nassertr(param != nullptr, nullptr);

    if (param->is_of_type(type)) {
      return param;
    }
  }

  return nullptr;
}
