/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file contextSwitch_posix_src.c
 * @author drose
 * @date 2010-04-15
 */

/* This is the implementation of user-space context switching using
   posix threads to manage the different execution contexts.  This
   isn't strictly user-space, since we use OS threading constructs,
   but we use a global lock to ensure that only one thread at a time
   is active.  Thus, we still don't have to defend the code against
   critical sections globally, so we still get the low-overhead
   benefit of SIMPLE_THREADS; this is just a simple, reliable way to
   manage context switches. */

#include <pthread.h>
#include <setjmp.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* The posix threads implementation doesn't use the stack pointer. */
const int needs_stack_prealloc = 0;
const int is_os_threads = 1;

static struct ThreadContext *current_context = NULL;

struct ThreadContext {
  /* Each context is really its own thread. */
  pthread_t _thread;

  /* This condition variable and flag is set true when the thread is
     ready to roll. */
  pthread_mutex_t _ready_mutex;
  pthread_cond_t _ready_cvar;
  int _ready_flag;

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

static void *
thread_main(void *data) {
  struct ThreadContext *context = (struct ThreadContext *)data;

  /* Wait for the thread to be awoken. */
  pthread_mutex_lock(&context->_ready_mutex);
  while (!context->_ready_flag) {
    pthread_cond_wait(&context->_ready_cvar, &context->_ready_mutex);
  }
  context->_ready_flag = FALSE;
  pthread_mutex_unlock(&context->_ready_mutex);

  if (context->_terminated) {
    /* We've been rudely terminated.  Exit gracefully. */
    return NULL;
  }

  /* Now we can begin. */
  (*context->_thread_func)(context->_data);

  return NULL;
}

void
init_thread_context(struct ThreadContext *context,
                    unsigned char *stack, size_t stack_size,
                    ThreadFunction *thread_func, void *data) {
  context->_thread_func = thread_func;
  context->_data = data;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stack_size);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  pthread_create(&(context->_thread), &attr, thread_main, context);
  pthread_attr_destroy(&attr);
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
  pthread_mutex_lock(&to_context->_ready_mutex);
  to_context->_ready_flag = TRUE;
  pthread_cond_signal(&to_context->_ready_cvar);
  pthread_mutex_unlock(&to_context->_ready_mutex);

  /* And now put the from thread to sleep until it is again awoken. */
  pthread_mutex_lock(&from_context->_ready_mutex);
  while (!from_context->_ready_flag) {
    pthread_cond_wait(&from_context->_ready_cvar, &from_context->_ready_mutex);
  }
  from_context->_ready_flag = FALSE;
  pthread_mutex_unlock(&from_context->_ready_mutex);

  if (from_context->_terminated) {
    /* We've been rudely terminated.  Exit gracefully. */
    pthread_exit(NULL);
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
  context->_ready_flag = FALSE;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  // The symbol PTHREAD_MUTEX_DEFAULT isn't always available?
  //  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
  pthread_mutex_init(&context->_ready_mutex, &attr);
  pthread_mutexattr_destroy(&attr);

  pthread_cond_init(&context->_ready_cvar, NULL);

  return context;
}

void
free_thread_context(struct ThreadContext *context) {
  /* Make sure the thread wakes and exits gracefully. */
  context->_terminated = TRUE;

  pthread_mutex_lock(&context->_ready_mutex);
  context->_ready_flag = TRUE;
  pthread_cond_signal(&context->_ready_cvar);
  pthread_mutex_unlock(&context->_ready_mutex);

  void *result = NULL;
  pthread_join(context->_thread, &result);

  pthread_cond_destroy(&context->_ready_cvar);
  pthread_mutex_destroy(&context->_ready_mutex);

  free(context);
}
