// Filename: eventParameter.cxx
// Created by:  drose (08Feb99)
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

#include "eventParameter.h"
#include "dcast.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle EventStoreValueBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EventParameter::
output(ostream &out) const {
  if (_ptr == (TypedWritableReferenceCount *)NULL) {
    out << "(empty)";

  } else if (_ptr->is_of_type(EventStoreValueBase::get_class_type())) {
    const EventStoreValueBase *sv_ptr;
    DCAST_INTO_V(sv_ptr, _ptr);
    sv_ptr->output(out);

  } else {
    out << _ptr->get_type();
  }
}
