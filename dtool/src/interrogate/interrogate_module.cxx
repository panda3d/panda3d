/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate_module.cxx
 * @author drose
 * @date 2000-08-08
 */

// This program generates a module-level file for interrogate.  This is a
// higher level than library, and groups several libraries together.
// Presently, the only thing that goes into the module file is a python table,
// but who knows what the future holds.

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "load_dso.h"
#include "pnotify.h"
#include "panda_getopt_long.h"
#include "preprocess_argv.h"
#include "vector_string.h"

#include <algorithm>

using std::cerr;
using std::string;

// This contains a big source string determined at compile time.
extern const char interrogate_preamble_python_native[];

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
  { "oc", required_argument, nullptr, CO_oc },
  { "module", required_argument, nullptr, CO_module },
  { "library", required_argument, nullptr, CO_library },
  { "c", no_argument, nullptr, CO_c },
  { "python", no_argument, nullptr, CO_python },
  { "python-native", no_argument, nullptr, CO_python_native },
  { "track-interpreter", no_argument, nullptr, CO_track_interpreter },
  { "import", required_argument, nullptr, CO_import },
  { nullptr }
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

/**
 * Finds a dependency cycle between the given dependency mapping, starting at
 * the node that is already placed in the given cycle vector.
 */
static bool find_dependency_cycle(vector_string &cycle, std::map<string, std::set<string> > &dependencies) {
  assert(!cycle.empty());

  const std::set<string> &deps = dependencies[cycle.back()];
  for (auto it = deps.begin(); it != deps.end(); ++it) {
    auto it2 = std::find(cycle.begin(), cycle.end(), *it);
    if (it2 != cycle.end()) {
      // Chop off the part of the chain that is not relevant.
      cycle.erase(cycle.begin(), it2);
      cycle.push_back(*it);
      return true;
    }

    // Recurse.
    cycle.push_back(*it);
    if (find_dependency_cycle(cycle, dependencies)) {
      return true;
    }
    cycle.pop_back();
  }

  return false;
}

/**
 * Given that a direct link has been established between the two libraries,
 * finds the two types that make up this relationship and prints out the
 * nature of their dependency.
 */
static bool print_dependent_types(const string &lib1, const string &lib2) {
  for (int ti = 0; ti < interrogate_number_of_global_types(); ti++) {
    TypeIndex thetype  = interrogate_get_global_type(ti);
    if (interrogate_type_has_module_name(thetype) &&
        interrogate_type_has_library_name(thetype) &&
        lib1 == interrogate_type_library_name(thetype) &&
        module_name == interrogate_type_module_name(thetype)) {

      // Get the dependencies for this library.
      int num_derivations = interrogate_type_number_of_derivations(thetype);
      for (int di = 0; di < num_derivations; ++di) {
        TypeIndex basetype = interrogate_type_get_derivation(thetype, di);
        if (interrogate_type_is_global(basetype) &&
            interrogate_type_has_library_name(basetype) &&
            interrogate_type_library_name(basetype) == lib2) {
          cerr
            << "  " << interrogate_type_scoped_name(thetype) << " ("
            << lib1 << ") inherits from "
            << interrogate_type_scoped_name(basetype) << " (" << lib2 << ")\n";
          return true;
        }
      }

      // It also counts if this is a typedef pointing to another type.
      if (interrogate_type_is_typedef(thetype)) {
        TypeIndex wrapped = interrogate_type_wrapped_type(thetype);
        if (interrogate_type_is_global(wrapped) &&
            interrogate_type_has_library_name(wrapped) &&
            interrogate_type_library_name(wrapped) == lib2) {
          cerr
            << "  " << interrogate_type_scoped_name(thetype) << " ("
            << lib1 << ") is a typedef to "
            << interrogate_type_scoped_name(wrapped) << " (" << lib2 << ")\n";
        }
      }
    }
  }
  return false;
}

