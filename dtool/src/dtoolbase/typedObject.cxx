// Filename: typedObject.cxx
// Created by:  drose (11May01)
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

#include "typedObject.h"

TypeHandle TypedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypedObject::
~TypedObject() {
}


////////////////////////////////////////////////////////////////////
//     Function: TypedObject::get_type
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypeHandle TypedObject::
get_type() const {
  // Normally, this function should never be called, because it is a
  // pure virtual function.  If it is called, you probably called
  // get_type() on a recently-destructed object.
  cerr
    << "TypedObject::get_type() called!\n";
  return _type_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::init_type
//       Access: Public, Static
//  Description: This function is declared non-inline to work around a
//               compiler bug in g++ 2.96.  Making it inline seems to
//               cause problems in the optimizer.
////////////////////////////////////////////////////////////////////
void TypedObject::
init_type() {
  register_type(_type_handle, "TypedObject");
}
