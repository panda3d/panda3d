/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file preprocess_argv.cxx
 * @author drose
 * @date 2011-11-08
 */

#include "preprocess_argv.h"
#include "win32ArgParser.h"

/**
 * Processes the argc, argv pair as needed before passing it to getopt().  If
 * this program is running on Windows, but not within Cygwin, this ignores the
 * incoming argv, argv values, replacing them from the GetCommandLine()
 * string, and expanding glob patterns like *.egg to a list of all matching
 * egg files.  On other platforms, this function does nothing and returns
 * argc, argv unchanged.
 *
 * The argc and argv values are modified by this function, if necessary, to
 * point to statically-allocated memory that will be valid until the next call
 * to preprocess_argv().
 */
void
preprocess_argv(int &argc, char **&argv) {
#ifndef _WIN32
  // Not Windows: do nothing.
  (void) argc;
  (void) argv;
#else  // _WIN32
  // Temporarily commenting out to fix build.  Revisit shortly.
  static Win32ArgParser parser;
  if (!parser.do_glob()) {
    // No globbing required.
    return;
  }

  // Globbing is required.  Process the args.
  parser.set_system_command_line();
  argc = parser.get_argc();
  argv = parser.get_argv();
#endif  // _WIN32
}
