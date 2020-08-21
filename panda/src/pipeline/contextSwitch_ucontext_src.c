/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file contextSwitch_ucontext_src.c
 * @author drose
 * @date 2010-04-15
 */

/* This is the implementation of user-space context switching using
   getcontext() / setcontext().  This is the preferred implementation,
   if these library functions are available; that's what they are
   designed for. */

#ifdef __APPLE__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

const int needs_stack_prealloc = 1;
const int is_os_threads = 0;

struct ThreadContext {
  ucontext_t _ucontext;
#ifdef __APPLE__
  // Due to a bug in OSX 10.5, the system ucontext_t declaration
  // doesn't reserve enough space, and we need to reserve some
  // additional space to make room.
#define EXTRA_PADDING_SIZE 4096
  char _extra_padding[EXTRA_PADDING_SIZE];
#endif
};

static void
begin_context(ThreadFunction *thread_func, void *data) {
  (*thread_func)(data);
}

void
init_thread_context(struct ThreadContext *context,
                    unsigned char *stack, size_t stack_size,
                    ThreadFunction *thread_func, void *data) {
  if (getcontext(&context->_ucontext) != 0) {
    fprintf(stderr, "getcontext failed in init_thread_context!\n");
    // Too bad for you.
    abort();
  }

  context->_ucontext.uc_stack.ss_sp = stack;
  context->_ucontext.uc_stack.ss_size = stack_size;
  context->_ucontext.uc_stack.ss_flags = 0;
  context->_ucontext.uc_link = NULL;

  makecontext(&context->_ucontext, (void (*)())&begin_context, 2, thread_func, data);
}

void
save_thread_context(struct ThreadContext *context,
                    ContextFunction *next_context, void *data) {
  /* getcontext requires us to use a volatile auto variable to
     differentiate between pass 1 (immediate return) and pass 2
     (return from setcontext). */
  volatile int context_return = 0;

  if (getcontext(&context->_ucontext) != 0) {
    fprintf(stderr, "getcontext failed!\n");
    // Nothing to do here.
    abort();
  }

  if (context_return) {
    /* We have just returned from setcontext.  In this case, return
       from the function.  The stack is still good. */
    return;
  }

  context_return = 1;

  /* We are still in the calling thread.  In this case, we cannot
     return from the function without damaging the stack.  Insted,
     call next_context() and trust the caller to call
     switch_to_thread_context() in there somewhere. */

  (*next_context)(context, data);

  /* We shouldn't get here. */
  abort();
}

void
switch_to_thread_context(struct ThreadContext *from_context,
                         struct ThreadContext *to_context) {
  setcontext(&to_context->_ucontext);

  /* Shouldn't get here. */
  abort();
}

struct ThreadContext *
alloc_thread_context() {
  struct ThreadContext *context =
    (struct ThreadContext *)malloc(sizeof(struct ThreadContext));
  memset(context, 0, sizeof(struct ThreadContext));

#if defined(__APPLE__) && defined(_DEBUG)
  {
    int p;
    // Pre-fill the extra_padding with bytes that we can recognize
    // later.
    for (p = 0; p < EXTRA_PADDING_SIZE; ++p) {
      context->_extra_padding[p] = (p & 0xff);
    }
  }
#endif  // __APPLE__

  return context;
}

void
free_thread_context(struct ThreadContext *context) {
#if defined(__APPLE__) && defined(_DEBUG)
  {
    // Because of the OSX 10.5 bug, we anticipate that the extra_padding
    // may have been filled in with junk.  Confirm this.
    int p = EXTRA_PADDING_SIZE;
    while (p > 0) {
      --p;
      if (context->_extra_padding[p] != (char)(p & 0xff)) {
        fprintf(stderr, "Context was mangled at byte %d: %d!\n", p, context->_extra_padding[p]);
        break;
      }
    }
  }
#endif  // __APPLE__
  free(context);
}
