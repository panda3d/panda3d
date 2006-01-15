// Filename: thread.cxx
// Created by:  drose (08Aug02)
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

#include "thread.h"
#include "mainThread.h"

Thread *Thread::_main_thread;
TypeHandle Thread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Thread::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Thread::
~Thread() {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Thread::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::init_main_thread
//       Access: Private, Static
//  Description: Creates the Thread object that represents the main
//               thread.
////////////////////////////////////////////////////////////////////
void Thread::
init_main_thread() {
  if (_main_thread == (Thread *)NULL) {
    _main_thread = new MainThread;
    ((Thread *)_main_thread)->_started = true;
  }
}
