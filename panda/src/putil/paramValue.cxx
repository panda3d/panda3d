// Filename: paramValue.cxx
// Created by:  drose (08Feb99)
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

#include "paramValue.h"
#include "dcast.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle ParamValueBase::_type_handle;
TypeHandle ParamTypedRefCount::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ParamValueBase::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParamValueBase::
~ParamValueBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTypedRefCount::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ParamTypedRefCount::
~ParamTypedRefCount() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParamTypedRefCount::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ParamTypedRefCount::
output(ostream &out) const {
  if (_value == (TypedReferenceCount *)NULL) {
    out << "(empty)";

  } else {
    out << _value->get_type();
  }
}
