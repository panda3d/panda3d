/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda_getopt_impl.h
 * @author drose
 * @date 2011-07-19
 */

#ifndef PANDA_GETOPT_IMPL_H
#define PANDA_GETOPT_IMPL_H

#include "dtoolbase.h"

/* This file defines a reimplementation of getopt(), getopt_long(),
   and getopt_long_only(), according to the LSB and Posix conventions.
   It is completely new code, contributed under the Panda3D license. */

#if defined(HAVE_GETOPT) && defined(HAVE_GETOPT_LONG_ONLY)
// If the system provides both of these functions, we don't need to provide
// our own implementation, so in that case this file does nothing.

#else
// If the system does lack one or the other of these functions, then we'll go
// ahead and provide it instead.

#define getopt panda_getopt
#define optind panda_optind
#define opterr panda_opterr
#define optopt panda_optopt
#define optarg panda_optarg
#define getopt_long panda_getopt_long
#define getopt_long_only panda_getopt_long_only

#ifdef  __cplusplus
extern "C" {
#endif

extern EXPCL_DTOOL_DTOOLUTIL char *optarg;
extern EXPCL_DTOOL_DTOOLUTIL int optind, opterr, optopt;

struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

#define no_argument 0
#define required_argument 1
#define optional_argument 2

extern EXPCL_DTOOL_DTOOLUTIL int
getopt(int argc, char *const argv[], const char *optstring);
extern EXPCL_DTOOL_DTOOLUTIL int
getopt_long(int argc, char *const argv[], const char *optstring,
            const struct option *longopts, int *longindex);
extern EXPCL_DTOOL_DTOOLUTIL int
getopt_long_only(int argc, char *const argv[], const char *optstring,
                 const struct option *longopts, int *longindex);
extern EXPCL_DTOOL_DTOOLUTIL void
pgetopt_reset();

#ifdef  __cplusplus
}
#endif

#endif  // defined(HAVE_GETOPT) && defined(HAVE_GETOPT_LONG_ONLY)

#endif