int write_python_table_native(std::ostream &out) {
  out << "\n#include \"dtoolbase.h\"\n"
      << "#include \"interrogate_request.h\"\n\n"
      << "#include \"py_panda.h\"\n\n";

  int count = 0;

  std::map<string, std::set<string> > dependencies;

// out << "extern \"C\" {\n";

  // Walk through all of the Python functions.
  int num_functions = interrogate_number_of_functions();
  int fi;
  for (fi = 0; fi < num_functions; fi++) {
    FunctionIndex function_index = interrogate_get_function(fi);

    // Consider only those that belong in the module we asked for.  if
    // (interrogate_function_has_module_name(function_index) && module_name ==
    // interrogate_function_module_name(function_index)) { if it has a library
    // name add it to set of libraries
      if (interrogate_function_has_library_name(function_index)) {
        string library_name = interrogate_function_library_name(function_index);
        dependencies[library_name];
      }
    // }
  }

  for (int ti = 0; ti < interrogate_number_of_global_types(); ti++) {
    TypeIndex thetype  = interrogate_get_global_type(ti);
    if (interrogate_type_has_module_name(thetype) && module_name == interrogate_type_module_name(thetype)) {
      if (interrogate_type_has_library_name(thetype)) {
        string library_name = interrogate_type_library_name(thetype);
        std::set<string> &deps = dependencies[library_name];

        // Get the dependencies for this library.
        int num_derivations = interrogate_type_number_of_derivations(thetype);
        for (int di = 0; di < num_derivations; ++di) {
          TypeIndex basetype = interrogate_type_get_derivation(thetype, di);
          if (interrogate_type_is_global(basetype) &&
              interrogate_type_has_library_name(basetype)) {
            string baselib = interrogate_type_library_name(basetype);
            if (baselib != library_name) {
              deps.insert(std::move(baselib));
            }
          }
        }

        if (interrogate_type_is_typedef(thetype)) {
          TypeIndex wrapped = interrogate_type_wrapped_type(thetype);
          if (interrogate_type_is_global(wrapped) &&
              interrogate_type_has_library_name(wrapped)) {
            string wrappedlib = interrogate_type_library_name(wrapped);
            if (wrappedlib != library_name) {
              deps.insert(std::move(wrappedlib));
            }
          }
        }
      }
    }
  }

  // Now add the libraries in their proper ordering, based on dependencies.
  vector_string libraries;
  while (libraries.size() < dependencies.size()) {
    // We have this check to make sure we don't enter an infinite loop.
    bool added_any = false;

    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
      const string &library_name = it->first;
      std::set<string> &deps = dependencies[library_name];

      // Remove the dependencies that have already been added from the deps.
      if (!deps.empty()) {
        for (auto li = libraries.begin(); li != libraries.end(); ++li) {
          deps.erase(*li);
        }
      }

      if (deps.empty()) {
        // OK, no remaining dependencies, so we can add this.
        if (std::find(libraries.begin(), libraries.end(), library_name) == libraries.end()) {
          libraries.push_back(library_name);
          added_any = true;
        }
      }
    }

    if (!added_any) {
      // Oh dear, we must have hit a circular dependency.  Go through the
      // remaining libraries to figure it out and print it.
      cerr << "Circular dependency between libraries detected:\n";
      for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
        const string &library_name = it->first;
        std::set<string> &deps = dependencies[library_name];
        if (deps.empty()) {
          continue;
        }

        // But since it does indicate a potential architectural flaw, we do
        // want to let the user know about this.
        vector_string cycle;
        cycle.push_back(library_name);
        if (!find_dependency_cycle(cycle, dependencies)) {
          continue;
        }
        assert(cycle.size() >= 2);

        // Show the cycle of library dependencies.
        auto ci = cycle.begin();
        cerr << "  " << *ci;
        for (++ci; ci != cycle.end(); ++ci) {
          cerr << " -> " << *ci;
        }
        cerr << "\n";

        // Now print out the actual types that make up the cycle.
        ci = cycle.begin();
        string prev = *ci;
        for (++ci; ci != cycle.end(); ++ci) {
          print_dependent_types(prev, *ci);
          prev = *ci;
        }

        // We have to arbitrarily break one of the dependencies in order to be
        // able to proceed.  Break the first dependency.
        dependencies[cycle[0]].erase(cycle[1]);
      }
    }
  }

  vector_string::const_iterator ii;
  for (ii = libraries.begin(); ii != libraries.end(); ++ii) {
    printf("Referencing Library %s\n", (*ii).c_str());
    out << "extern const struct LibraryDef " << *ii << "_moddef;\n";
    out << "extern void Dtool_" << *ii << "_RegisterTypes();\n";
    out << "extern void Dtool_" << *ii << "_BuildInstants(PyObject *module);\n";
  }

  out.put('\n');

  out << "#if PY_MAJOR_VERSION >= 3\n"
      << "extern \"C\" EXPORT_CLASS PyObject *PyInit_" << library_name << "();\n"
      << "#else\n"
      << "extern \"C\" EXPORT_CLASS void init" << library_name << "();\n"
      << "#endif\n";

  out << "\n"
      << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef py_" << library_name << "_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << library_name << "\",\n"
      << "  nullptr,\n"
      << "  -1,\n"
      << "  nullptr,\n"
      << "  nullptr, nullptr, nullptr, nullptr\n"
      << "};\n"
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
  out << "\n";

  out << "  const LibraryDef *defs[] = {";
  for(ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "&" << *ii << "_moddef, ";
  }

  out << "nullptr};\n"
      << "\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(defs, &py_" << library_name << "_module);\n"
      << "  if (module != nullptr) {\n";

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "    Dtool_" << *ii << "_BuildInstants(module);\n";
  }

  out << "  }\n"
      << "  return module;\n"
      << "}\n"
      << "\n"

      << "#else  // Python 2 case\n"
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
  out << "\n";

  out << "  const LibraryDef *defs[] = {";
  for(ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "&" << *ii << "_moddef, ";
  }

  out << "nullptr};\n"
      << "\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(defs, \"" << module_name << "\");\n"
      << "  if (module != nullptr) {\n";

  for (ii = libraries.begin(); ii != libraries.end(); ii++) {
    out << "    Dtool_" << *ii << "_BuildInstants(module);\n";
  }

  out << "  }\n"
      << "}\n"
      << "#endif\n"
      << "\n";


  return count;
}

