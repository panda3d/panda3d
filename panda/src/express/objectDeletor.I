// Filename: objectDeletor.I
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


////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::get_global_ptr
//       Access: Public, Static
//  Description: Returns the global ObjectDeletor object.  This may
//               return NULL if there is no such object.
////////////////////////////////////////////////////////////////////
INLINE ObjectDeletor *ObjectDeletor::
get_global_ptr() {
  return (ObjectDeletor *)AtomicAdjust::get_ptr(_global_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::set_global_ptr
//       Access: Public, Static
//  Description: Assigns the global ObjectDeletor object.  Returns
//               the pointer to the previous object, if any.
////////////////////////////////////////////////////////////////////
INLINE ObjectDeletor *ObjectDeletor::
set_global_ptr(ObjectDeletor *ptr) {
  return (ObjectDeletor *)AtomicAdjust::set_ptr(_global_ptr, ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::DeleteToken::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ObjectDeletor::DeleteToken::
DeleteToken(DeleteFunc *func, void *ptr) :
  _func(func),
  _ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ObjectDeletor::DeleteToken::do_delete
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ObjectDeletor::DeleteToken::
do_delete() {
  (*_func)(_ptr);
}
