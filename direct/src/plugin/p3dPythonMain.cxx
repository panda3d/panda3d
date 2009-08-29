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
using namespace std;

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef _WIN32
static const string dll_ext = ".dll";
#elif defined(__APPLE__)
static const string dll_ext = ".dylib";
#else
static const string dll_ext = ".so";
#endif

////////////////////////////////////////////////////////////////////
//     Function: main
//  Description: This is a trivial main() function that loads and runs
//               libp3dpython.dll.  It's used to build p3dpython.exe,
//               which is the preferred way to run Python in a child
//               process, as a separate executable.
////////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[]) {
  const char *program_name = argv[0];
  const char *dll_file = NULL;
  const char *archive_file = NULL;
  if (argc > 1) {
    dll_file = argv[1];
  }
  if (argc > 2) {
    archive_file = argv[2];
  }

  if (dll_file == NULL || *dll_file == '\0') {
    cerr << "No libp3dpython filename specified on command line.\n";
    return 1;
  }

  if (archive_file == NULL || *archive_file == '\0') {
    cerr << "No archive filename specified on command line.\n";
    return 1;
  }

  // For some vague idea of security, we insist that this program can
  // only run libp3dpython.dll: you can't use it to load just any
  // arbitrary DLL on the system.  Of course, if you're successfully
  // running this program in the first place, you probably don't need
  // any help to load an arbitrary DLL, but whatever.

  // Find the basename of the dll_file.
  const char *slash = strrchr(dll_file, '/');
#ifdef _WIN32
  const char *backslash = strrchr(dll_file, '\\');
  if (backslash != NULL && (slash == NULL || backslash > slash)) {
    slash = backslash;
  }
#endif
  if (slash == NULL) {
    slash = dll_file;
  } else {
    ++slash;
  }

  string expected_basename = "libp3dpython" + dll_ext;

  if (memcmp(slash, expected_basename.data(), expected_basename.size()) != 0) {
    cerr << dll_file << " does not name " << expected_basename << "\n";
    return 1;
  }

  // Everything checks out.  Load and run the library.

#ifdef _WIN32
  SetErrorMode(0);
  HMODULE module = LoadLibrary(dll_file);
  if (module == NULL) {
    // Couldn't load the DLL.
    cerr << "Couldn't load " << dll_file << "\n";
    return 1;
  }

  #define get_func GetProcAddress
  FHandle input = GetStdHandle(STD_INPUT_HANDLE);
  FHandle output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (!SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE)) {
    cerr << "unable to reset input handle\n";
  }
  if (!SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE)) {
    cerr << "unable to reset input handle\n";
  }

#else  // _WIN32
  // Posix case.
  void *module = dlopen(dll_file, RTLD_GLOBAL);
  if (module == NULL) {
    // Couldn't load the .so.
    cerr << "Couldn't load " << dll_file << "\n";
    return 1;
  }

  #define get_func dlsym
  FHandle input = STDIN_FILENO;
  FHandle output = STDOUT_FILENO;

#endif  // _WIN32

  run_p3dpython_func *run_p3dpython = (run_p3dpython_func *)get_func(module, "run_p3dpython");
  if (run_p3dpython == NULL) {
    cerr << "Couldn't find run_p3dpython\n";
    return 1;
  }

  if (!run_p3dpython(program_name, archive_file, input, output)) {
    cerr << "Failure on startup.\n";
    return 1;
  }
  return 0;
}
