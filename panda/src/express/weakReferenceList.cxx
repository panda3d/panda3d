// Filename: weakReferenceList.cxx
// Created by:  drose (27Sep04)
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

#include "weakReferenceList.h"

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
  Pointers::iterator pi;
  for (pi = _pointers.begin(); pi != _pointers.end(); ++pi) {
    (*pi)->mark_deleted();
  }
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
  bool inserted = _pointers.insert(ptv).second;
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
  Pointers::iterator pi = _pointers.find(ptv);
  nassertv_always(pi != _pointers.end());
  _pointers.erase(pi);
}
