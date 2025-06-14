/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_interrogate.cxx
 * @author drose
 * @date 1999-12-09
 */

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "load_dso.h"
#include "filename.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

#include <stdlib.h>

using std::cerr;
using std::cout;
using std::ostream;
using std::string;

static ostream &
indent(ostream &out, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    out << ' ';
  }
  return out;
}

// Indents one or more lines of text, breaking the text up at newline
// characters.
static ostream &
hanging_indent(ostream &out, const string &text, int first_indent_level,
               int next_indent_level = -1) {
  if (next_indent_level < 0) {
    next_indent_level = first_indent_level;
  }

  size_t start = 0;
  size_t newline = text.find('\n');
  int indent_level = first_indent_level;
  while (newline != string::npos) {
    indent(out, indent_level)
      << text.substr(start, newline - start) << "\n";
    start = newline + 1;
    newline = text.find('\n', start);
    indent_level = next_indent_level;
  }
  indent(out, indent_level)
    << text.substr(start) << "\n";

  return out;
}

void
show_type(int type, bool verbose = false) {
  cout << interrogate_type_name(type) << " ";
  if (verbose) {
    if (strcmp(interrogate_type_name(type),
               interrogate_type_true_name(type)) != 0) {
      cout << "(" << interrogate_type_true_name(type) << ") ";
    }
  }
  cout << "(" << type << ")";
}

void
show_function(int function) {
  cout << interrogate_function_scoped_name(function) << " (" << function << ")";
}

void
describe_wrapper(int wrapper, int indent_level) {
  indent(cout, indent_level)
    << "Wrapper (" << wrapper << ")";

  if (interrogate_wrapper_has_return_value(wrapper)) {
    cout << " returns ";
    show_type(interrogate_wrapper_return_type(wrapper));
  } else {
    cout << " no return value";
  }

  int num_params = interrogate_wrapper_number_of_parameters(wrapper);
  cout << ", ";
  if (num_params == 0) {
    cout << "no parameters.\n";
  } else if (num_params == 1) {
    cout << "1 parameter:\n";
  } else {
    cout << num_params << " parameters:\n";
  }

  for (int i = 0; i < num_params; i++) {
    indent(cout, indent_level + 4);
    if (interrogate_wrapper_parameter_is_this(wrapper, i)) {
      cout << "*";
    } else {
      cout << i;
    }
    cout << ": ";
    show_type(interrogate_wrapper_parameter_type(wrapper, i));
    if (interrogate_wrapper_parameter_has_name(wrapper, i)) {
      cout << " '" << interrogate_wrapper_parameter_name(wrapper, i) << "'";
    } else {
      cout << " (no name)";
    }
    cout << "\n";
  }

  if (interrogate_wrapper_has_comment(wrapper)) {
    string comment = interrogate_wrapper_comment(wrapper);
    hanging_indent(cout, comment, indent_level + 2);
  }


  if (interrogate_wrapper_caller_manages_return_value(wrapper)) {
    indent(cout, indent_level + 2)
      << "Caller manages return value using ";
    show_function(interrogate_wrapper_return_value_destructor(wrapper));
    cout << "\n";
  }

  indent(cout, indent_level + 2)
    << "Wrapper name: " << interrogate_wrapper_name(wrapper);
  if (interrogate_wrapper_is_callable_by_name(wrapper)) {
    cout << " (callable)";
  }
  cout << "\n";
  if (interrogate_wrapper_has_pointer(wrapper)) {
    indent(cout, indent_level + 2)
      << "Has pointer: " << interrogate_wrapper_pointer(wrapper) << "\n";
  }
  string unique_name = interrogate_wrapper_unique_name(wrapper);
  indent(cout, indent_level + 2)
    << "Unique name is " << unique_name;
  int reverse_lookup = interrogate_get_wrapper_by_unique_name(unique_name.c_str());
  if (reverse_lookup == 0) {
    cout << " (no reverse lookup)";
  } else if (reverse_lookup != wrapper) {
    cout << " (*** reverse lookup returns " << reverse_lookup << "! ***)";
  }
  cout << "\n";
}

