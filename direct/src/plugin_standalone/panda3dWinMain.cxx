// Filename: panda3dWinMain.cxx
// Created by:  drose (23Oct09)
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


#include "panda3d.h"

// On Windows, we may need to build panda3dw.exe, a non-console
// version of this program.

// Returns a newly-allocated string representing the quoted argument
// beginning at p.  Advances p to the first character following the
// close quote.
static char *
parse_quoted_arg(char *&p) {
  char quote = *p;
  ++p;
  string result;

  while (*p != '\0' && *p != quote) {
    // TODO: handle escape characters?  Not sure if we need to.
    result += *p;
    ++p;
  }
  if (*p == quote) {
    ++p;
  }
  return strdup(result.c_str());
}

// Returns a newly-allocated string representing the unquoted argument
// beginning at p.  Advances p to the first whitespace following the
// argument.
static char *
parse_unquoted_arg(char *&p) {
  string result;
  while (*p != '\0' && !isspace(*p)) {
    result += *p;
    ++p;
  }
  return strdup(result.c_str());
}

int WINAPI 
WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  char *command_line = GetCommandLine();

  vector<char *> argv;
  
  char *p = command_line;
  while (*p != '\0') {
    if (*p == '"') {
      char *arg = parse_quoted_arg(p);
      argv.push_back(arg);
    } else {
      char *arg = parse_unquoted_arg(p);
      argv.push_back(arg);
    }

    // Skip whitespace.
    while (*p != '\0' && isspace(*p)) {
      ++p;
    }
  }

  assert(!argv.empty());

  Panda3D program(false);
  return program.run_command_line(argv.size(), &argv[0]);
}
