// Filename: typedObject.cxx
// Created by:  drose (11May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "typedObject.h"
#include "config_interrogatedb.h"


TypeHandle TypedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypedObject::
~TypedObject() {
}


////////////////////////////////////////////////////////////////////
//     Function: TypedObject::get_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypeHandle TypedObject::
get_type() const {
  // Normally, this function should never be called, because it is a
  // pure virtual function.  If it is called, you probably called
  // get_type() on a recently-destructed object.
  interrogatedb_cat.warning()
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
