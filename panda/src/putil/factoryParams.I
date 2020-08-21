/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file factoryParams.I
 * @author drose
 * @date 2000-05-08
 */

#include "pnotify.h"

/**
 * Returns the custom pointer that was associated with the factory function.
 */
INLINE void *FactoryParams::
get_user_data() const {
  return _user_data;
}

/**
 * A handy convenience template function that extracts a parameter of the
 * indicated type from the FactoryParams list.  If the parameter type is
 * found, it fills the pointer and returns true; otherwise, it sets the
 * pointer to NULL and returns false.
 */
template<class ParamType>
bool get_param_into(ParamType *&pointer, const FactoryParams &params) {
  FactoryParam *param =
    params.get_param_of_type(ParamType::get_class_type());
  if (param == nullptr) {
    pointer = nullptr;
    return false;
  }
  DCAST_INTO_R(pointer, param, false);
  return true;
}
