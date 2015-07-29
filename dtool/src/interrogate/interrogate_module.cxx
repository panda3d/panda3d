// Filename: interrogate_module.cxx
// Created by:  drose (08Aug00)
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

// This program generates a module-level file for interrogate.  This
// is a higher level than library, and groups several libraries
// together.  Presently, the only thing that goes into the module file
// is a python table, but who knows what the future holds.

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "load_dso.h"
#include "pystub.h"
#include "pnotify.h"
#include "panda_getopt_long.h"
#include "preprocess_argv.h"
#include "pset.h"
#include "vector_string.h"

Filename output_code_filename;
string module_name;
string library_name;
bool build_c_wrappers = false;
bool build_python_wrappers = false;
bool build_python_native_wrappers = false;
bool track_interpreter = false;
vector_string imports;

// Short command-line options.
static const char *short_options = "";

// Long command-line options.
enum CommandOptions {
  CO_oc = 256,
  CO_module,
  CO_library,
  CO_c,
  CO_python,
  CO_python_native,
  CO_track_interpreter,
  CO_import,
};

static struct option long_options[] = {
  { "oc", required_argument, NULL, CO_oc },
  { "module", required_argument, NULL, CO_module },
  { "library", required_argument, NULL, CO_library },
  { "c", no_argument, NULL, CO_c },
  { "python", no_argument, NULL, CO_python },
  { "python-native", no_argument, NULL, CO_python_native },
  { "track-interpreter", no_argument, NULL, CO_track_interpreter },
  { "import", required_argument, NULL, CO_import },
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

int write_python_table_native(ostream &out) {
  out << "\n#include \"dtoolbase.h\"\n"
      << "#include \"interrogate_request.h\"\n\n"
      << "#undef _POSIX_C_SOURCE\n"
      << "#include \"py_panda.h\"\n\n";

  int count = 0;

  pset<std::string> libraries;

//  out << "extern \"C\" {\n";

  // Walk through all of the Python functions.
  int num_functions = interrogate_number_of_functions();
  int fi;
  for (fi = 0; fi < num_functions; fi++) {
    FunctionIndex function_index = interrogate_get_function(fi);

    // Consider only those that belong in the module we asked for.
    //if (interrogate_function_has_module_name(function_index) &&
    //  module_name == interrogate_function_module_name(function_index)) {
      // if it has a library name add it to set of libraries
      if (interrogate_function_has_library_name(function_index)) {
        libraries.insert(interrogate_function_library_name(function_index));
      }
    //}
  }

  for (int ti = 0; ti < interrogate_number_of_types(); ti++) {
    TypeIndex thetype  = interrogate_get_type(ti);
    if (interrogate_type_has_module_name(thetype) && module_name == interrogate_type_module_name(thetype)) {
      if (interrogate_type_has_library_name(thetype)) {
        libraries.insert(interrogate_type_library_name(thetype));
      }
    }
  }

  pset<std::string >::iterator ii;
  for(ii = libraries.begin(); ii != libraries.end(); ii++) {
    printf("Referencing Library %s\n", (*ii).c_str());
    out << "extern LibraryDef " << *ii << "_moddef;\n";
    out << "extern void Dtool_" << *ii << "_RegisterTypes();\n";
    out << "extern void Dtool_" << *ii << "_ResolveExternals();\n";
    out << "extern void Dtool_" << *ii << "_BuildInstants(PyObject *module);\n";
  }

  out << "\n"
      << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef py_" << library_name << "_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << library_name << "\",\n"
      << "  NULL,\n"
      << "  -1,\n"
      << "  NULL,\n"
      << "  NULL, NULL, NULL, NULL\n"
      << "};\n"
      << "\n"
      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) PyObject *PyInit_" << library_name << "();\n"
      << "#elif __GNUC__ >= 4\n"
      << "extern \"C\" __attribute__((visibility(\"default\"))) PyObject *PyInit_" << library_name << "();\n"
      << "#else\n"
      << "extern \"C\" PyObject *PyInit_" << library_name << "();\n"
      << "#endif\n"
      << "\n"
      << "PyObject *PyInit_" << library_name << "() {\n";

  if (track_interpreter) {
    out << "  in_interpreter = 1;\n";
  }

