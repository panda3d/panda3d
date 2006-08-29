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
//     Function: EventParameter::Pointer constructor
//       Access: Public
//  Description: Defines an EventParameter that stores a pointer to
//               a TypedReferenceCount object.  Note that a
//               TypedReferenceCount is not the same kind of pointer
//               as a TypedWritableReferenceCount, hence we require
//               both constructors.
//
//               This accepts a const pointer, even though it stores
//               (and eventually returns) a non-const pointer.  This
//               is just the simplest way to allow both const and
//               non-const pointers to be stored, but it does lose the
//               constness.  Be careful.
//
//               This constructor, and the accessors for this type of
//               event parameter, are declared non-inline so we don't
//               have to worry about exporting this template class
//               from this DLL.
////////////////////////////////////////////////////////////////////
EventParameter::
EventParameter(const TypedReferenceCount *ptr) : _ptr(new EventStoreTypedRefCount((TypedReferenceCount *)ptr)) { }

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::is_typed_ref_count
//       Access: Public
//  Description: Returns true if the EventParameter stores a
//               TypedReferenceCount pointer, false otherwise.  Note
//               that a TypedReferenceCount is not exactly the same
//               kind of pointer as a TypedWritableReferenceCount,
//               hence the need for this separate call.
////////////////////////////////////////////////////////////////////
bool EventParameter::
is_typed_ref_count() const {
  if (is_empty()) {
    return false;
  }
  return _ptr->is_of_type(EventStoreTypedRefCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: EventParameter::get_typed_ref_count_value
//       Access: Public
//  Description: Retrieves the value stored in the EventParameter.  It
//               is only valid to call this if is_typed_ref_count()
//               has already returned true.
////////////////////////////////////////////////////////////////////
TypedReferenceCount *EventParameter::
get_typed_ref_count_value() const {
  nassertr(is_typed_ref_count(), NULL);
  return ((const EventStoreTypedRefCount *)_ptr.p())->get_value();
}

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
