// Filename: win32ArgParser.h
// Created by:  drose (08Nov11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef WIN32ARGPARSER_H
#define WIN32ARGPARSER_H

#include "dtoolbase.h"

#ifdef _WIN32

#include "vector_string.h"
#include "pvector.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////
//       Class : Win32ArgParser
// Description : This class is used to parse the single command-line
//               string provided by Windows into the standard argc,
//               argv array of strings.  In this way it duplicates the
//               functionality of Windows' own CommandLineToArgv()
//               function, but it is also supports automatic expansion
//               of glob filenames, e.g. *.egg is turned into an
//               explicit list of egg files in the directory.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL Win32ArgParser {
public:
  Win32ArgParser();
  ~Win32ArgParser();

  void clear();

  void set_command_line(const string &command_line);
  void set_command_line(const wstring &command_line);
  void set_system_command_line();

  char **get_argv();
  int get_argc();

  static bool do_glob();

private:
  string parse_quoted_arg(const char *&p);
  void parse_unquoted_arg(const char *&p);
  void save_arg(const string &arg);

  typedef vector_string Args;
  Args _args;

  char **_argv;
  int _argc;
};

#endif  // _WIN32

#endif
