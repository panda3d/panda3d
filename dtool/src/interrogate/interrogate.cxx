/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogate.cxx
 * @author drose
 * @date 2000-07-31
 */

#include "interrogate.h"
#include "interrogateBuilder.h"

#include "interrogateDatabase.h"
#include "cppGlobals.h"
#include "pnotify.h"
#include "panda_getopt_long.h"
#include "preprocess_argv.h"
#include <time.h>

using std::cerr;
using std::string;

CPPParser parser;

Filename output_code_filename;
Filename output_include_filename;
Filename output_data_filename;
Filename source_file_directory;
string output_data_basename;
bool output_module_specific = false;
bool output_function_pointers = false;
bool output_function_names = false;
bool convert_strings = false;
bool manage_reference_counts = false;
bool watch_asserts = false;
bool true_wrapper_names = false;
bool build_c_wrappers = false;
bool build_python_wrappers = false;
bool build_python_obj_wrappers = false;
bool build_python_native = false;
bool track_interpreter = false;
bool save_unique_names = false;
bool no_database = false;
bool generate_spam = false;
bool left_inheritance_requires_upcast = true;
bool mangle_names = true;
CPPVisibility min_vis = V_published;
string library_name;
string module_name;

// Short command-line options.
static const char *short_options = "I:S:D:F:vh";

// Long command-line options.
enum CommandOptions {
  CO_oc = 256,
  CO_od,
  CO_srcdir,
  CO_module,
  CO_library,
  CO_do_module,
  CO_fptrs,
  CO_fnames,
  CO_string,
  CO_refcount,
  CO_assert,
  CO_true_names,
  CO_c,
  CO_python,
  CO_python_obj,
  CO_python_native,
  CO_track_interpreter,
  CO_unique_names,
  CO_nodb,
  CO_longlong,
  CO_promiscuous,
  CO_spam,
  CO_noangles,
  CO_nomangle,
  CO_help,
};

static struct option long_options[] = {
  { "oc", required_argument, nullptr, CO_oc },
  { "od", required_argument, nullptr, CO_od },
  { "srcdir", required_argument, nullptr, CO_srcdir },
  { "module", required_argument, nullptr, CO_module },
  { "library", required_argument, nullptr, CO_library },
  { "do-module", no_argument, nullptr, CO_do_module },
  { "fptrs", no_argument, nullptr, CO_fptrs },
  { "fnames", no_argument, nullptr, CO_fnames },
  { "string", no_argument, nullptr, CO_string },
  { "refcount", no_argument, nullptr, CO_refcount },
  { "assert", no_argument, nullptr, CO_assert },
  { "true-names", no_argument, nullptr, CO_true_names },
  { "c", no_argument, nullptr, CO_c },
  { "python", no_argument, nullptr, CO_python },
  { "python-obj", no_argument, nullptr, CO_python_obj },
  { "python-native", no_argument, nullptr, CO_python_native },
  { "track-interpreter", no_argument, nullptr, CO_track_interpreter },
  { "unique-names", no_argument, nullptr, CO_unique_names },
  { "nodb", no_argument, nullptr, CO_nodb },
  { "longlong", required_argument, nullptr, CO_longlong },
  { "promiscuous", no_argument, nullptr, CO_promiscuous },
  { "spam", no_argument, nullptr, CO_spam },
  { "noangles", no_argument, nullptr, CO_noangles },
  { "nomangle", no_argument, nullptr, CO_nomangle },
  { "help", no_argument, nullptr, CO_help },
  { nullptr }
};

void
show_usage() {
  cerr
    << "\nUsage:\n"
    << "  interrogate [opts] file.C [file.C ...]\n"
    << "  interrogate -h\n\n";
}

