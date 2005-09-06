// Filename: asyncUtility.cxx
// Created by:  mike (09Jan97)
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

#include "config_downloader.h"
#include "asyncUtility.h"

#if defined(WIN32)
  #define WINDOWS_LEAN_AND_MEAN
  #include <wtypes.h>
  #undef WINDOWS_LEAN_AND_MEAN  
#else
  #include <sys/time.h>
#endif

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AsyncUtility::
AsyncUtility(float frequency) : _frequency(frequency) {
  _next_token = 1;
  _shutdown = false;
  _threaded = false;
  _threads_enabled = true;

#ifdef OLD_HAVE_IPC
  _request_cond = new condition_variable(_lock);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AsyncUtility::
~AsyncUtility() {
#ifdef OLD_HAVE_IPC
  delete _request_cond;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::create_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AsyncUtility::
create_thread() {
#ifdef OLD_HAVE_IPC
  if (_threaded == false && _threads_enabled == true) {
    downloader_cat.debug()
      << "AsyncUtility::create_thread()" << endl;
    _thread = thread::create(&st_callback, this, thread::PRIORITY_NORMAL);
    _threaded = true;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::destroy_thread
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void AsyncUtility::
destroy_thread() {
#ifdef OLD_HAVE_IPC
  if (_threaded == false)
    return;

  // Tell the thread to shut itself down.
  // We need to grab the lock in order to signal the condition variable
  _lock.lock();
    _shutdown = true;
    _request_cond->signal();
  _lock.unlock();

  // Join the loader thread - calling process blocks until the loader
  // thread returns.
  void *ret;
  _thread->join(&ret);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::st_callback
//       Access: Protected, Static
//  Description: This is a static wrapper around the callback()
//               method, below.  It's static just so we can pass it to
//               the thread-creation function.  In addition, the
//               function has a void* return type even though we
//               don't actually return anything.  This is necessary
//               because ipc assumes a function that does not return
//               anything indicates that the associated thread should
//               be created as unjoinable (detached).
////////////////////////////////////////////////////////////////////
void* AsyncUtility::
st_callback(void *arg) {
#ifdef OLD_HAVE_IPC
  nassertr(arg != NULL, NULL);
  ((AsyncUtility *)arg)->callback();
#endif
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::callback
//       Access: Protected
//  Description: This is the main body of the sub-thread.  It waits
//               forever for a request to show up, and then serves it.
////////////////////////////////////////////////////////////////////
void AsyncUtility::
callback() {
#ifdef OLD_HAVE_IPC
  while (process_request()) {
    // Sleep until a signal arrives
    _lock.lock();
      _request_cond->wait();
    _lock.unlock();
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::nap
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void AsyncUtility::
nap() const {
#ifdef OLD_HAVE_IPC
#ifdef WIN32
  _sleep((DWORD)(1000 * _frequency));
#else
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = (long)(1000000 * _frequency);
  select(0, NULL, NULL, NULL, &tv);
#endif
#endif  // OLD_HAVE_IPC
}
