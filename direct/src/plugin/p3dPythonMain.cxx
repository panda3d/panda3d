// Filename: p3dPythonMain.cxx
// Created by:  drose (29Aug09)
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

#include "run_p3dpython.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <string.h>  // strrchr
using namespace std;

#if defined(_WIN32) && defined(NON_CONSOLE)
// On Windows, we may need to build p3dpythonw.exe, a non-console
// version of this program.

// We'll wrap the main() function with our own startup WinMain().
#define main local_main
int main(int argc, char *argv[]);

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
  return main(argv.size(), &argv[0]);
}
#endif  // NON_CONSOLE

////////////////////////////////////////////////////////////////////
//     Function: main
//  Description: This is a trivial main() function that invokes
//               P3DPythonRun.  It's used to build p3dpython.exe,
//               which is the preferred way to run Python in a child
//               process, as a separate executable.
////////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[]) {
  const char *program_name = argv[0];
  const char *archive_file = NULL;
  const char *input_handle_str = NULL;
  const char *output_handle_str = NULL;
  const char *interactive_console_str = NULL;

  if (argc > 1) {
    archive_file = argv[1];
  }
  if (argc > 2) {
    input_handle_str = argv[2];
  }
  if (argc > 3) {
    output_handle_str = argv[3];
  }
  if (argc > 4) {
    interactive_console_str = argv[4];
  }

  if (archive_file == NULL || *archive_file == '\0') {
    cerr << "No archive filename specified on command line.\n";
    return 1;
  }

  FHandle input_handle = invalid_fhandle;
  if (input_handle_str != NULL && *input_handle_str) {
    stringstream stream(input_handle_str);
    stream >> input_handle;
    if (!stream) {
      input_handle = invalid_fhandle;
    }
  }

  FHandle output_handle = invalid_fhandle;
  if (output_handle_str != NULL && *output_handle_str) {
    stringstream stream(output_handle_str);
    stream >> output_handle;
    if (!stream) {
      output_handle = invalid_fhandle;
    }
  }

  bool interactive_console = false;
  if (interactive_console_str != NULL && *interactive_console_str) {
    stringstream stream(interactive_console_str);
    int flag;
    stream >> flag;
    if (!stream.fail()) {
      interactive_console = (flag != 0);
    }
  }

  if (!run_p3dpython(program_name, archive_file, input_handle, output_handle, 
                     NULL, interactive_console)) {
    cerr << "Failure on startup.\n";
    return 1;
  }
  return 0;
}