int write_python_table(std::ostream &out) {
  out << "\n#include \"dtoolbase.h\"\n"
      << "#include \"interrogate_request.h\"\n\n"
      << "#undef _POSIX_C_SOURCE\n"
      << "#include \"Python.h\"\n\n";

  int count = 0;

  // First, we have to declare extern C prototypes for each of the function
  // names.

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

  out << "  { nullptr, nullptr }\n"
      << "};\n\n"

      << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef python_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << library_name << "\",\n"
      << "  nullptr,\n"
      << "  -1,\n"
      << "  python_methods,\n"
      << "  nullptr, nullptr, nullptr, nullptr\n"
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

  preprocess_argv(argc, argv);
  flag = getopt_long_only(argc, argv, short_options, long_options, nullptr);
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
    flag = getopt_long_only(argc, argv, short_options, long_options, nullptr);
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
      // If the filename ends in ".in", it's an interrogate database file, not
      // a shared library--read it directly.
      interrogate_request_database(param.c_str());

    } else {
      // Otherwise, assume it's a shared library, and try to load it.
      Filename pathname = argv[i];
      pathname.set_type(Filename::T_dso);
      nout << "Loading " << pathname << "\n";
      void *dl = load_dso(DSearchPath(), pathname);
      if (dl == nullptr) {
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

        // Output the support code.
        output_code << interrogate_preamble_python_native << "\n";
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
