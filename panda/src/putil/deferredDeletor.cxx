// Filename: deferredDeletor.cxx
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

#include "deferredDeletor.h"
#include "config_util.h"
#include "mutexHolder.h"

////////////////////////////////////////////////////////////////////
//     Function: DeferredDeletor::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DeferredDeletor::
DeferredDeletor() : _lock("DeferredDeletor") {
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredDeletor::delete_object
//       Access: Public, Virtual
//  Description: Adds the pointer to the object to be deleted, along
//               with a pointer to a function that can delete it.
////////////////////////////////////////////////////////////////////
void DeferredDeletor::
delete_object(DeleteFunc *func, void *ptr) {
  MutexHolder holder(_lock);
  if (util_cat.is_spam()) {
    util_cat.spam()
      << "Deferring deleting pointer " << ptr << "\n";
  }
  _tokens.push_back(DeleteToken(func, ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredDeletor::flush
//       Access: Public, Virtual
//  Description: Ensures that any objects queued up for deletion have
//               been fully deleted by the time flush() returns.
////////////////////////////////////////////////////////////////////
void DeferredDeletor::
flush() {
  Tokens new_tokens;
  {
    MutexHolder holder(_lock);
    _tokens.swap(new_tokens);
  }

  Tokens::iterator ti;
  for (ti = new_tokens.begin(); ti != new_tokens.end(); ++ti) {
    (*ti).do_delete();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredDeletor::register_deletor
//       Access: Public, Static
//  Description: Registers this deletor with the global pool.
////////////////////////////////////////////////////////////////////
void DeferredDeletor::
register_deletor() {
  register_subclass(new DeferredDeletor, "deferred");
}
