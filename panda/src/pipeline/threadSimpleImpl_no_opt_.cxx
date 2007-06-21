// Filename: threadSimpleImpl_setup_context.cxx
// Created by:  drose (20Jun07)
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

#include "threadSimpleImpl.h"

#ifdef THREAD_SIMPLE_IMPL

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::setup_context_2
//       Access: Private, Static
//  Description: Continuation of setup_context().  We have this as a
//               separate method so we can ensure that the stack gets
//               set up with our this pointer properly.  It is defined
//               in a separate file so we can disable compiler
//               optimizations on this one method (gcc prefers to
//               disable optimizations globally on the command line,
//               not via pragmas).
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
setup_context_2(ThreadSimpleImpl *self) {
  ThreadSimpleImpl *volatile v_self = self;
  if (setjmp(self->_context) == 0) {
    // The _context is now set up and ready to run.  Now we can
    // return.
    return;
  }
   
  // Here we are executing within the thread.
  v_self->setup_context_3();

  // This code will never run, but we need to have some real code
  // here, to force the compiler to build an appropriate stack frame
  // for the above call (otherwise it might optimize the stack frame
  // away).
  *v_self = 0;
  abort();
}

#endif  // THREAD_SIMPLE_IMPL
