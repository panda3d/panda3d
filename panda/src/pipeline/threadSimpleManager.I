// Filename: threadSimpleManager.I
// Created by:  drose (19Jun07)
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
//     Function: ThreadSimpleManager::get_current_thread
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadSimpleImpl *ThreadSimpleManager::
get_current_thread() {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::get_current_time
//       Access: Public
//  Description: Returns elapsed time in seconds from some undefined
//               epoch, via whatever clock the manager is using for
//               all thread timing.
////////////////////////////////////////////////////////////////////
INLINE double ThreadSimpleManager::
get_current_time() const {
  return _clock->get_short_raw_time();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::get_global_ptr
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadSimpleManager *ThreadSimpleManager::
get_global_ptr() {
  if (!_pointers_initialized) {
    init_pointers();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::CompareStartTime::operator()
//       Access: Public
//  Description: STL function object to sort the priority queue of
//               sleeping threads.
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleManager::CompareStartTime::
operator ()(ThreadSimpleImpl *a, ThreadSimpleImpl *b) const {
  return a->get_start_time() < b->get_start_time();
}
