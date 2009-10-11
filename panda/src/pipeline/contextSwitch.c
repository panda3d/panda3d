/* Filename: contextSwitch.c
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

#include "contextSwitch.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef THREAD_SIMPLE_IMPL

#if defined(PHAVE_UCONTEXT_H)

#else  /* PHAVE_UCONTEXT_H */

#endif  /* PHAVE_UCONTEXT_H */

#if defined(PHAVE_UCONTEXT_H)

/* We'd prefer to use getcontext() / setcontext() to portably change
   execution contexts within C code.  That's what these library
   functions are designed for. */
#include <ucontext.h>

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
begin_context(ContextFunction *thread_func, void *data) {
  (*thread_func)(data);
}

void
init_thread_context(struct ThreadContext *context, 
                    unsigned char *stack, size_t stack_size,
                    ContextFunction *thread_func, void *data) {
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

  (*next_context)(data);

  /* We shouldn't get here. */
  abort();
}

void
switch_to_thread_context(struct ThreadContext *context) {
  setcontext(&context->_ucontext);

  /* Shouldn't get here. */
  abort();
}

#else

/* Unfortunately, setcontext() is not defined everywhere (even though
   it claims to be adopted by Posix).  So we have to fall back to
   setjmp() / longjmp() in its absence.  This is a hackier solution. */

#if defined(_M_IX86) || defined(__i386__)
/* Maybe we can implement our own setjmp/longjmp in assembly code.
   This will be safer than the system version, since who knows what
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

#elif defined(__ppc__)
  /* This was determined experimentally through test_setjmp. */
#define CS_JB_SP 0

#endif

#endif  /* __i386__ */

struct ThreadContext {
  cs_jmp_buf _jmp_context;
};

/* The approach is: hack our way onto the new stack pointer right now,
   then call setjmp() to record that stack pointer in the
   _jmp_context.  Then restore back to the original stack pointer. */

#if defined(_M_IX86)
/* Here is our own implementation of setjmp and longjmp for I386, via
   Windows syntax. */

/* warning C4731: frame pointer register 'ebp' modified by inline assembly code */
#pragma warning(disable:4731)

int
cs_setjmp(cs_jmp_buf env) {
  __asm {
    pop ebp;  /* Restore the frame pointer that the compiler pushed */

    pop edx;  /* edx = return address */
    pop eax;  /* eax = &env */
    push eax; /* keep &env on the stack; the caller will remove it */

    mov [eax + 0], ebx;
    mov [eax + 4], edi;
    mov [eax + 8], esi;
    mov [eax + 12], ebp;
    mov [eax + 16], esp;
    mov [eax + 20], edx;

    fnsave [eax + 24];  /* save floating-point state */

    xor eax,eax;  /* return 0: pass 1 return */
    jmp edx;      /* this works like ret */
  }
}

void
cs_longjmp(cs_jmp_buf env) {
  _asm {
    mov eax, env;
    
    mov ebx, [eax + 0];
    mov edi, [eax + 4];
    mov esi, [eax + 8];
    mov ebp, [eax + 12];
    mov esp, [eax + 16];
    mov edx, [eax + 20];
    
    frstor [eax + 24];  /* restore floating-point state */

    mov eax, 1;   /* return 1 from setjmp: pass 2 return */
    jmp edx;      /* return from above setjmp call */
  }
}


#elif defined(__i386__)
/* Here is our own implementation of setjmp and longjmp for I386, via
   GNU syntax. */

#if defined(IS_LINUX)
/* On Linux, the leading underscores are not implicitly added for C
   function names. */
#define cs_setjmp _cs_setjmp
#define cs_longjmp _cs_longjmp
#endif

int cs_setjmp(cs_jmp_buf env);
void cs_longjmp(cs_jmp_buf env);

__asm__
("_cs_setjmp:\n"
 "popl %edx\n"
 "popl %eax\n"
 "pushl %eax\n"

 "movl %ebx, 0(%eax)\n"
 "movl %edi, 4(%eax)\n"
 "movl %esi, 8(%eax)\n"
 "movl %ebp, 12(%eax)\n"
 "movl %esp, 16(%eax)\n"
 "movl %edx, 20(%eax)\n"

 "fnsave 24(%eax)\n"

 "xorl %eax, %eax\n"
 "jmp *%edx\n");

