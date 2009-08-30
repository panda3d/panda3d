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
  const char *input_handle_str = NULL;
  const char *output_handle_str = NULL;
  const char *error_handle_str = NULL;
  const char *interactive_console_str = NULL;

  if (argc > 1) {
    dll_file = argv[1];
  }
  if (argc > 2) {
    archive_file = argv[2];
  }
  if (argc > 3) {
    input_handle_str = argv[3];
  }
  if (argc > 4) {
    output_handle_str = argv[4];
  }
  if (argc > 5) {
    error_handle_str = argv[5];
  }
  if (argc > 6) {
    interactive_console_str = argv[6];
  }

  if (dll_file == NULL || *dll_file == '\0') {
    cerr << "No libp3dpython filename specified on command line.\n";
    return 1;
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

  FHandle error_handle = invalid_fhandle;
  if (error_handle_str != NULL && *error_handle_str) {
    stringstream stream(error_handle_str);
    stream >> error_handle;
    if (!stream) {
      error_handle = invalid_fhandle;
    }
  }

  bool interactive_console = false;
  if (interactive_console_str != NULL && *interactive_console_str) {
    stringstream stream(interactive_console_str);
    int flag;
    stream >> flag;
    if (stream) {
      interactive_console = (flag != 0);
    }
  }

  cerr << "handles: " << input_handle << ", " << output_handle
       << ", " << error_handle << "\n";
  cerr << "interactive_console = " << interactive_console << "\n";

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
  string basename;
  if (slash == NULL) {
    basename = dll_file;
  } else {
    //dirname = string(dll_file, slash - dll_file);
    basename = (slash + 1);
  }

  string expected_basename = "libp3dpython" + dll_ext;
  if (basename != expected_basename) {
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

  // Get the default values for the communication handles, if we
  // weren't given specific handles.
  if (input_handle == invalid_fhandle) {
    input_handle = GetStdHandle(STD_INPUT_HANDLE);

    // Close the system input handle, so application code won't
    // accidentally read from our private input stream.
    if (!SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE)) {
      cerr << "unable to reset input handle\n";
    }
  }

  if (output_handle == invalid_fhandle) {
    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Close the system output handle, so application code won't
    // accidentally write to our private output stream.
    if (!SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE)) {
      cerr << "unable to reset input handle\n";
    }
  }

  // No matter what error handle we were given, make it
  // STD_ERROR_HANDLE.
  if (error_handle == invalid_fhandle) {
    error_handle = GetStdHandle(STD_ERROR_HANDLE);
  } else {
    SetStdHandle(STD_ERROR_HANDLE, error_handle);
  }
    
#else  // _WIN32
  // Posix case.
  void *module = dlopen(dll_file, RTLD_LOCAL);
  if (module == NULL) {
    // Couldn't load the .so.
    cerr << "Couldn't load " << dll_file << "\n";
    return 1;
  }

  #define get_func dlsym

  // Get the default values for the communication handles, if we
  // weren't given specific handles.
  if (input_handle == invalid_fhandle) {
    input_handle = STDIN_FILENO;
  }

  if (output_handle == invalid_fhandle) {
    output_handle = STDOUT_FILENO;
  }

  // No matter what error handle we were given, make it STDERR_FILENO.
  if (error_handle == invalid_fhandle) {
    error_handle = STDERR_FILENO;
  } else if (error_handle != STDERR_FILENO) {
    dup2(error_handle, STDERR_FILENO);
    close(error_handle);
    error_handle = STDERR_FILENO;
  }

#endif  // _WIN32

  run_p3dpython_func *run_p3dpython = (run_p3dpython_func *)get_func(module, "run_p3dpython");
  if (run_p3dpython == NULL) {
    cerr << "Couldn't find run_p3dpython\n";
    return 1;
  }

  if (!run_p3dpython(program_name, archive_file, input_handle, output_handle, 
                     error_handle, interactive_console)) {
    cerr << "Failure on startup.\n";
    return 1;
  }
  return 0;
}
