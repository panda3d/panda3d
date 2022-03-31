/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackObject.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "callbackObject.h"

TypeHandle CallbackObject::_type_handle;


/**
 *
 */
void CallbackObject::
output(std::ostream &out) const {
  out << get_type();
}

/**
 * This method called when the callback is triggered; it *replaces* the
 * original function.  To continue performing the original function, you must
 * call cbdata->upcall() during the callback.
 */
void CallbackObject::
do_callback(CallbackData *) {
}
