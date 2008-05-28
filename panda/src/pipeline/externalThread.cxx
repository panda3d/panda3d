// Filename: externalThread.cxx
// Created by:  drose (30Jan06)
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

#include "externalThread.h"

TypeHandle ExternalThread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ExternalThread::Constructor
//       Access: Private
//  Description: This constructor is used to create the one global
//               ExternalThread object that is shared by all
//               externally-created threads that are not specifically
//               bound to a particular Thread object.
////////////////////////////////////////////////////////////////////
ExternalThread::
ExternalThread() : Thread("External", "External") {
  init_type();  // in case static init comes in the wrong order
  _started = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ExternalThread::Constructor
//       Access: Private
//  Description: This constructor is used to create the particular
//               Thread object for each external thread that is bound
//               via Thread::bind_thread().
////////////////////////////////////////////////////////////////////
ExternalThread::
ExternalThread(const string &name, const string &sync_name) : 
  Thread(name, sync_name)
{
  _started = true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: ExternalThread::thread_main
//       Access: Private, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ExternalThread::
thread_main() {
}