void
describe_function(int function, int indent_level) {
  indent(cout, indent_level)
    << "Function " << interrogate_function_scoped_name(function)
    << " (" << function << ")\n";

  indent(cout, indent_level + 2)
    << "In C: ";
  hanging_indent(cout, interrogate_function_prototype(function),
                 0, indent_level + 2 + 6);

  if (interrogate_function_is_method(function)) {
    indent(cout, indent_level + 2)
      << "Method of ";
    show_type(interrogate_function_class(function));
    cout << "\n";
  }

  if (interrogate_function_is_virtual(function)) {
    indent(cout, indent_level + 2) << "is virtual.\n";
  }

  int w;

  int num_c_wrappers = interrogate_function_number_of_c_wrappers(function);
  if (num_c_wrappers == 0) {
  } else if (num_c_wrappers == 1) {
    indent(cout, indent_level + 2)
      << "1 C-style wrapper:\n";
  } else {
    indent(cout, indent_level + 2)
      << num_c_wrappers << " C-style wrappers:\n";
  }
  for (w = 0; w < num_c_wrappers; w++) {
    describe_wrapper(interrogate_function_c_wrapper(function, w),
                     indent_level + 4);
  }

  int num_python_wrappers =
    interrogate_function_number_of_python_wrappers(function);
  if (num_python_wrappers == 0) {
  } else if (num_python_wrappers == 1) {
    indent(cout, indent_level + 2)
      << "1 Python-style wrapper:\n";
  } else {
    indent(cout, indent_level + 2)
      << num_python_wrappers << " Python-style wrappers:\n";
  }
  for (w = 0; w < num_python_wrappers; w++) {
    describe_wrapper(interrogate_function_python_wrapper(function, w),
                     indent_level + 4);
  }
}

void
describe_make_seq(int make_seq, int indent_level) {
  indent(cout, indent_level)
    << "MakeSeq " << interrogate_make_seq_seq_name(make_seq)
    << " (" << make_seq << "): "
    << interrogate_make_seq_num_name(make_seq)
    << ", " << interrogate_make_seq_element_name(make_seq)
    << "\n";
}

void
report_manifests() {
  int num_manifests = interrogate_number_of_manifests();
  cout << "\n" << num_manifests << " manifests:\n";
  for (int i = 0; i < num_manifests; i++) {
    int manifest = interrogate_get_manifest(i);

    cout << "  Manifest " << interrogate_manifest_name(manifest);
    if (interrogate_manifest_has_type(manifest)) {
      cout << " of type ";
      show_type(interrogate_manifest_get_type(manifest));
      cout << "\n";
    } else {
      cout << " of unknown type\n";
    }
    cout << "    definition is \""
         << interrogate_manifest_definition(manifest) << "\"\n";

    if (interrogate_manifest_has_getter(manifest)) {
      cout << "    value getter: ";
      show_function(interrogate_manifest_getter(manifest));
      cout << "\n";
    }

    if (interrogate_manifest_has_int_value(manifest)) {
      cout << "    int value = "
           << interrogate_manifest_get_int_value(manifest)
           << "\n";
    }
  }
}

void
describe_element(int element, int indent_level) {
  indent(cout, indent_level)
    << "Element " << interrogate_element_scoped_name(element)
    << " of type ";
  show_type(interrogate_element_type(element));
  cout << "\n";

  if (interrogate_element_has_getter(element)) {
    indent(cout, indent_level + 2)
      << "Getter is ";
    show_function(interrogate_element_getter(element));
    cout << "\n";
  }

  if (interrogate_element_has_setter(element)) {
    indent(cout, indent_level + 2)
      << "Setter is ";
    show_function(interrogate_element_setter(element));
    cout << "\n";
  }
}

void
report_globals() {
  int num_globals = interrogate_number_of_globals();
  cout << "\n" << num_globals << " globals:\n";
  for (int i = 0; i < num_globals; i++) {
    describe_element(interrogate_get_global(i), 2);
  }
}

