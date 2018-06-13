/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackData.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "callbackData.h"

TypeHandle CallbackData::_type_handle;


/**
 *
 */
void CallbackData::
output(std::ostream &out) const {
  out << get_type();
}

/**
 * You should make this call during the callback if you want to continue the
 * normal function that would have been done in the absence of a callback.
 */
void CallbackData::
upcall() {
}