__asm__
("_cs_longjmp:\n"
 "popl %edx\n"
 "popl %eax\n"

 "movl 0(%eax), %ebx\n"
 "movl 4(%eax), %edi\n"
 "movl 8(%eax), %esi\n"
 "movl 12(%eax), %ebp\n"
 "movl 16(%eax), %esp\n"
 "movl 20(%eax), %edx\n"

 "frstor 24(%eax)\n"

 "mov $1,%eax\n"
 "jmp *%edx\n");

#endif  /* __i386__ */

/* Ideally, including setjmp.h would have defined JB_SP, which will
   tell us where in the context structure we can muck with the stack
   pointer.  If it didn't define this symbol, we have to guess it. */
#ifndef CS_JB_SP

#if defined(IS_OSX) && defined(__i386__)
/* We have determined this value empirically, via test_setjmp.cxx in
   this directory. */
#define CS_JB_SP 9

#endif

#endif  /* CS_JB_SP */

static struct ThreadContext *st_context;
static unsigned char *st_stack;
static size_t st_stack_size;
static ContextFunction *st_thread_func;
static void *st_data;

static cs_jmp_buf orig_stack;

/* We can't declare this function static--gcc might want to inline it
   in that case, and then the code crashes.  I hope this doesn't mean
   that the stack is still not getting restored correctly in the above
   assembly code. */
void
setup_context_2(void) {
  /* Here we are running on the new stack.  Copy the key data onto our
     new stack. */
  ContextFunction *volatile thread_func = st_thread_func;
  void *volatile data = st_data;

  if (cs_setjmp(st_context->_jmp_context) == 0) {
    /* The _jmp_context is set up and ready to run.  Now restore the
       original stack and return.  We can't simply return from this
       function, since it might overwrite some of the stack data on
       the way out. */
    cs_longjmp(orig_stack);

    /* Shouldn't get here. */
    abort();
  }

  /* We come here the first time the thread starts. */
  (*thread_func)(data);

  /* We shouldn't get here, since we don't expect the thread_func to
     return. */
  abort();
}

static void
setup_context_1(void) {
  /* Save the current stack frame so we can return to it (at the end
     of setup_context_2()). */
  if (cs_setjmp(orig_stack) == 0) {
    /* First, switch to the new stack.  Save the current context using
       setjmp().  This saves out all of the processor register values,
       though it doesn't muck with the stack. */
    static cs_jmp_buf temp;
    if (cs_setjmp(temp) == 0) {
      /* This is the initial return from setjmp.  Still the original
         stack. */

      /* Now we overwrite the stack pointer value in the saved
         register context.  This doesn't work with all implementations
         of setjmp/longjmp. */

      /* We give ourselves a small buffer of unused space at the top
         of the stack, to allow for the stack frame and such that this
         code might be assuming is there. */
      (*(void **)&temp[CS_JB_SP]) = (st_stack + st_stack_size - 0x100);

      /* And finally, we place ourselves on the new stack by using
         longjmp() to reload the modified context. */
      cs_longjmp(temp);

      /* Shouldn't get here. */
      abort();
    }

    /* This is the second return from setjmp.  Now we're on the new
       stack. */
    setup_context_2();

    /* Shouldn't get here. */
    abort();
  }

  /* By now we are back to the original stack. */
}

void
init_thread_context(struct ThreadContext *context, 
                    unsigned char *stack, size_t stack_size,
                    ContextFunction *thread_func, void *data) {
  /* Copy all of the input parameters to static variables, then begin
     the stack-switching process. */
  st_context = context;
  st_stack = stack;
  st_stack_size = stack_size;
  st_thread_func = thread_func;
  st_data = data;

  setup_context_1();
}  

void
save_thread_context(struct ThreadContext *context,
                    ContextFunction *next_context, void *data) {
  if (cs_setjmp(context->_jmp_context) != 0) {
    /* We have just returned from longjmp.  In this case, return from
       the function.  The stack is still good. */
    return;
  }

  /* We are still in the calling thread.  In this case, we cannot
     return from the function without damaging the stack.  Insted,
     call next_context() and trust the caller to call
     switch_to_thread_context() in there somewhere. */

  (*next_context)(data);

  /* We shouldn't get here. */
  abort();
}

void
switch_to_thread_context(struct ThreadContext *context) {
  cs_longjmp(context->_jmp_context);

  /* Shouldn't get here. */
  abort();
}

#endif  /* PHAVE_UCONTEXT_H */

struct ThreadContext *
alloc_thread_context() {
  struct ThreadContext *context =
    (struct ThreadContext *)malloc(sizeof(struct ThreadContext));

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

#endif  /* THREAD_SIMPLE_IMPL */
