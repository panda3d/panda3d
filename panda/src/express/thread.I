// Filename: thread.I
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


////////////////////////////////////////////////////////////////////
//     Function: Thread::Constructor
//       Access: Public
//  Description: Creates a new Thread object, but does not
//               immediately start executing it.  This gives the
//               caller a chance to store it in a PT(Thread) object,
//               if desired, before the thread gets a chance to
//               terminate and destruct itself.
//
//               Call start() to begin thread execution.
////////////////////////////////////////////////////////////////////
INLINE Thread::
Thread(const string &name) : _name(name), _impl(this) {
  _started = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy threads.
////////////////////////////////////////////////////////////////////
INLINE Thread::
Thread(const Thread &copy) : _impl(this) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy threads.
////////////////////////////////////////////////////////////////////
INLINE void Thread::
operator = (const Thread &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::get_name
//       Access: Public
//  Description: Returns the name of the thread.
////////////////////////////////////////////////////////////////////
INLINE const string &Thread::
get_name() const {
  return _name;
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
//               importance of this thread, and global should be set
//               true if the thread will perform a lot of blocking
//               I/O, or false otherwise (see the NSPR documentation
//               on global vs. local threads for more on this).  Both
//               of these parameters may be ignored by the thread
//               implementation.
//
//               joinable should be set true if you intend to call
//               join() to wait for the thread to terminate, or false
//               if you don't care and you will never call join().
//
//               The return value is true if the thread is
//               successfully started, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Thread::
start(ThreadPriority priority, bool global, bool joinable) {
  nassertr(!_started, false);
  _started = _impl.start(priority, global, joinable);
  return _started;
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::interrupt
//       Access: Public
//  Description: Sends an interrupt message to the thread.  This will
//               interrupt any blocking-type system calls the thread
//               may be waiting on, such as I/O, so that the thread
//               may continue some other processing.  The specific
//               behavior is implementation dependent.
////////////////////////////////////////////////////////////////////
INLINE void Thread::
interrupt() {
  _impl.interrupt();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::join
//       Access: Public
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
////////////////////////////////////////////////////////////////////
INLINE void Thread::
join() {
  _impl.join();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::prepare_for_exit
//       Access: Public
//  Description: Should be called by the main thread just before
//               exiting the program, this blocks until any remaining
//               thread cleanup has finished.
////////////////////////////////////////////////////////////////////
INLINE void Thread::
prepare_for_exit() {
  ThreadImpl::prepare_for_exit();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::get_current_thread
//       Access: Public, Static
//  Description: Returns a pointer to the currently-executing Thread
//               object, or NULL if the main thread (or some system
//               thread other than one started from the Panda
//               interface) is currently executing.
////////////////////////////////////////////////////////////////////
INLINE Thread *Thread::
get_current_thread() {
  return ThreadImpl::get_current_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::is_threading_supported
//       Access: Public, Static
//  Description: Returns true if a real threading library is available
//               that supports threads, or false if no threading
//               library is available (and Thread::start() will always
//               fail).
////////////////////////////////////////////////////////////////////
INLINE bool Thread::
is_threading_supported() {
  return ThreadImpl::is_threading_supported();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::sleep
//       Access: Public, Static
//  Description: Suspends the current thread for at least the
//               indicated amount of time.  It might be suspended for
//               longer.
////////////////////////////////////////////////////////////////////
INLINE void Thread::
sleep(double seconds) {
  ThreadImpl::sleep(seconds);
}

INLINE ostream &
operator << (ostream &out, const Thread &thread) {
  thread.output(out);
  return out;
}
