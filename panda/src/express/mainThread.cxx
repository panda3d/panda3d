// Filename: mainThread.cxx
// Created by:  drose (15Jan06)
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

#include "mainThread.h"

TypeHandle MainThread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MainThread::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
MainThread::
MainThread() : Thread("main") {
}
 
////////////////////////////////////////////////////////////////////
//     Function: MainThread::thread_main
//       Access: Private, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MainThread::
thread_main() {
}
