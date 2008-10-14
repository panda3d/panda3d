// Filename: weakReferenceList.cxx
// Created by:  drose (27Sep04)
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

#include "weakReferenceList.h"
#include "weakPointerToVoid.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: WeakReferenceList::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WeakReferenceList::
WeakReferenceList() {
}

////////////////////////////////////////////////////////////////////
//     Function: WeakReferenceList::Destructor
//       Access: Public
//  Description: The destructor tells all of the owned references that
//               we're gone.
////////////////////////////////////////////////////////////////////
WeakReferenceList::
~WeakReferenceList() {
  _lock.acquire();
  Pointers::iterator pi;
  for (pi = _pointers.begin(); pi != _pointers.end(); ++pi) {
    (*pi)->mark_deleted();
  }
  _lock.release();
}

////////////////////////////////////////////////////////////////////
//     Function: WeakReferenceList::add_reference
//       Access: Public
//  Description: Intended to be called only by WeakPointerTo (or by
//               any class implementing a weak reference-counting
//               pointer), this adds the indicated PointerToVoid
//               structure to the list of such structures that are
//               maintaining a weak pointer to this object.
//
//               When the WeakReferenceList destructs (presumably
//               because its owning object destructs), the pointer
//               within the PointerToVoid object will be set to NULL.
////////////////////////////////////////////////////////////////////
void WeakReferenceList::
add_reference(WeakPointerToVoid *ptv) {
  _lock.acquire();
  bool inserted = _pointers.insert(ptv).second;
  _lock.release();
  nassertv(inserted);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakReferenceList::clear_reference
//       Access: Public
//  Description: Intended to be called only by WeakPointerTo (or by
//               any class implementing a weak reference-counting
//               pointer), this removes the indicated PointerToVoid
//               structure from the list of such structures that are
//               maintaining a weak pointer to this object.
////////////////////////////////////////////////////////////////////
void WeakReferenceList::
clear_reference(WeakPointerToVoid *ptv) {
  _lock.acquire();
  Pointers::iterator pi = _pointers.find(ptv);
  bool valid = (pi != _pointers.end());
  if (valid) {
    _pointers.erase(pi);
  }
  _lock.release();
  nassertv(valid);
}
