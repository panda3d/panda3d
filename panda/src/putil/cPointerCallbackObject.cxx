/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cPointerCallbackObject.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "cPointerCallbackObject.h"

TypeHandle CPointerCallbackObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CPointerCallbackObject::do_callback
//       Access: Public, Virtual
//  Description: This method called when the callback is triggered; it
//               *replaces* the original function.  To continue
//               performing the original function, you must call
//               cbdata->upcall() during the callback.
////////////////////////////////////////////////////////////////////
void CPointerCallbackObject::
do_callback(CallbackData *cbdata) {
  (*_func)(cbdata, _data);
}