void
describe_type(int type, int indent_level) {
  indent(cout, indent_level) << "Type ";
  show_type(type, true);
  cout << "\n";

  if (interrogate_type_has_comment(type)) {
    string comment = interrogate_type_comment(type);
    hanging_indent(cout, comment, indent_level + 2);
  }

  if (interrogate_type_is_nested(type)) {
    indent(cout, indent_level + 2)
      << "Nested within ";
    show_type(interrogate_type_outer_class(type));
    cout << "\n";
  }
  if (interrogate_type_is_atomic(type)) {
    indent(cout, indent_level + 2)
      << "atomic " << (int)interrogate_type_atomic_token(type) << "\n";
  }
  if (interrogate_type_is_unsigned(type)) {
    indent(cout, indent_level + 2)
      << "unsigned\n";
  }
  if (interrogate_type_is_signed(type)) {
    indent(cout, indent_level + 2)
      << "signed\n";
  }
  if (interrogate_type_is_long(type)) {
    indent(cout, indent_level + 2)
      << "long\n";
  }
  if (interrogate_type_is_longlong(type)) {
    indent(cout, indent_level + 2)
      << "long long\n";
  }
  if (interrogate_type_is_short(type)) {
    indent(cout, indent_level + 2)
      << "short\n";
  }
  if (interrogate_type_is_wrapped(type)) {
    indent(cout, indent_level + 2)
      << "wrapped ";
    show_type(interrogate_type_wrapped_type(type));
    cout << "\n";
  }
  if (interrogate_type_is_pointer(type)) {
    indent(cout, indent_level + 2)
      << "pointer\n";
  }
  if (interrogate_type_is_const(type)) {
    indent(cout, indent_level + 2)
      << "const\n";
  }
  if (interrogate_type_is_fully_defined(type)) {
    indent(cout, indent_level + 2)
      << "fully defined\n";
  }
  if (interrogate_type_is_unpublished(type)) {
    indent(cout, indent_level + 2)
      << "undefined because unpublished\n";
  }
  if (interrogate_type_is_enum(type)) {
    indent(cout, indent_level + 2)
      << "is enum type\n";
  }
  if (interrogate_type_is_struct(type)) {
    indent(cout, indent_level + 2)
      << "is struct type\n";
  }
  if (interrogate_type_is_class(type)) {
    indent(cout, indent_level + 2)
      << "is class type\n";
  }
  if (interrogate_type_is_union(type)) {
    indent(cout, indent_level + 2)
      << "is union type\n";
  }
  int num_enum_values = interrogate_type_number_of_enum_values(type);
  if (num_enum_values > 0) {
    for (int i = 0; i < num_enum_values; i++) {
      indent(cout, indent_level + 4)
        << interrogate_type_enum_value_name(type, i)
        << " = " << interrogate_type_enum_value(type, i) << "\n";
    }
  }
  int num_constructors = interrogate_type_number_of_constructors(type);
  if (num_constructors > 0) {
    indent(cout, indent_level + 2)
      << num_constructors << " constructors:\n";
    for (int i = 0; i < num_constructors; i++) {
      describe_function(interrogate_type_get_constructor(type, i), 6);
    }
  }
  if (interrogate_type_has_destructor(type)) {
    indent(cout, indent_level + 2)
      << "destructor:\n";
    describe_function(interrogate_type_get_destructor(type), 6);
  }
  int num_casts = interrogate_type_number_of_casts(type);
  if (num_casts > 0) {
    indent(cout, indent_level + 2)
      << num_casts << " casts:\n";
    for (int i = 0; i < num_casts; i++) {
      describe_function(interrogate_type_get_cast(type, i), 6);
    }
  }
  int num_methods = interrogate_type_number_of_methods(type);
  if (num_methods > 0) {
    indent(cout, indent_level + 2)
      << num_methods << " methods:\n";
    for (int i = 0; i < num_methods; i++) {
      describe_function(interrogate_type_get_method(type, i), 6);
    }
  }
  int num_make_seqs = interrogate_type_number_of_make_seqs(type);
  if (num_make_seqs > 0) {
    indent(cout, indent_level + 2)
      << num_make_seqs << " make_seqs:\n";
    for (int i = 0; i < num_make_seqs; i++) {
      describe_make_seq(interrogate_type_get_make_seq(type, i), 6);
    }
  }
  int num_elements = interrogate_type_number_of_elements(type);
  if (num_elements > 0) {
    indent(cout, indent_level + 2)
      << num_elements << " elements:\n";
    for (int i = 0; i < num_elements; i++) {
      describe_element(interrogate_type_get_element(type, i), indent_level + 2);
    }
  }
  int num_derivations = interrogate_type_number_of_derivations(type);
  if (num_derivations > 0) {
    indent(cout, indent_level + 2)
      << num_derivations << " derivations:\n";
    for (int i = 0; i < num_derivations; i++) {
      int derivation = interrogate_type_get_derivation(type, i);
      indent(cout, indent_level + 4);
      show_type(derivation);
      if (interrogate_type_derivation_has_upcast(type, i)) {
        cout << " (has upcast)";
      }
      if (interrogate_type_derivation_downcast_is_impossible(type, i)) {
        cout << " (downcast is impossible)";
      }
      if (interrogate_type_derivation_has_downcast(type, i)) {
        cout << " (has downcast)";
      }
      cout << "\n";
      /*
        if (interrogate_type_derivation_has_upcast(type, i)) {
        describe_function(interrogate_type_get_upcast(type, i), 8);
        }
        if (interrogate_type_derivation_has_downcast(type, i)) {
        describe_function(interrogate_type_get_downcast(type, i), 8);
        }
      */
    }
  }
  int num_nested_types = interrogate_type_number_of_nested_types(type);
  if (num_nested_types > 0) {
    indent(cout, indent_level + 2)
      << num_nested_types << " nested types:\n";
    for (int i = 0; i < num_nested_types; i++) {
      indent(cout, indent_level + 4);
      show_type(interrogate_type_get_nested_type(type, i));
      cout << "\n";
    }
  }
}

