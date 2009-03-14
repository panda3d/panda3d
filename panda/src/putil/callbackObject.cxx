// Filename: callbackObject.cxx
// Created by:  drose (13Mar09)
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

#include "callbackObject.h"

TypeHandle CallbackObject::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CallbackObject::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CallbackObject::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackObject::do_callback
//       Access: Public, Virtual
//  Description: This method called when the callback is triggered; it
//               *replaces* the original function.  To continue
//               performing the original function, you must call
//               cbdata->upcall() during the callback.
////////////////////////////////////////////////////////////////////
void CallbackObject::
do_callback(CallbackData *) {
}
