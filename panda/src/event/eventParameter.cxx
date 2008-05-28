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

TypeHandle EventStoreValueBase::_type_handle;
TypeHandle EventStoreTypedRefCount::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::output
//       Access: Published
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

////////////////////////////////////////////////////////////////////
//     Function: EventStoreValueBase::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EventStoreValueBase::
~EventStoreValueBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreTypedRefCount::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EventStoreTypedRefCount::
~EventStoreTypedRefCount() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventStoreTypedRefCount::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void EventStoreTypedRefCount::
output(ostream &out) const {
  if (_value == (TypedReferenceCount *)NULL) {
    out << "(empty)";

  } else {
    out << _value->get_type();
  }
}
