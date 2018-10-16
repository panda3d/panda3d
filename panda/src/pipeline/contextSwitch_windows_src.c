/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file contextSwitch_windows_src.c
 * @author drose
 * @date 2010-04-15
 */

/* This is the implementation of user-space context switching using
   native Windows threading constructs to manage the different
   execution contexts.  This isn't strictly user-space, since we use
   OS threading constructs, but we use a global lock to ensure that
   only one thread at a time is active.  Thus, we still don't have to
   defend the code against critical sections globally, so we still get
   the low-overhead benefit of SIMPLE_THREADS; this is just a simple,
   reliable way to manage context switches. */

#include <windows.h>
#include <setjmp.h>

/* The Windows implementation doesn't use the stack pointer. */
const int needs_stack_prealloc = 0;
const int is_os_threads = 1;

static struct ThreadContext *current_context = NULL;

struct ThreadContext {
  /* Each context is really its own thread. */
  HANDLE _thread;

  /* This event is in the signaled state when the thread is ready to
     roll. */
  HANDLE _ready;

  /* This is set FALSE while the thread is alive, and TRUE if the
     thread is to be terminated when it next wakes up. */
  int _terminated;

  /* These are preloaded with the startup parameters, then cleared to
     NULL for subsequent runs. */
  ThreadFunction *_thread_func;
  void *_data;

  /* We use setjmp()/longjmp() to manage the detail of returning from
     save_thread_context() when we call switch_to_thread_context(). */
  jmp_buf _jmp_context;
};

static DWORD WINAPI
thread_main(LPVOID data) {
  struct ThreadContext *context = (struct ThreadContext *)data;

  /* Wait for the thread to be awoken. */
  WaitForSingleObject(context->_ready, INFINITE);

  if (context->_terminated) {
    /* We've been rudely terminated.  Exit gracefully. */
    ExitThread(1);
  }

  /* Now we can begin. */
  (*context->_thread_func)(context->_data);

  return 0;
}

void
init_thread_context(struct ThreadContext *context,
                    unsigned char *stack, size_t stack_size,
                    ThreadFunction *thread_func, void *data) {
  context->_thread_func = thread_func;
  context->_data = data;

  context->_thread = CreateThread(NULL, stack_size,
                                  thread_main, context, 0, NULL);
}

void
save_thread_context(struct ThreadContext *context,
                    ContextFunction *next_context, void *data) {
  /* Save the current context so we can return here when the thread is
     awoken. */
  if (setjmp(context->_jmp_context) != 0) {
    /* We have just returned from longjmp.  In this case, return from
       the function. */
    return;
  }

  current_context = context;
  (*next_context)(context, data);

  /* Should not get here. */
  assert(FALSE);
  abort();
}

void
switch_to_thread_context(struct ThreadContext *from_context,
                         struct ThreadContext *to_context) {
  /* Pause the current thread, and switch to the indicated context.
     This function should not return. */
  assert(from_context == current_context);

  /* Wake up the target thread. */
  SetEvent(to_context->_ready);

  /* And now put the from thread to sleep until it is again awoken. */
  WaitForSingleObject(from_context->_ready, INFINITE);

  if (from_context->_terminated) {
    /* We've been rudely terminated.  Exit gracefully. */
    ExitThread(1);
  }

  /* Now we have been signaled again, and we're ready to resume the
     thread. */
  longjmp(from_context->_jmp_context, 1);

  /* Should not get here. */
  assert(FALSE);
  abort();
}

struct ThreadContext *
alloc_thread_context() {
  struct ThreadContext *context =
    (struct ThreadContext *)malloc(sizeof(struct ThreadContext));

  memset(context, 0, sizeof(struct ThreadContext));
  context->_ready = CreateEvent(NULL, FALSE, FALSE, NULL);

  return context;
}

void
free_thread_context(struct ThreadContext *context) {
  /* Make sure the thread wakes and exits gracefully. */
  context->_terminated = TRUE;
  SetEvent(context->_ready);
  WaitForSingleObject(context->_thread, INFINITE);

  CloseHandle(context->_ready);
  if (context->_thread != NULL) {
    CloseHandle(context->_thread);
  }

  free(context);
}
