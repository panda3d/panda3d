// Filename: interrogate_module.cxx
// Created by:  drose (08Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

// This program generates a module-level file for interrogate.  This
// is a higher level than library, and groups several libraries
// together.  Presently, the only thing that goes into the module file
// is a python table, but who knows what the future holds.

#include <interrogate_interface.h>
#include <interrogate_request.h>
#include <load_dso.h>
#include <pystub.h>
#include "notify.h"

// If our system getopt() doesn't come with getopt_long_only(), then use
// the GNU flavor that we've got in tool for this purpose.
#ifndef HAVE_GETOPT_LONG_ONLY
#include <gnu_getopt.h>
#else
#include <getopt.h>
#endif

Filename output_code_filename;
string module_name;
string library_name;
bool build_c_wrappers = false;
bool build_python_wrappers = false;
bool track_interpreter = false;

// Short command-line options.
static const char *short_options = "";

// Long command-line options.
enum CommandOptions {
  CO_oc = 256,
  CO_module,
  CO_library,
  CO_c,
  CO_python,
  CO_track_interpreter,
};

static struct option long_options[] = {
  { "oc", required_argument, NULL, CO_oc },
  { "module", required_argument, NULL, CO_module },
  { "library", required_argument, NULL, CO_library },
  { "c", no_argument, NULL, CO_c },
  { "python", no_argument, NULL, CO_python },
  { "track-interpreter", no_argument, NULL, CO_track_interpreter },
  { NULL }
};

/*
static string
upcase_string(const string &str) {
  string result;
  for (string::const_iterator si = str.begin();
       si != str.end();
       ++si) {
    result += toupper(*si);
  }
  return result;
}
*/

int
write_python_table(ostream &out) {
  out << "\n#include \"dtoolbase.h\"\n"
      << "#include \"interrogate_request.h\"\n\n"
      << "#include \"Python.h\"\n\n";

  int count = 0;

  // First, we have to declare extern C prototypes for each of the
  // function names.

  out << "extern \"C\" {\n";

  // Walk through all of the Python functions.
  int num_functions = interrogate_number_of_functions();
  int fi;
  for (fi = 0; fi < num_functions; fi++) {
    FunctionIndex function_index = interrogate_get_function(fi);

    // Consider only those that belong in the module we asked for.
    if (interrogate_function_has_module_name(function_index) &&
        module_name == interrogate_function_module_name(function_index)) {

      // For each function, get all of the python wrappers.
      int num_wrappers =
        interrogate_function_number_of_python_wrappers(function_index);

      for (int wi = 0; wi < num_wrappers; wi++) {
        FunctionWrapperIndex wrapper_index =
          interrogate_function_python_wrapper(function_index, wi);

        if (interrogate_wrapper_is_callable_by_name(wrapper_index)) {
          count++;
          const char *wrapper_name =
            interrogate_wrapper_name(wrapper_index);
          out << "  PyObject *" << wrapper_name
              << "(PyObject *self, PyObject *args);\n";
        }
      }
    }
  }

  out << "}\n\n";

  // Now go back through and build the table of function names.
  out << "static PyMethodDef python_methods[" << count + 1 << "] = {\n";

  // Walk through all of the Python functions.
  for (fi = 0; fi < num_functions; fi++) {
    FunctionIndex function_index = interrogate_get_function(fi);

    // Consider only those that belong in the module we asked for.
    if (interrogate_function_has_module_name(function_index) &&
        module_name == interrogate_function_module_name(function_index)) {

      // For each function, get all of the python wrappers.
      int num_wrappers =
        interrogate_function_number_of_python_wrappers(function_index);
      for (int wi = 0; wi < num_wrappers; wi++) {
        FunctionWrapperIndex wrapper_index =
          interrogate_function_python_wrapper(function_index, wi);

        if (interrogate_wrapper_is_callable_by_name(wrapper_index)) {
          const char *wrapper_name =
            interrogate_wrapper_name(wrapper_index);
          out << "  { \""
              << wrapper_name << "\", &"
              << wrapper_name << ", METH_VARARGS },\n";
        }
      }
    }
  }

  if (library_name.empty()) {
    library_name = module_name;
  }

  out << "  { NULL, NULL }\n"
      << "};\n\n"

      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) void init" << library_name << "();\n"
      << "#else\n"
      << "extern \"C\" void init" << library_name << "();\n"
      << "#endif\n\n"

      << "void init" << library_name << "() {\n";
  if (track_interpreter) {
    out << "  in_interpreter = 1;\n";
  }
  out << "  Py_InitModule(\"" << library_name << "\", python_methods);\n"
      << "}\n\n";

  return count;
}

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  int flag;

  flag = getopt_long_only(argc, argv, short_options, long_options, NULL);
  while (flag != EOF) {
    switch (flag) {
    case CO_oc:
      output_code_filename = optarg;
      break;

    case CO_module:
      module_name = optarg;
      break;

    case CO_library:
      library_name = optarg;
      break;

    case CO_c:
      build_c_wrappers = true;
      break;

    case CO_python:
      build_python_wrappers = true;
      break;

    case CO_track_interpreter:
      track_interpreter = true;
      break;

    default:
      exit(1);
    }
    flag = getopt_long_only(argc, argv, short_options, long_options, NULL);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    nout
      << "\nUsage:\n"
      << "  interrogate-module [opts] libname.in [libname.in ...]\n\n";
    exit(1);
  }

  output_code_filename.set_text();

  if (!build_c_wrappers && !build_python_wrappers) {
    build_c_wrappers = true;
  }

  for (int i = 1; i < argc; i++) {
    string param = argv[i];


    if (param.length() > 3 && param.substr(param.length() - 3) == ".in") {
      // If the filename ends in ".in", it's an interrogate database
      // file, not a shared library--read it directly.
      interrogate_request_database(param.c_str());

    } else {
      // Otherwise, assume it's a shared library, and try to load it.
      Filename pathname = argv[i];
      pathname.set_type(Filename::T_dso);
      nout << "Loading " << pathname << "\n";
      void *dl = load_dso(pathname);
      if (dl == NULL) {
        nout << "Unable to load: " << load_dso_error() << "\n";
        exit(1);
      }
    }
  }

  // Now output the table.
  if (!output_code_filename.empty()) {
    ofstream output_code;

    if (!output_code_filename.open_write(output_code)) {
      nout << "Unable to write to " << output_code_filename << "\n";
    } else {
      if (build_python_wrappers) {
        int count = write_python_table(output_code);
        nout << count << " python function wrappers exported.\n";
      }
    }
  }

  return (0);
}
