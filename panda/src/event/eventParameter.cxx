// Filename: eventParameter.cxx
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

#include "eventParameter.h"
#include "dcast.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void EventParameter::
output(ostream &out) const {
  if (_ptr == (TypedWritableReferenceCount *)NULL) {
    out << "(empty)";

  } else if (_ptr->is_of_type(ParamValueBase::get_class_type())) {
    const ParamValueBase *sv_ptr;
    DCAST_INTO_V(sv_ptr, _ptr);
    sv_ptr->output(out);

  } else {
    out << _ptr->get_type();
  }
}
