// Filename: callbackData.cxx
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

#include "callbackData.h"

TypeHandle CallbackData::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CallbackData::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CallbackData::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CallbackData::upcall
//       Access: Published, Virtual
//  Description: You should make this call during the callback if you
//               want to continue the normal function that would have
//               been done in the absence of a callback.
////////////////////////////////////////////////////////////////////
void CallbackData::
upcall() {
}

