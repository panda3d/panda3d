/* Filename: contextSwitch.h
 * Created by:  drose (21Jun07)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://etc.cmu.edu/panda3d/docs/license/ .
 *
 * To contact the maintainers of this program write to
 * panda3d-general@lists.sourceforge.net .
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

#if defined(HAVE_UCONTEXT_H)
/* We'd prefer to use getcontext() / setcontext() to portably change
   execution contexts within C code.  That's what these library
   functions are designed for. */
#include <ucontext.h>

struct ThreadContext {
  ucontext_t _ucontext;
};

#else  /* HAVE_UCONTEXT_H */
/* Unfortunately, setcontext() is not defined everywhere (even though
   it claims to be adopted by Posix).  So we have to fall back to
   setjmp() / longjmp() in its absence.  This is a hackier solution. */

#if defined(__i386__)
/* Maybe we can implement our own setjmp/longjmp in assembly code.
   This will be safe than the system version, since who knows what
   that one's really doing? */

typedef int cs_jmp_buf[33];

#define CS_JB_SP 4

#else

/* Fall back to the system implmentation of setjmp/longjmp. */
#include <setjmp.h>

typedef jmp_buf cs_jmp_buf;
#define cs_setjmp setjmp
#define cs_longjmp(buf) longjmp(buf, 1)

#ifdef JB_SP
#define CS_JB_SP JB_SP
#endif

#endif  /* __i386__ */

struct ThreadContext {
  cs_jmp_buf _jmp_context;
};

#endif  /* HAVE_UCONTEXT_H */

#ifdef __cplusplus
extern "C" {
#endif 

typedef void ContextFunction(void *);

/* Call this to fill in the appropriate values in context.  The stack
   must already have been allocated.  The context will be initialized
   so that when switch_to_thread_context() is called, it will begin
   executing thread_func(data), which should not return.  This function
   will return normally. */
void init_thread_context(struct ThreadContext *context, 
                         unsigned char *stack, size_t stack_size,
                         ContextFunction *thread_func, void *data);

/* Call this to save the current thread context.  This function does
   not return until switch_to_thread_context() is called.  Instead it
   immediately calls next_context(data), which should not return. */
void save_thread_context(struct ThreadContext *context,
                         ContextFunction *next_context, void *data);

/* Call this to resume executing a previously saved context.  When
   called, it will return from save_thread_context() in the saved
   stack (or begin executing thread_func()). */
void switch_to_thread_context(struct ThreadContext *context);

#ifdef __cplusplus
}
#endif 

#endif  /* THREAD_SIMPLE_IMPL */

#endif  /* CONTEXTSWITCH_H */