void show_help() {
  show_usage();
  cerr
    << "Interrogate is a program to parse a body of C++ code and build up a table\n"
    << "of classes, methods, functions, and symbols found, for the purposes of\n"
    << "calling into the codebase via a non-C++ scripting language like Scheme,\n"
    << "Smalltalk, or Python.\n\n"

    << "In addition to identifying all the classes and their relationships,\n"
    << "interrogate will generate a wrapper function for each callable function.\n"
    << "The wrapper functions will be callable directly from the scripting language,\n"
    << "with no understanding of C++ necessary; these wrapper functions will in turn\n"
    << "call the actual C++ functions or methods.\n\n"

    << "Most exportable features of C++ are supported, including templates, default\n"
    << "parameters, and function overloading.\n\n"

    << "Options:\n\n"

    << "  -oc output.C\n"
    << "        Specify the name of the file to which generated code will be written.\n"
    << "        This includes all of the function wrappers, as well as those tables\n"
    << "        which must be compiled into the library.\n\n"

    << "  -od output.in\n"
    << "        Specify the name of the file to which the non-compiled data tables\n"
    << "        will be written.  This file describes the relationships between\n"
    << "        all the types and the functions, and associates the function wrappers\n"
    << "        above with this data.  This file will be opened and read at runtime\n"
    << "        when the scripting language first calls some interrogate query\n"
    << "        function.\n\n"

    << "  -srcdir directory\n"
    << "        Specify the name of the directory to which the source filenames are\n"
    << "        relative.\n\n"

    << "  -module module_name\n"
    << "        Defines the name of the module this data is associated with.  This\n"
    << "        is strictly a code-organizational tool.  Conceptually, a module is\n"
    << "        the highest level of grouping for interrogate data; a module may\n"
    << "        contain several libraries.  If this is omitted, no module name is\n"
    << "        specified.\n\n"

    << "        Sometimes, depending on the type of wrappers being generated, there\n"
    << "        may be additional code that needs to be generated on the module\n"
    << "        level, above that which was already generated at the library level.\n"
    << "        Python, for instance, generates the table of python-callable function\n"
    << "        wrappers at the module level.  Use the program interrogate-module\n"
    << "        to generate the appropriate code at the module level.\n\n"

    << "  -library library_name\n"
    << "        Defines the name of the library this data is associated with.  This\n"
    << "        is another code-organizational tool.  Typically, there will be one\n"
    << "        invocation of interrogate for each library, and there will be\n"
    << "        multiple libraries per module.  If this is omitted, no library name\n"
    << "        is specified.\n\n"

    << "  -do-module\n"
    << "        Generate whatever module-level code should be generated immediately,\n"
    << "        rather than waiting for a special interrogate-module pass.\n"
    << "        This, of course, prohibits grouping several libraries together\n"
    << "        into a single module.\n\n"

    << "  -fptrs\n"
    << "        Make void* pointers to the function wrappers directly available.  A\n"
    << "        scripting language will be able to call the interrogate functions\n"
    << "        directly by pointer.\n\n"

    << "  -fnames\n"
    << "        Make the names of the function wrappers public symbols so that the\n"
    << "        scripting language will be able to call the interrogate functions\n"
    << "        by name.\n\n"

    << "  Either or both of -fptrs and/or -fnames may be specified.  If both are\n"
    << "  omitted, the default is -fnames.\n\n"

    << "  -string\n"
    << "        Treat char* and basic_string<char> as special cases, and map\n"
    << "        parameters of these types to type atomic string.  The scripting\n"
    << "        language will see only functions that receive and return strings,\n"
    << "        not pointers to character or structures of basic_string<char>.\n"
    << "        If C calling convention wrappers are being generated, the atomic\n"
    << "        string type means type char*.  In any other calling convention, the\n"
    << "        atomic string type is whatever the native string type is.\n\n"

    << "  -refcount\n"
    << "        Treat classes that inherit from a class called ReferenceCount as a\n"
    << "        special case.  Any wrapper function that returns a pointer to\n"
    << "        one of these classes will automatically increment the reference\n"
    << "        count by calling ref() on the object first, and any destructors\n"
    << "        that are generated will call unref_delete() on the object instead of\n"
    << "        simply delete.\n\n"
    << "        Furthermore, parameters of type PointerTo<N> or ConstPointerTo<N>\n"
    << "        will automatically be mapped to N * and const N *, respectively.\n\n"

    << "  -assert\n"
    << "        Generate code in each wrapper that will check the state of the assert\n"
    << "        flag and trigger an exception in the scripting language when a\n"
    << "        C++ assertion fails.  Presently, this only has meaning to the Python\n"
    << "        wrappers.\n\n"

    << "  -true-names\n"
    << "        Use the actual name of the function being wrapped as the name of\n"
    << "        the generated wrapper function, instead of an ugly hash name.\n"
    << "        This means the wrapper functions may be called directly using a\n"
    << "        meaningful name (especially if -fnames is also given), but it\n"
    << "        also means that C++ function overloading (including default values\n"
    << "        for parameters) cannot be used, as it will lead to multiple wrapper\n"
    << "        functions with the same name.\n\n"

    << "  -c\n"
    << "        Generate function wrappers using the C calling convention.  Any\n"
    << "        scripting language that can call a C function should be able to\n"
    << "        make advantage of the interrogate database.\n\n"
    << "  -python\n"
    << "        Generate function wrappers using the Python calling convention.\n"
    << "        The shared library will be directly loadable as a Python module\n"
    << "        (especially if the module definitions are made available either by\n"
    << "        running interrogate-module later, or by specifying -do-module on\n"
    << "        the command line now).  However, C++ objects and methods will be\n"
    << "        converted into an object handle and a list of independent Python\n"
    << "        functions.\n\n"
    << "  -python-obj\n"
    << "        Generate Python function wrappers that convert C++ objects to true\n"
    << "        python objects, with all methods converted to Python methods.  This\n"
    << "        is currently experimental.\n\n"
    << "  -python-native\n"
    << "        Generate Python function wrappers that convert C++ objects to true\n"
    << "        python objects, with all methods converted to Python methods.  This\n"
    << "        is currently experimental.\n\n"

    << "  Any combination of -c, -python, or -python-obj may be specified.  If all\n"
    << "  are omitted, the default is -c.\n\n"

    << "  -track-interpreter\n"
    << "        Generate code within each wrapper function to adjust the global\n"
    << "        variable \"in_interpreter\" to indicated whether code is running\n"
    << "        within the Panda C++ environment or within the high-level language.\n"

    << "  -unique-names\n"
    << "        Compile a table into the library (i.e. generate code into the -oc\n"
    << "        file) that defines a lookup of each function wrapper by its unique\n"
    << "        name.  This makes it possible to consistently identify function\n"
    << "        wrappers between sessions, at the cost of having this additional\n"
    << "        table in memory.\n\n"

    << "  -nodb\n"
    << "        Do not build a full interrogate database, but just generate function\n"
    << "        wrappers.  It is assumed that the user will know how to call the\n"
    << "        function wrappers already, from some external source.  This is most\n"
    << "        useful in conjunction with -true-names.\n\n"

    << "  -promiscuous\n"
    << "        Export *all* public symbols, functions, and classes seen, even those\n"
    << "        not explicitly marked to be published.\n\n"

    << "  -spam\n"
    << "        Generate wrapper functions that report each invocation to Notify.\n"
    << "        This can sometimes be useful for tracking down bugs.\n\n"

    << "  -noangles\n"
    << "        Treat #include <file> the same as #include \"file\".  This means -I\n"
    << "        and -S are equivalent.\n\n"

    << "  -nomangle\n"
    << "        Do not generate camelCase equivalents of functions.\n\n";
}

