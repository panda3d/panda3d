// Filename: mainThread.cxx
// Created by:  drose (15Jan06)
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

#include "mainThread.h"

TypeHandle MainThread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MainThread::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
MainThread::
MainThread() : Thread("Main", "Main") {
  init_type();  // in case static init comes in the wrong order
  _impl.setup_main_thread();
  _started = true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: MainThread::thread_main
//       Access: Private, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MainThread::
thread_main() {
}
