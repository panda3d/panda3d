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

#ifdef WIN32
/* Define this macro to use native Windows threading constructs to
   switch contexts. */
#define WIN_THREAD_CONTEXT
#endif

#if defined(WIN_THREAD_CONTEXT)

#include "contextSwitch_windows_src.c"

#elif defined(PHAVE_UCONTEXT_H)

#include "contextSwitch_ucontext_src.c"

#else

#include "contextSwitch_longjmp_src.c"

#endif  /* PHAVE_UCONTEXT_H */

#endif  /* THREAD_SIMPLE_IMPL */
