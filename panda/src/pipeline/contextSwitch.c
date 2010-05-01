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

#if defined(THREAD_SIMPLE_IMPL) && !defined(CPPPARSER)

/* With OS_SIMPLE_THREADS, we will try to implement context-switching
   using OS-provided threading constructs.  This is via either Windows
   or Posix threads. */

#if defined(WIN32) && defined(OS_SIMPLE_THREADS)

#include "contextSwitch_windows_src.c"

#elif defined(HAVE_POSIX_THREADS) && defined(OS_SIMPLE_THREADS)

#include "contextSwitch_posix_src.c"

#elif defined(PHAVE_UCONTEXT_H)

/* Without OS_SIMPLE_THREADS, or without Windows or Posix threads
   libraries available, we have to implement context-switching
   entirely in user space.  First choice: the ucontext.h interface. */

#include "contextSwitch_ucontext_src.c"

#else

/* Second choice: old fashioned setjmp/longjmp, with some a priori
   hacks to make it switch stacks. */

#include "contextSwitch_longjmp_src.c"

#endif  /* PHAVE_UCONTEXT_H */

#endif  /* THREAD_SIMPLE_IMPL */