  vector_string::const_iterator si;
  for (si = imports.begin(); si != imports.end(); ++si) {
    out << "  PyImport_Import(PyUnicode_FromString(\"" << *si << "\"));\n";
  }

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "  Dtool_" << *ii << "_RegisterTypes();\n";
  }
  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "  Dtool_" << *ii << "_ResolveExternals();\n";
  }
  out << "\n";

  out << "  LibraryDef *defs[] = {";
  for(ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "&" << *ii << "_moddef, ";
  }

  out << "NULL};\n"
      << "\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(defs, &py_" << library_name << "_module);\n"
      << "  if (module != NULL) {\n";

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "    Dtool_" << *ii << "_BuildInstants(module);\n";
  }

  out << "  }\n"
      << "  return module;\n"
      << "}\n"
      << "\n"
      << "#else  // Python 2 case\n"
      << "\n"
      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) void init" << library_name << "();\n"
      << "#elif __GNUC__ >= 4\n"
      << "extern \"C\" __attribute__((visibility(\"default\"))) void init" << library_name << "();\n"
      << "#else\n"
      << "extern \"C\" void init" << library_name << "();\n"
      << "#endif\n"
      << "\n"
      << "void init" << library_name << "() {\n";

  if (track_interpreter) {
    out << "  in_interpreter = 1;\n";
  }

  for (si = imports.begin(); si != imports.end(); ++si) {
    out << "  PyImport_Import(PyUnicode_FromString(\"" << *si << "\"));\n";
  }

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "  Dtool_" << *ii << "_RegisterTypes();\n";
  }
  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "  Dtool_" << *ii << "_ResolveExternals();\n";
  }
  out << "\n";

  out << "  LibraryDef *defs[] = {";
  for(ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "&" << *ii << "_moddef, ";
  }

  out << "NULL};\n"
      << "\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(defs, \"" << module_name << "\");\n"
      << "  if (module != NULL) {\n";

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "    Dtool_" << *ii << "_BuildInstants(module);\n";
  }

  out << "  }\n"
      << "}\n"
      << "#endif\n"
      << "\n";


  return count;
}

int write_python_table(ostream &out) {
  out << "\n#include \"dtoolbase.h\"\n"
      << "#include \"interrogate_request.h\"\n\n"
      << "#undef _POSIX_C_SOURCE\n"
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

      << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef python_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << library_name << "\",\n"
      << "  NULL,\n"
      << "  -1,\n"
      << "  python_methods,\n"
      << "  NULL, NULL, NULL, NULL\n"
      << "};\n\n"

      << "#define INIT_FUNC PyObject *PyInit_" << library_name << "\n"
      << "#else\n"
      << "#define INIT_FUNC void init" << library_name << "\n"
      << "#endif\n\n"

      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) INIT_FUNC();\n"
      << "#else\n"
      << "extern \"C\" INIT_FUNC();\n"
      << "#endif\n\n"

      << "INIT_FUNC() {\n";

  if (track_interpreter) {
    out << "  in_interpreter = 1;\n";
  }

  out << "#if PY_MAJOR_VERSION >= 3\n"
      << "  return PyModule_Create(&python_module);\n"
      << "#else\n"
      << "  Py_InitModule(\"" << library_name << "\", python_methods);\n"
      << "#endif\n"
      << "}\n\n";

  return count;
}

int main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  int flag;

  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  preprocess_argv(argc, argv);
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

    case CO_python_native:
      build_python_native_wrappers = true;
      break;

    case CO_track_interpreter:
      track_interpreter = true;
      break;

    case CO_import:
      imports.push_back(optarg);
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

  if (!build_c_wrappers && !build_python_wrappers && !build_python_native_wrappers) {
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
      void *dl = load_dso(DSearchPath(), pathname);
      if (dl == NULL) {
        nout << "Unable to load: " << load_dso_error() << "\n";
        exit(1);
      }
    }
  }

  // Now output the table.
  if (!output_code_filename.empty()) {
    pofstream output_code;

    if (!output_code_filename.open_write(output_code)) {
      nout << "Unable to write to " << output_code_filename << "\n";
    } else {

      if (build_python_wrappers) {
        int count = write_python_table(output_code);
        nout << count << " python function wrappers exported.\n";
      }

      if (build_python_native_wrappers) {
        write_python_table_native(output_code);
      }

    }
  }

  if (interrogate_error_flag()) {
    nout << "Error reading interrogate data.\n";
    output_code_filename.unlink();
    exit(1);
  }

  return (0);
}
