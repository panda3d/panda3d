/* Filename: contextSwitch.h
 * Created by:  drose (21Jun07)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CONTEXTSWITCH_H
#define CONTEXTSWITCH_H

#include "pandabase.h"
#include "selectThreadImpl.h"

/* This file defines the code to perform fundamental context switches
   between different threads of execution within user space code.  It
   does this by saving and restoring the register state, including
   transferring to a new stack. */

/* The code in this file is all written in C code, rather than C++, to
   reduce possible conflicts from longjmp implementations that attempt
   to be smart with respect to C++ destructors and exception
   handling. */

#ifdef THREAD_SIMPLE_IMPL

struct ThreadContext;

#ifdef __cplusplus
extern "C" {
#endif 

typedef void ContextFunction(struct ThreadContext *from_context, void *);
typedef void ThreadFunction(void *);

extern const int needs_stack_prealloc;
extern const int is_os_threads;

/* Call this to fill in the appropriate values in context.  If
   needs_stack_prealloc (above) is true, the stack must already have
   been allocated; if needs_stack_prealloc is false, the stack pointer
   is not used and may be NULL.  The context will be initialized so
   that when switch_to_thread_context() is called, it will begin
   executing thread_func(data), which should not return.  This
   function will return normally. */
void init_thread_context(struct ThreadContext *context, 
                         unsigned char *stack, size_t stack_size,
                         ThreadFunction *thread_func, void *data);

/* Call this to save the current thread context.  This function does
   not return until switch_to_thread_context() is called.  Instead it
   immediately calls next_context(data), which should not return. */
void save_thread_context(struct ThreadContext *context,
                         ContextFunction *next_context, void *data);

/* Call this to resume executing a previously saved context.
   from_context must be the currently-executing context, and
   to_context is the context to resume.  When called, it will return
   from save_thread_context() in the saved stack (or begin executing
   thread_func()). */
void switch_to_thread_context(struct ThreadContext *from_context,
                              struct ThreadContext *to_context);

/* Use this pair of functions to transparently allocate and destroy an
   opaque ThreadContext object of the appropriate size.  These
   functions allocate memory, and initialize the context as
   appropriate for the main thread.  See init_main_context() to finish
   the initialization for a new thread. */
struct ThreadContext *alloc_thread_context();
void free_thread_context(struct ThreadContext *context);

#ifdef __cplusplus
}
#endif 

#endif  /* THREAD_SIMPLE_IMPL */

#endif  /* CONTEXTSWITCH_H */

