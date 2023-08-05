/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedSkel.cxx
 * @author jyelon
 * @date 2007-01-31
 */

#include "typedSkel.h"

TypeHandle TypedSkel::_type_handle;

/**
 * Stores an integer value.  Exact same functionality as set_value, except
 * that this isn't an inline function.
 */
void TypedSkel::
set_value_alt(int n) {
  _value = n;
}

/**
 * Retreives a value that was previously stored.  Exact same functionality as
 * get_value, except that this isn't an inline function.
 */
int TypedSkel::
get_value_alt() const {
  return _value;
}