void
report_global_types() {
  int num_types = interrogate_number_of_global_types();
  cout << "\n" << num_types << " global types:\n";

  for (int i = 0; i < num_types; i++) {
    int type = interrogate_get_global_type(i);
    describe_type(type, 2);
  }
}

void
report_global_functions() {
  int num_functions = interrogate_number_of_global_functions();
  cout << "\n" << num_functions << " global functions:\n";
  for (int i = 0; i < num_functions; i++) {
    int function = interrogate_get_global_function(i);
    describe_function(function, 2);
  }
}

void
report_all_types() {
  int num_types = interrogate_number_of_types();
  cout << "\n" << num_types << " total types:\n";

  for (int i = 0; i < num_types; i++) {
    int type = interrogate_get_type(i);
    describe_type(type, 2);
  }
}

void
report_all_functions() {
  int num_functions = interrogate_number_of_functions();
  cout << "\n" << num_functions << " total functions:\n";
  for (int i = 0; i < num_functions; i++) {
    int function = interrogate_get_function(i);
    describe_function(function, 2);
  }
}

void
usage() {
  cerr <<
    "\n"
    "test_interrogate [opts] libfile.so [libfile.so ...]\n\n"

    "Loads the given shared library or libraries, if possible, and reports the\n"
    "symbols, types, and functions available within those libraries as reported\n"
    "by interrogate.\n\n"

    "In lieu of loading a shared library, you may also read the interrogate\n"
    "database file directly by specifying a filename like libfile.in.  This will\n"
    "report the symbols defined in that file only (without pulling in dependent\n"
    "files), and will not have any function pointers available.\n\n"

    "Options:\n\n"
    "  -p [path]\n"
    "      Specify the search path for *.in files.  This option may be repeated.\n"
    "  -f  Give a detailed report of each function in the database, including\n"
    "      synthesized functions like upcasts and downcasts.\n"
    "  -t  Give a detailed report of every type in the database, including types\n"
    "      like pointers and const pointers.\n"
    "  -q  Quickly load up each shared library, if possible, and then immediately\n"
    "      exit.  Useful for quickly determining whether a library can even load.\n\n";
}

int
main(int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "p:ftqh";

  bool all_functions = false;
  bool all_types = false;
  bool quick_load = false;
  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'p':
      interrogate_add_search_path(optarg);
      break;

    case 'f':
      all_functions = true;
      break;

    case 't':
      all_types = true;
      break;

    case 'q':
      quick_load = true;
      break;

    case 'h':
      usage();
      exit(0);

    default:
      exit(1);
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    cerr << "No libraries specified.\n";
    exit(1);
  }

  int return_status = 0;

  for (int i = 1; i < argc; i++) {
    string param = argv[i];

    if (param.length() > 3 && param.substr(param.length() - 3) == ".in") {
      // If the filename ends in ".in", it's an interrogate database file, not
      // a shared library--read it directly.
      interrogate_request_database(param.c_str());

    } else {
      // Otherwise, assume it's a shared library, and try to load it.
      Filename pathname = Filename::dso_filename(argv[i]);
      cerr << "Loading " << pathname << "\n";

#ifdef _WIN32
      // test_interrogate always wants to show an error dialog.
      SetErrorMode(0);
#endif

      void *dl = load_dso(DSearchPath(), pathname);
      if (dl == nullptr) {
        cerr << "Unable to load: " << load_dso_error() << "\n";
        return_status++;
      }
    }
  }

  if (!quick_load) {
    if (all_types) {
      report_all_types();
    }
    if (all_functions) {
      report_all_functions();
    }

    if (!all_types && !all_functions) {
      report_manifests();
      report_globals();
      report_global_types();
      report_global_functions();
    }
  }

  return (return_status);
}
