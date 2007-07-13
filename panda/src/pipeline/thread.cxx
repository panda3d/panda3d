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
#include "config_pipeline.h"

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
#ifdef DEBUG_THREADS
  nassertv(_blocked_on_mutex == NULL);
#endif
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
//     Function: Thread::set_pipeline_stage
//       Access: Published
//  Description: Specifies the Pipeline stage number associated with
//               this thread.  The default stage is 0 if no stage is
//               specified otherwise.
//
//               This must be a value in the range [0
//               .. pipeline->get_num_stages() - 1].  It specifies the
//               values that this thread observes for all pipelined
//               data.  Typically, an application thread will leave
//               this at 0, but a render thread may set it to 1 or 2
//               (to operate on the previous frame's data, or the
//               second previous frame's data).
////////////////////////////////////////////////////////////////////
void Thread::
set_pipeline_stage(int pipeline_stage) {
#ifdef THREADED_PIPELINE
  _pipeline_stage = pipeline_stage;
#else
  if (pipeline_stage != 0) {
    pipeline_cat.warning()
      << "Requested pipeline stage " << pipeline_stage
      << " but multithreaded render pipelines not enabled in build.\n";
  }
  _pipeline_stage = 0;
#endif
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
//     Function: Thread::write_status
//       Access: Published, Static
//  Description:
////////////////////////////////////////////////////////////////////
void Thread::
write_status(ostream &out) {
#ifdef SIMPLE_THREADS
  ThreadImpl::write_status(out);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::start
//       Access: Public
//  Description: Starts the thread executing.  It is only valid to
//               call this once.
//
//               The thread will begin executing its thread_main()
//               function, and will terminate when thread_main()
//               returns.
//
//               priority is intended as a hint to the relative
//               importance of this thread.  This may be ignored by
//               the thread implementation.
//
//               joinable should be set true if you intend to call
//               join() to wait for the thread to terminate, or false
//               if you don't care and you will never call join().
//
//               The return value is true if the thread is
//               successfully started, false otherwise.
////////////////////////////////////////////////////////////////////
bool Thread::
start(ThreadPriority priority, bool joinable) {
  nassertr(!_started, false);

  if (!support_threads) {
    thread_cat.warning()
      << *this << " could not be started: support-threads is false.\n";
    return false;
  }

  _started = _impl.start(priority, joinable);

  if (!_started) {
    thread_cat.warning()
      << *this << " could not be started!\n";
  }

  return _started;
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::init_main_thread
//       Access: Private, Static
//  Description: Creates the Thread object that represents the main
//               thread.
////////////////////////////////////////////////////////////////////
void Thread::
init_main_thread() {
  // There is a chance of mutual recursion at startup.  The count
  // variable here attempts to protect against that.
  static int count = 0;
  ++count;
  if (count == 1 && _main_thread == (Thread *)NULL) {
    _main_thread = new MainThread;
    _main_thread->ref();
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
    _external_thread->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::Destructor
//       Access: Public, Virtual
//  Description: Since this class is just an interface definition,
//               there is no need to have a destructor.  However, we
//               must have one anyway to stop gcc's annoying warning.
////////////////////////////////////////////////////////////////////
Thread::PStatsCallback::
~PStatsCallback() {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::deactivate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is deactivated (swapped for
//               another running thread).  This is intended to provide
//               a callback hook for PStats to assign time to
//               individual threads properly, particularly in the
//               SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void Thread::PStatsCallback::
deactivate_hook(Thread *) {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::activate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is activated (resumes
//               execution).  This is intended to provide a callback
//               hook for PStats to assign time to individual threads
//               properly, particularly in the SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void Thread::PStatsCallback::
activate_hook(Thread *) {
}
