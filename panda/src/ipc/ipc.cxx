// Filename: ipc.cxx
// Created by:  frang (06Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "ipc_condition.h"

base_condition_variable* const base_condition_variable::Null =
  (base_condition_variable*)0L;

#include "ipc_library.h"

base_library* const base_library::Null = (base_library*)0L;

#include "ipc_file.h"

base_file* const base_file::Null = (base_file*)0L;

#include "ipc_mutex.h"

base_mutex* const base_mutex::Null = (base_mutex*)0L;

#include "ipc_semaphore.h"

base_semaphore* const base_semaphore::Null = (base_semaphore*)0L;

#include "ipc_thread.h"

base_thread* const base_thread::Null = (base_thread*)0L;
base_thread::mutex* base_thread::_next_id_mutex = base_thread::mutex::Null;

int base_thread::_next_id = 0;

void* base_thread::thread_wrapper(void* data)
{
   base_thread* me = (base_thread *) data;

   me->_thread->start_in();

   // now invoke the thread functin with the given argument

   if (me->_fn_void != NULL)
   {
     (*me->_fn_void)(me->_thread_arg);
     base_thread::exit();
   }

   if (me->_fn_ret != NULL)
   {
     void* return_value = (*me->_fn_ret)(me->_thread_arg);
     base_thread::exit(return_value);
   }

   if (me->_detached)
   {
     me->run(me->_thread_arg);
     base_thread::exit();
   }
   else
   {
     void* return_value = me->run_undetached(me->_thread_arg);
     base_thread::exit(return_value);
   }

   // we should never get here, but this makes the compilers happy

   return (void *) NULL;
}

void base_thread::run(void*) {
}

void* base_thread::run_undetached(void*) {
  return (void*)0L;
}


