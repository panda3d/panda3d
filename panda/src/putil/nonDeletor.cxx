// Filename: nonDeletor.cxx
// Created by:  drose (10Apr06)
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

#include "nonDeletor.h"
#include "config_util.h"

////////////////////////////////////////////////////////////////////
//     Function: NonDeletor::delete_object
//       Access: Public, Virtual
//  Description: Adds the pointer to the object to be deleted, along
//               with a pointer to a function that can delete it.
////////////////////////////////////////////////////////////////////
void NonDeletor::
delete_object(DeleteFunc *, void *ptr) {
  if (util_cat.is_spam()) {
    util_cat.spam()
      << "Not deleting pointer " << ptr << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonDeletor::register_deletor
//       Access: Public, Static
//  Description: Registers this deletor with the global pool.
////////////////////////////////////////////////////////////////////
void NonDeletor::
register_deletor() {
  register_subclass(new NonDeletor, "none");
}
