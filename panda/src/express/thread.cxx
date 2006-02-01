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
#include "externalThread.h"

Thread *Thread::_main_thread;
Thread *Thread::_external_thread;
TypeHandle Thread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Thread::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Thread::
~Thread() {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::bind_thread
//       Access: Published, Static
//  Description: Returns a new Panda Thread object associated with the
//               current thread (which has been created externally).
//               This can be used to bind a unique Panda Thread object
//               with an external thread, such as a new Python thread.
//
//               It is particularly useful to bind a Panda Thread
//               object to an external thread for the purposes of
//               PStats monitoring.  Without this call, each external
//               thread will be assigned the same global
//               ExternalThread object, which means they will all
//               appear in the same PStats graph.
//
//               It is the caller's responsibility to save the
//               returned Thread pointer for the lifetime of the
//               external thread.  It is an error for the Thread
//               pointer to destruct while the external thread is
//               still in the system.
//
//               It is also an error to call this method from the main
//               thread, or twice within a given thread, unless it is
//               given the same name each time (in which case the same
//               pointer will be returned each time).
////////////////////////////////////////////////////////////////////
PT(Thread) Thread::
bind_thread(const string &name, const string &sync_name) {
  Thread *current_thread = get_current_thread();
  if (current_thread != get_external_thread()) {
    // This thread already has an associated thread.
    nassertr(current_thread->get_name() == name && 
             current_thread->get_sync_name() == sync_name, current_thread);
    return current_thread;
  }

  PT(Thread) thread = new ExternalThread(name, sync_name);
  ThreadImpl::bind_thread(thread);
  return thread;
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::output
//       Access: Published, Virtual
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
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::init_external_thread
//       Access: Private, Static
//  Description: Creates the Thread object that represents all of the
//               external threads.
////////////////////////////////////////////////////////////////////
void Thread::
init_external_thread() {
  if (_external_thread == (Thread *)NULL) {
    _external_thread = new ExternalThread;
  }
}