// handle commandline -D options
static void
predefine_macro(CPPParser& parser, const string& inoption) {
  string macro_name, macro_def;

  size_t eq = inoption.find('=');
  if (eq != string::npos) {
    macro_name = inoption.substr(0, eq);
    macro_def = inoption.substr(eq + 1);
  } else {
    macro_name = inoption;
  }

  CPPManifest *macro = new CPPManifest(macro_name, macro_def);
  parser._manifests[macro->_name] = macro;
}

int
main(int argc, char **argv) {
  preprocess_argv(argc, argv);
  string command_line;
  int i;
  for (i = 0; i < argc; i++) {
    if (i > 0) {
      command_line += ' ';
    }
    command_line += string(argv[i]);
  }

  Filename fn;
  extern char *optarg;
  extern int optind;
  int flag;

  flag = getopt_long_only(argc, argv, short_options, long_options, nullptr);
  while (flag != EOF) {
    switch (flag) {
    case 'I':
      fn = Filename::from_os_specific(optarg);
      fn.make_absolute();
      parser._quote_include_path.append_directory(fn);
      parser._quote_include_kind.push_back(CPPFile::S_alternate);
      break;

    case 'S':
      fn = Filename::from_os_specific(optarg);
      fn.make_absolute();
      parser._angle_include_path.append_directory(fn);
      parser._quote_include_path.append_directory(fn);
      parser._quote_include_kind.push_back(CPPFile::S_system);
      break;

    case 'D':
      predefine_macro(parser, optarg);
      break;

    case 'F':
      // This is just a compile directive which we ignore.
      break;

    case 'v':
      parser.set_verbose(parser.get_verbose() + 1);
      break;

    case CO_oc:
      output_code_filename = Filename::from_os_specific(optarg);
      output_code_filename.make_absolute();
      break;

    case CO_od:
      output_data_filename = Filename::from_os_specific(optarg);
      output_data_filename.make_absolute();
      break;

    case CO_srcdir:
      source_file_directory = Filename::from_os_specific(optarg);
      source_file_directory.make_absolute();
      break;

    case CO_module:
      module_name = optarg;
      break;

    case CO_library:
      library_name = optarg;
      break;

    case CO_do_module:
      output_module_specific = true;
      break;

    case CO_fptrs:
      output_function_pointers = true;
      break;

    case CO_fnames:
      output_function_names = true;
      break;

    case CO_string:
      convert_strings = true;
      break;

    case CO_refcount:
      manage_reference_counts = true;
      break;

    case CO_assert:
      watch_asserts = true;
      break;

    case CO_true_names:
      true_wrapper_names = true;
      break;

    case CO_c:
      build_c_wrappers = true;
      break;

    case CO_python:
      build_python_wrappers = true;
      break;

    case CO_python_obj:
      build_python_obj_wrappers = true;
      break;

    case CO_python_native:
        build_python_native = true;
        break;

    case CO_track_interpreter:
      track_interpreter = true;
      break;

    case CO_unique_names:
      save_unique_names = true;
      break;

    case CO_nodb:
      no_database = true;
      break;

    case CO_longlong:
      cerr << "Warning: ignoring deprecated -longlong option.\n";
      cpp_longlong_keyword = optarg;
      break;

    case CO_promiscuous:
      min_vis = V_public;
      break;

    case CO_spam:
      generate_spam = true;
      break;

    case CO_noangles:
      parser._noangles = true;
      break;

    case CO_nomangle:
      mangle_names = false;
      break;

    case 'h':
    case CO_help:
      show_help();
      exit(0);

    default:
      exit(1);
    }
    flag = getopt_long_only(argc, argv, short_options, long_options, nullptr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    show_usage();
    exit(1);
  }

  // If requested, change directory to the source-file directory.
  if (source_file_directory != "") {
    if (!source_file_directory.chdir()) {
      cerr << "Could not change directory to " << source_file_directory << "\n";
      exit(1);
    }
  }

// if(!output_code_filename.empty()) { output_include_filename =
// output_code_filename.get_fullpath_wo_extension() +".h"; printf(" Include
// File Will be Set to %s \n",output_include_filename.c_str()); }

  output_code_filename.set_text();
  output_data_filename.set_text();
// output_include_filename.set_text();
  output_data_basename = output_data_filename.get_basename();

  if (output_function_names && true_wrapper_names) {
    cerr
      << "Cannot simultaneously export function names and report\n"
      << "true wrapper names--wrapper names will clash with the\n"
      << "wrapped functions!\n";
    exit(1);
  }

  if (!build_c_wrappers && !build_python_wrappers &&
      !build_python_obj_wrappers &&!build_python_native) {
    build_c_wrappers = true;
  }

  // Add all of the .h files we are explicitly including to the parser.
  for (i = 1; i < argc; ++i) {
    Filename filename = Filename::from_os_specific(argv[i]);
    filename.make_absolute();
    parser._explicit_files.insert(filename);
  }

  // Now go through them again and feed them into the C++ parser.
  for (i = 1; i < argc; ++i) {
    Filename filename = Filename::from_os_specific(argv[i]);
    if (!parser.parse_file(filename)) {
      cerr << "Error parsing file: '" << argv[i] << "'\n";
      exit(1);
    }
    builder.add_source_file(filename.to_os_generic());
  }

  // Now that we've parsed all the source code, change the way things are
  // output from now on so we can compile our generated code using VC++.
  // Sheesh.

  // Actually, don't do this any more, since it bitches some of the logic
  // (particularly with locating alt names), and it shouldn't be necessary
  // with modern VC++. cppparser_output_class_keyword = false;

  // Now look for the .N files.
  for (i = 1; i < argc; ++i) {
    Filename filename = Filename::from_os_specific(argv[i]);
    Filename nfilename = filename;
    nfilename.set_extension("N");
    nfilename.set_text();
    pifstream nfile;
    if (nfilename.open_read(nfile)) {
      builder.read_command_file(nfile);
    }
  }

  builder.build();

  // Make up a file identifier.  This is just some bogus number that should be
  // the same in both the compiled-in code and in the database, so we can
  // check synchronicity at load time.
  int file_identifier = time(nullptr);
  InterrogateModuleDef *def = builder.make_module_def(file_identifier);

  pofstream * the_output_include = nullptr;
  pofstream output_include;


  if (1==2 && !output_include_filename.empty())
  {
    output_include_filename.open_write(output_include);

    output_include << "#ifndef   " << output_include_filename.get_basename_wo_extension() << "__HH__\n";
    output_include << "#define   " << output_include_filename.get_basename_wo_extension() << "__HH__\n";

    output_include
      << "/*\n"
      << " * This file was generated by:\n"
      << " * " << command_line << "\n"
      << " *\n"
      << " */\n\n";


    if (output_include.fail())
    {
      nout << "Unable to write to " << output_include_filename << "\n";
      exit(-1);
    }
    the_output_include = &output_include;
  }

  int status = 0;

  // Now output all of the wrapper functions.
  if (!output_code_filename.empty())
  {
    pofstream output_code;
    output_code_filename.open_write(output_code);

    output_code
      << "/*\n"
      << " * This file was generated by:\n"
      << " * " << command_line << "\n"
      << " *\n"
      << " */\n\n";

    if(the_output_include != nullptr)
    {
        output_code << "#include \""<<output_include_filename<<"\"\n";
        *the_output_include << "#include \"" << output_include_filename.get_fullpath_wo_extension() << "_pynative.h\"\n";
    }

    if (output_code.fail()) {
      nout << "Unable to write to " << output_code_filename << "\n";
      status = -1;
    } else {
      builder.write_code(output_code,the_output_include, def);
    }
  }


  if(the_output_include != nullptr)
      *the_output_include << "#endif  // #define   " << output_include_filename.get_basename_wo_extension() << "__HH__\n";

  // And now output the bulk of the database.
  if (!output_data_filename.empty()) {
    pofstream output_data;
    output_data_filename.open_write(output_data);

    if (output_data.fail()) {
      nout << "Unable to write to " << output_data_filename << "\n";
      status = -1;
    } else {
      InterrogateDatabase::get_ptr()->write(output_data, def);
    }
  }

  return status;
}
