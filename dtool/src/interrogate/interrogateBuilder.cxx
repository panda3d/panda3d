// Filename: interrogateBuilder.cxx
// Created by:  drose (01Aug00)
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

#include "interrogateBuilder.h"
#include "interrogate.h"
#include "parameterRemap.h"
#include "typeManager.h"
#include "functionWriters.h"
#include "interfaceMakerC.h"
#include "interfaceMakerPythonObj.h"
#include "interfaceMakerPythonSimple.h"
#include "interfaceMakerPythonNative.h"
#include "functionRemap.h"

#include "interrogateType.h"
#include "interrogateDatabase.h"
#include "indexRemapper.h"
#include "cppParser.h"
#include "cppDeclaration.h"
#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppInstance.h"
#include "cppSimpleType.h"
#include "cppPointerType.h"
#include "cppReferenceType.h"
#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppExtensionType.h"
#include "cppStructType.h"
#include "cppExpression.h"
#include "cppTypedefType.h"
#include "cppTypeDeclaration.h"
#include "cppEnumType.h"
#include "cppCommentBlock.h"
#include "cppMakeProperty.h"
#include "cppMakeSeq.h"
#include "pnotify.h"

#include <ctype.h>
#include <algorithm>

InterrogateBuilder builder;
std::string EXPORT_IMPORT_PREFIX;

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::add_source_file
//       Access: Public
//  Description: Adds the given source filename to the list of files
//               that we are scanning.  Those source files that appear
//               to be header files will be #included in the generated
//               code file.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
add_source_file(const string &filename) {
  if (filename.empty()) {
    return;
  }

  _include_files[filename] = '"';
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::read_command_file
//       Access: Public
//  Description: Reads a .N file that might contain control
//               information for the interrogate process.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
read_command_file(istream &in) {
  string line;
  getline(in, line);
  while (!in.fail() && !in.eof()) {
    // Strip out the comment.
    size_t hash = line.find('#');
    if (hash != string::npos) {
      line = line.substr(0, hash);
    }

    // Skip leading whitespace.
    size_t p = 0;
    while (p < line.length() && isspace(line[p])) {
      p++;
    }

    if (p < line.length()) {
      // Get the first word.
      size_t q = p;
      while (q < line.length() && !isspace(line[q])) {
        q++;
      }
      string command = line.substr(p, q - p);

      // Get the rest.
      p = q;
      while (p < line.length() && isspace(line[p])) {
        p++;
      }
      // Except for the trailing whitespace.
      q = line.length();
      while (q > p && isspace(line[q - 1])) {
        q--;
      }
      string params = line.substr(p, q - p);

      do_command(command, params);
    }
    getline(in, line);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::do_command
//       Access: Public
//  Description: Executes a single command as read from the .N file.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
do_command(const string &command, const string &params) {

  if (command == "forcevisible") {
    CPPType *type = parser.parse_type(params);
    if (type == (CPPType *)NULL) {
      nout << "Unknown type: allowtype " << params << "\n";
    } else {
      type = type->resolve_type(&parser, &parser);
      type->_vis = min_vis;
    }

  } else if (command == "forcetype") {
    // forcetype explicitly exports the given type.
    CPPType *type = parser.parse_type(params);
    if (type == (CPPType *)NULL) {
      nout << "Unknown type: forcetype " << params << "\n";
    } else {
      type = type->resolve_type(&parser, &parser);
      type->_forcetype = true;
      _forcetype.insert(type->get_local_name(&parser));
    }

  } else if (command == "renametype") {
    // rename exports the type as the indicated name.  We strip off
    // the last word as the new name; the new name may not contain
    // spaces (although the original type name may).

    size_t space = params.rfind(' ');
    if (space == string::npos) {
      nout << "No new name specified for renametype " << params << "\n";
    } else {
      string orig_name = params.substr(0, space);
      string new_name = params.substr(space + 1);

      CPPType *type = parser.parse_type(orig_name);
      if (type == (CPPType *)NULL) {
        nout << "Unknown type: renametype " << orig_name << "\n";
      } else {
        type = type->resolve_type(&parser, &parser);
        _renametype[type->get_local_name(&parser)] = new_name;
      }
    }

  } else if (command == "ignoretype") {
    // ignoretype explicitly ignores the given type.
    CPPType *type = parser.parse_type(params);
    if (type == (CPPType *)NULL) {
      nout << "Unknown type: ignoretype " << params << "\n";
    } else {
      type = type->resolve_type(&parser, &parser);
      _ignoretype.insert(type->get_local_name(&parser));
    }

  } else if (command == "defconstruct") {
    // defining the parameters that are implicitly supplied to the
    // generated default constructor.  Especially useful for linmath
    // objects, whose default constructor in C++ is uninitialized, but
    // whose Python-level constructor should initialize to 0.

    size_t space = params.find(' ');
    if (space == string::npos) {
      nout << "No constructor specified for defconstruct " << params << "\n";
    } else {
      string class_name = params.substr(0, space);
      string constructor = params.substr(space + 1);

      CPPType *type = parser.parse_type(class_name);
      if (type == (CPPType *)NULL) {
        nout << "Unknown type: defconstruct " << class_name << "\n";
      } else {
        type = type->resolve_type(&parser, &parser);
        _defconstruct[type->get_local_name(&parser)] = constructor;
      }
    }

  } else if (command == "ignoreinvolved") {
    _ignoreinvolved.insert(params);

  } else if (command == "ignorefile") {
    insert_param_list(_ignorefile, params);

  } else if (command == "ignoremember") {
    insert_param_list(_ignoremember, params);

  } else if (command == "noinclude") {
    insert_param_list(_noinclude, params);

  } else if (command == "forceinclude") {
    size_t nchars = params.size();
    if (nchars >= 2 && params[0] == '"' && params[nchars-1] == '"') {
      string incfile = params.substr(1, nchars - 2);
      _include_files[incfile] = '"';

    } else if (nchars >= 2 && params[0] == '<' && params[nchars-1] == '>') {
      string incfile = params.substr(1, nchars - 2);
      _include_files[incfile] = '<';

    } else {
      nout << "Ignoring invalid forceinclude " << params << "\n"
              "Expected to be in one of the following forms:\n"
              "  forceinclude \"file.h\"\n"
              "  forceinclude <file.h>\n";
    }
  } else {
    nout << "Ignoring " << command << " " << params << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::build
//       Access: Public
//  Description: Builds all of the interrogate data.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
build() {
  _library_hash_name = hash_string(library_name, 5);

  // Make sure we have the complete set of #includes we need.
  CPPParser::Includes::const_iterator ii;
  for (ii = parser._quote_includes.begin();
       ii != parser._quote_includes.end();
       ++ii) {
    const string &filename = (*ii);
    _include_files[filename] = '"';
  }
  for (ii = parser._angle_includes.begin();
       ii != parser._angle_includes.end();
       ++ii) {
    const string &filename = (*ii);
    _include_files[filename] = '<';
  }

  // First, get all the types that were explicitly forced.
  Commands::const_iterator ci;
  for (ci = _forcetype.begin();
       ci != _forcetype.end();
       ++ci) {
    CPPType *type = parser.parse_type(*ci);
    assert(type != (CPPType *)NULL);
    get_type(type, true);
  }

  // Now go through all of the top-level declarations in the file(s).

  CPPScope::Declarations::const_iterator di;
  for (di = parser._declarations.begin();
       di != parser._declarations.end();
       ++di) {
    if ((*di)->get_subtype() == CPPDeclaration::ST_instance) {
      CPPInstance *inst = (*di)->as_instance();
      if (inst->_type->get_subtype() == CPPDeclaration::ST_function) {
        // Here's a function declaration.
        scan_function(inst);
      } else {
        // Here's a data element declaration.
        scan_element(inst, (CPPStructType *)NULL, &parser);
      }

    } else if ((*di)->get_subtype() == CPPDeclaration::ST_typedef) {
      CPPTypedefType *tdef = (*di)->as_typedef_type();

      if (tdef->_type->get_subtype() == CPPDeclaration::ST_struct) {
        // A typedef counts as a declaration.  This lets us pick up
        // most template instantiations.
        CPPStructType *struct_type =
          tdef->_type->resolve_type(&parser, &parser)->as_struct_type();
        scan_struct_type(struct_type);
      }

      scan_typedef_type(tdef);

    } else if ((*di)->get_subtype() == CPPDeclaration::ST_type_declaration) {
      CPPType *type = (*di)->as_type_declaration()->_type;

      type->_vis = (*di)->_vis;

      if (type->get_subtype() == CPPDeclaration::ST_struct) {
        CPPStructType *struct_type =
          type->as_type()->resolve_type(&parser, &parser)->as_struct_type();
        scan_struct_type(struct_type);

      } else if (type->get_subtype() == CPPDeclaration::ST_enum) {
        CPPEnumType *enum_type =
          type->as_type()->resolve_type(&parser, &parser)->as_enum_type();
        scan_enum_type(enum_type);
      }
    }
  }

  CPPPreprocessor::Manifests::const_iterator mi;
  for (mi = parser._manifests.begin(); mi != parser._manifests.end(); ++mi) {
    CPPManifest *manifest = (*mi).second;
    scan_manifest(manifest);
  }

  // Now that we've gone through all the code and generated all the
  // functions and types, build the function wrappers.
  //  make_wrappers();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::write_code
//       Access: Public
//  Description: Generates all the code necessary to the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
write_code(ostream &out_code,ostream * out_include, InterrogateModuleDef *def) {
  typedef vector<InterfaceMaker *> InterfaceMakers;
  InterfaceMakers makers;

  if (build_c_wrappers) {
    InterfaceMaker *maker = new InterfaceMakerC(def);
    makers.push_back(maker);
  }

  if (build_python_wrappers) {
    InterfaceMaker *maker = new InterfaceMakerPythonSimple(def);
    makers.push_back(maker);
  }

  if (build_python_obj_wrappers) {
    InterfaceMaker *maker = new InterfaceMakerPythonObj(def);
    makers.push_back(maker);
  }

  if (build_python_native) {
    InterfaceMakerPythonNative *maker = new InterfaceMakerPythonNative(def);
    makers.push_back(maker);
  }

  EXPORT_IMPORT_PREFIX = std::string("EXPCL_") + def->module_name;
  for (size_t i = 0; i < EXPORT_IMPORT_PREFIX.size(); i++) {
    EXPORT_IMPORT_PREFIX[i] = toupper(EXPORT_IMPORT_PREFIX[i]);
  }

  InterfaceMakers::iterator mi;
  // First, make all the wrappers.
  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->generate_wrappers();
  }

  // Now generate all the function bodies to a temporary buffer.  By
  // generating these first, we ensure that we know all of the
  // pointers we'll be using ahead of time (and can therefore generate
  // correct prototypes).
  ostringstream function_bodies;
  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->write_functions(function_bodies);
  }

  // Now, begin the actual output.  Start with the #include lines.
  if (!no_database) {
    out_code << "#include \"dtoolbase.h\"\n"
             << "#include \"interrogate_request.h\"\n"
             << "#include \"dconfig.h\"\n";
  }

  ostringstream declaration_bodies;

  if (watch_asserts) {
    declaration_bodies << "#include \"pnotify.h\"\n";
  }

  declaration_bodies << "#include <sstream>\n";

  if (build_python_native) {
    if (library_name.size() > 1) {
      declaration_bodies << "#define PANDA_LIBRARY_NAME_" << library_name << "\n";
    }
    declaration_bodies << "#include \"py_panda.h\"\n";
    declaration_bodies << "#include \"extension.h\"\n";
    declaration_bodies << "#include \"dcast.h\"\n";
  }
  declaration_bodies << "\n";

  IncludeFiles::const_iterator ifi;
  for (ifi = _include_files.begin();
       ifi != _include_files.end();
       ++ifi) {
    const string &filename = (*ifi).first;
    char delimiter = (*ifi).second;
    if (should_include(filename)) {
      if (delimiter == '"') {
        declaration_bodies << "#include \"" << filename << "\"\n";
      } else {
        declaration_bodies << "#include <" << filename << ">\n";
      }
    }
  }
  declaration_bodies << "\n";

  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->write_includes(declaration_bodies);
  }

  if (generate_spam) {
    declaration_bodies << "#include \"config_interrogatedb.h\"\n"
        << "#include \"notifyCategoryProxy.h\"\n\n"
        << "NotifyCategoryDeclNoExport(in_" << library_name << ");\n"
        << "NotifyCategoryDef(in_" << library_name << ", interrogatedb_cat);\n\n";
  }

  declaration_bodies << "\n";

  // And now the prototypes.
  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->write_prototypes(declaration_bodies,out_include);
  }
  declaration_bodies << "\n";

//  if(out_include != NULL)
//    (*out_include) << declaration_bodies.str();
//  else
  out_code << declaration_bodies.str();

  // Followed by the function bodies.
  out_code << function_bodies.str() << "\n";

  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->write_module_support(out_code, out_include, def);
  }

  if (output_module_specific) {
    // Output whatever stuff we should output if this were a module.
    for (mi = makers.begin(); mi != makers.end(); ++mi) {
      (*mi)->write_module(out_code, out_include, def);
    }
  }

  // Now collect all the function wrappers.
  vector<FunctionRemap *> remaps;
  for (mi = makers.begin(); mi != makers.end(); ++mi) {
    (*mi)->get_function_remaps(remaps);
  }

  // Make sure all of the function wrappers appear first in the set of
  // indices, and that they occupy consecutive index numbers, so we
  // can build a simple array of function pointers by index.
  remap_indices(remaps);

  // Get the function wrappers in index-number order.
  int num_wrappers = 0;
  map<int, FunctionRemap *> wrappers_by_index;

  vector<FunctionRemap *>::iterator ri;
  for (ri = remaps.begin(); ri != remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    wrappers_by_index[remap->_wrapper_index] = remap;
    num_wrappers++;
  }

  if (output_function_pointers) {
    // Write out the table of function pointers.
    out_code << "static void *_in_fptrs[" << num_wrappers << "] = {\n";
    int next_index = 1;
    map<int, FunctionRemap *>::iterator ii;
    for (ii = wrappers_by_index.begin();
         ii != wrappers_by_index.end();
         ++ii) {
      int this_index = (*ii).first;
      while (next_index < this_index) {
        out_code << "  (void *)0,\n";
        next_index++;
      }
      assert(next_index == this_index);
      FunctionRemap *remap = (*ii).second;

      out_code << "  (void *)&" << remap->_wrapper_name << ",\n";
      next_index++;
    }
    while (next_index < num_wrappers + 1) {
      out_code << "  (void *)0,\n";
      next_index++;
    }
    out_code << "};\n\n";
  }

  if (save_unique_names) {
    // Write out the table of unique names, in no particular order.
    out_code << "static InterrogateUniqueNameDef _in_unique_names["
        << num_wrappers << "] = {\n";
    for (ri = remaps.begin(); ri != remaps.end(); ++ri) {
      FunctionRemap *remap = (*ri);
      out_code << "  { \""
          << remap->_unique_name << "\", "
          << remap->_wrapper_index - 1 << " },\n";
    }
    out_code << "};\n\n";
  }

  if (!no_database) {
    // Now build the module definition structure to add ourselves to
    // the global interrogate database.
    out_code << "static InterrogateModuleDef _in_module_def = {\n"
        << "  " << def->file_identifier << ",  /* file_identifier */\n"
        << "  \"" << def->library_name << "\",  /* library_name */\n"
        << "  \"" << def->library_hash_name << "\",  /* library_hash_name */\n"
        << "  \"" << def->module_name << "\",  /* module_name */\n";
    if (def->database_filename != (const char *)NULL) {
      out_code << "  \"" << def->database_filename
          << "\",  /* database_filename */\n";
    } else {
      out_code << "  (const char *)0,  /* database_filename */\n";
    }

    if (save_unique_names) {
      out_code << "  _in_unique_names,\n"
          << "  " << num_wrappers << ",  /* num_unique_names */\n";
    } else {
      out_code << "  (InterrogateUniqueNameDef *)0,  /* unique_names */\n"
          << "  0,  /* num_unique_names */\n";
    }

    if (output_function_pointers) {
      out_code << "  _in_fptrs,\n"
          << "  " << num_wrappers << ",  /* num_fptrs */\n";
    } else {
      out_code << "  (void **)0,  /* fptrs */\n"
          << "  0,  /* num_fptrs */\n";
    }

    out_code << "  1,  /* first_index */\n"
        << "  " << InterrogateDatabase::get_ptr()->get_next_index()
        << "  /* next_index */\n"
        << "};\n\n";

    // And now write the static-init code that tells the interrogate
    // database to load up this module.
    out_code << "Configure(_in_configure_" << library_name << ");\n"
        << "ConfigureFn(_in_configure_" << library_name << ") {\n"
        << "  interrogate_request_module(&_in_module_def);\n"
        << "}\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::make_module_def
//       Access: Public
//  Description: Allocates and returns a new InterrogateModuleDef
//               structure that reflects the data we have just build,
//               or at least that subset of the InterrogateModuleDef
//               data that we have available at this time.
//
//               The data in this structure may include pointers that
//               reference directly into the InterrogateBuilder
//               object; thus, this structure is only valid for as
//               long as the builder itself remains in scope.
////////////////////////////////////////////////////////////////////
InterrogateModuleDef *InterrogateBuilder::
make_module_def(int file_identifier) {
  InterrogateModuleDef *def = new InterrogateModuleDef;
  memset(def, 0, sizeof(InterrogateModuleDef));

  def->file_identifier = file_identifier;
  def->library_name = library_name.c_str();
  def->library_hash_name = _library_hash_name.c_str();
  def->module_name = module_name.c_str();
  if (!output_data_filename.empty()) {
    def->database_filename = output_data_basename.c_str();
  }

  return def;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::clean_identifier
//       Access: Public, Static
//  Description: Adjusts the given string to remove any characters we
//               don't want to export as part of an identifier name.
//               Returns the cleaned string.
//
//               This replaces any consecutive invalid characters with
//               an underscore.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
clean_identifier(const string &name) {
  string result;

  bool last_invalid = false;

  string::const_iterator ni;
  for (ni = name.begin(); ni != name.end(); ++ni) {
    if (isalnum(*ni)) {
      if (last_invalid) {
        result += '_';
        last_invalid = false;
      }
      result += (*ni);
    } else {
      last_invalid = true;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::descope
//       Access: Public, Static
//  Description: Removes the leading "::", if present, from a
//               fully-scoped name.  Sometimes CPPParser throws this
//               on, and sometimes it doesn't.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
descope(const string &name) {
  if (name.length() >= 2 && name.substr(0, 2) == "::") {
    return name.substr(2);
  }
  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_destructor_for
//       Access: Public
//  Description: Returns the FunctionIndex for the destructor
//               appropriate to destruct an instance of the indicated
//               type, or 0 if no suitable destructor exists.
////////////////////////////////////////////////////////////////////
FunctionIndex InterrogateBuilder::
get_destructor_for(CPPType *type) {
  TypeIndex type_index = get_type(type, false);

  const InterrogateType &itype =
    InterrogateDatabase::get_ptr()->get_type(type_index);

  return itype.get_destructor();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_preferred_name
//       Access: Public
//  Description: Returns the name of the type as it should be reported
//               to the database.  This is either the name indicated
//               by the user via a renametype command, or the
//               "preferred name" of the type itself (i.e. the typedef
//               name within the C++ code), or failing that, the
//               type's true name.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
get_preferred_name(CPPType *type) {
  string true_name = type->get_local_name(&parser);
  string name = in_renametype(true_name);
  if (!name.empty()) {
    return name;
  }
  return type->get_preferred_name();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::hash_string
//       Access: Public, Static
//  Description: Hashes an arbitrary string into a four-character
//               string using only the characters legal in a C
//               identifier.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
hash_string(const string &name, int shift_offset) {
  int hash = 0;

  int shift = 0;
  string::const_iterator ni;
  for (ni = name.begin(); ni != name.end(); ++ni) {
    int c = (int)(unsigned char)(*ni);
    int shifted_c = (c << shift) & 0xffffff;
    if (shift > 16) {
      // We actually want a circular shift, not an arithmetic shift.
      shifted_c |= ((c >> (24 - shift)) & 0xff) ;
    }
    hash = (hash + shifted_c) & 0xffffff;
    shift = (shift + shift_offset) % 24;
  }

  // Now multiply the hash by a biggish prime number and apply the
  // high-order bits back at the bottom, to scramble up the bits a
  // bit.  This helps reduce hash conflicts from names that are
  // similar to each other, by separating adjacent hash codes.
  int prime = 4999;
  int low_order = (hash * prime) & 0xffffff;
  int high_order = (int)((double)hash * (double)prime / (double)(1 << 24));
  hash = low_order ^ high_order;

  // Also add in the additional_number, times some prime factor.
  //  hash = (hash + additional_number * 1657) & 0xffffff;

  // Now turn the hash code into a four-character string.  For each
  // six bits, we choose a character in the set [A-Za-z0-9_].  Note
  // that there are only 63 characters to choose from; we have to
  // duplicate '_' for values 62 and 63.  This introduces a small
  // additional chance of hash conflicts.  No big deal, since we have
  // to resolve hash conflicts anyway.

  string result;
  int extract_h = hash;
  for (int i = 0; i < 4; i++) {
    int value = (extract_h & 0x3f);
    extract_h >>= 6;
    if (value < 26) {
      result += (char)('A' + value);

    } else if (value < 52) {
      result += (char)('a' + value - 26);

    } else if (value < 62) {
      result += (char)('0' + value - 52);

    } else {
      result += '_';
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::insert_param_list
//       Access: Public
//  Description: Inserts a list of space-separated parameters into the
//               given command parameter list.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
insert_param_list(InterrogateBuilder::Commands &commands,
                  const string &params) {
  size_t p = 0;
  while (p < params.length()) {
    while (p < params.length() && isspace(params[p])) {
      p++;
    }
    size_t q = p;
    while (q < params.length() && !isspace(params[q])) {
      q++;
    }
    if (p < q) {
      commands.insert(params.substr(p, q - p));
    }
    p = q;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_forcetype
//       Access: Private
//  Description: Returns true if the indicated name is one that the
//               user identified with a forcetype command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_forcetype(const string &name) const {
  return (_forcetype.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_renametype
//       Access: Private
//  Description: If the user requested an explicit name for this type
//               via the renametype command, returns that name;
//               otherwise, returns the empty string.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
in_renametype(const string &name) const {
  CommandParams::const_iterator pi;
  pi = _renametype.find(name);
  if (pi != _renametype.end()) {
    return (*pi).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_ignoretype
//       Access: Private
//  Description: Returns true if the indicated name is one that the
//               user identified with an ignoretype command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_ignoretype(const string &name) const {
  return (_ignoretype.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_defconstruct
//       Access: Private
//  Description: If the user requested an explicit default constructor
//               for this type via the defconstruct command, returns
//               that string; otherwise, returns the empty string.
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
in_defconstruct(const string &name) const {
  CommandParams::const_iterator pi;
  pi = _defconstruct.find(name);
  if (pi != _defconstruct.end()) {
    return (*pi).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_ignoreinvolved
//       Access: Private
//  Description: Returns true if the indicated name is one that the
//               user identified with an ignoreinvolved command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_ignoreinvolved(const string &name) const {
  return (_ignoreinvolved.count(name) != 0);
}
////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_ignoreinvolved
//       Access: Private
//  Description: Returns true if the indicated type involves some type
//               name that the user identified with an ignoreinvolved
//               command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_ignoreinvolved(CPPType *type) const {
  switch (type->get_subtype()) {
  case CPPDeclaration::ST_pointer:
    {
      CPPPointerType *ptr = type->as_pointer_type();
      return in_ignoreinvolved(ptr->_pointing_at);
    }

  case CPPDeclaration::ST_array:
    {
      CPPArrayType *ary = type->as_array_type();
      return in_ignoreinvolved(ary->_element_type);
    }

  case CPPDeclaration::ST_reference:
    {
      CPPReferenceType *ref = type->as_reference_type();
      return in_ignoreinvolved(ref->_pointing_at);
    }

  case CPPDeclaration::ST_const:
    {
      CPPConstType *cnst = type->as_const_type();
      return in_ignoreinvolved(cnst->_wrapped_around);
    }

  case CPPDeclaration::ST_function:
    {
      CPPFunctionType *ftype = type->as_function_type();
      if (in_ignoreinvolved(ftype->_return_type)) {
        return true;
      }
      const CPPParameterList::Parameters &params =
        ftype->_parameters->_parameters;
      CPPParameterList::Parameters::const_iterator pi;
      for (pi = params.begin(); pi != params.end(); ++pi) {
        if (in_ignoreinvolved((*pi)->_type)) {
          return true;
        }
      }
      return false;
    }

  case CPPDeclaration::ST_typedef:
    {
      if (in_ignoreinvolved(type->get_simple_name())) {
        return true;
      }
      CPPTypedefType *tdef = type->as_typedef_type();
      return in_ignoreinvolved(tdef->_type);
    }

  default:
    return in_ignoreinvolved(type->get_simple_name());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_ignorefile
//       Access: Private
//  Description: Returns true if the indicated name is one that the
//               user identified with an ignorefile command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_ignorefile(const string &name) const {
  return (_ignorefile.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_ignoremember
//       Access: Private
//  Description: Returns true if the indicated name is one that the
//               user identified with an ignoremember command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_ignoremember(const string &name) const {
  return (_ignoremember.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::in_noinclude
//       Access: Private
//  Description: Returns true if the indicated filename is one that
//               the user identified with a noinclude command.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
in_noinclude(const string &name) const {
  return (_noinclude.count(name) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::should_include
//       Access: Private
//  Description: Returns true if the indicated filename is a valid
//               file to explicitly #include in the generated .cxx
//               file, false otherwise.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
should_include(const string &filename) const {
  // Don't directly include any .cxx or .I files, except for extensions.
  if (CPPFile::is_c_or_i_file(filename)) {
    return false;
  }

  // Also, don't include any files specifically forbidden in a .N
  // file.
  if (in_noinclude(filename)) {
    return false;
  }

  // Much as I hate to do it, I'm going to code in a special-case
  // for two particularly nasty header files that we probably don't
  // want to actually ever include.
  if (filename == "winbase.h" || filename == "windows.h") {
    return false;
  }

  // Finally, don't include *_src.h or *_src.cxx.  These are special
  // "template" files that should not generally be included directly.
  if (filename.length() > 6 && filename.substr(filename.length() - 6) == "_src.h") {
    return false;
  }
  if (filename.length() > 8 && filename.substr(filename.length() - 8) == "_src.cxx") {
    return false;
  }

  // Ignore Objective-C files too.
  if (filename.length() > 3 && filename.substr(filename.length() - 3) == ".mm") {
    return false;
  }

  // Otherwise, no problem.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::is_inherited_published
//       Access: Private
//  Description: Recursively looks for the first inherited version of
//               this function in the derivation chain of this class.
//               Returns true if this function is declared published,
//               or false if it is not published, or if it can't be
//               found.
////////////////////////////////////////////////////////////////////
bool InterrogateBuilder::
is_inherited_published(CPPInstance *function, CPPStructType *struct_type) {
  nassertr(struct_type->_derivation.size() == 1, false);
  CPPStructType *base = struct_type->_derivation[0]._base->as_struct_type();
  nassertr(base != (CPPStructType *)NULL, false);

  CPPScope *base_scope = base->get_scope();
  CPPDeclaration *symbol = base_scope->find_symbol(function->get_simple_name(), true);
  if (symbol == (CPPDeclaration *)NULL) {
    // Couldn't find the inherited function.
    return false;
  }

  CPPFunctionGroup *fgroup = symbol->as_function_group();
  if (fgroup == (CPPFunctionGroup *)NULL) {
    // Weird, it wasn't a function.
    return false;
  }

  CPPFunctionGroup::Instances::iterator ii;
  for (ii = fgroup->_instances.begin(); ii != fgroup->_instances.end(); ++ii) {
    CPPInstance *inst = (*ii);
    if (inst->_vis != V_published) {
      // Some flavors of the method are not published.  Take no chances.
      return false;
    }
  }
  // It appears that all flavors of the inherited method are published.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::remap_indices
//       Access: Private
//  Description: Resequences all of the index numbers so that
//               function wrappers start at 1 and occupy consecutive
//               positions, and everything else follows.  This allows
//               us to build a table of function wrappers by index
//               number.
//
//               The "remaps" member is a list of FunctionRemap
//               pointers.  The collision in naming is unfortunate;
//               the FunctionRemap objects are so named because they
//               remap synthesized function wrappers to actual C++
//               methods and functions.  It has nothing to do with the
//               remapping of index numbers, which is the purpose of
//               this function.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
remap_indices(vector<FunctionRemap *> &remaps) {
  IndexRemapper index_remap;
  InterrogateDatabase::get_ptr()->remap_indices(1, index_remap);

  TypesByName::iterator ti;
  for (ti = _types_by_name.begin(); ti != _types_by_name.end(); ++ti) {
    (*ti).second = index_remap.map_from((*ti).second);
  }

  FunctionsByName::iterator fi;
  for (fi = _functions_by_name.begin();
       fi != _functions_by_name.end();
       ++fi) {
    (*fi).second = index_remap.map_from((*fi).second);
  }

  vector<FunctionRemap *>::iterator ri;
  for (ri = remaps.begin(); ri != remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    remap->_wrapper_index = index_remap.map_from(remap->_wrapper_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_function
//       Access: Private
//  Description: Adds the indicated global function to the database,
//               if warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_function(CPPFunctionGroup *fgroup) {
  CPPFunctionGroup::Instances::const_iterator fi;
  for (fi = fgroup->_instances.begin(); fi != fgroup->_instances.end(); ++fi) {
    CPPInstance *function = (*fi);
    scan_function(function);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_function
//       Access: Private
//  Description: Adds the indicated global function to the database,
//               if warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_function(CPPInstance *function) {
  assert(function != (CPPInstance *)NULL);
  assert(function->_type != (CPPType *)NULL &&  function->_type->as_function_type() != (CPPFunctionType *)NULL);

  CPPFunctionType *ftype =  function->_type->resolve_type(&parser, &parser)->as_function_type();
  assert(ftype != (CPPFunctionType *)NULL);

  CPPScope *scope = &parser;
  if (function->is_scoped()) {
    scope = function->get_scope(&parser, &parser);
    if (scope == (CPPScope *)NULL) {
      // Invalid scope.
      nout << "Invalid scope: " << *function->_ident << "\n";
      return;
    }

    if (scope->get_struct_type() != (CPPStructType *)NULL) {
      // Wait, this is a method, not a function.  This must be the
      // declaration for the method (since it's appearing
      // out-of-scope).  We don't need to define a new method for it,
      // but we'd like to update the comment, if we have a comment.
      update_function_comment(function, scope);
      return;
    }
  }

  if (function->is_template()) {
    // The function is a template function, not a true function.
    return;
  }

  if (function->_file.is_c_file()) {
    // This function declaration appears in a .C file.  We can only
    // export functions whose prototypes appear in an .h file.

    string function_name = TypeManager::get_function_name(function);

    // Still, we can update the comment, at least.
    update_function_comment(function, scope);
    return;
  }

  if (function->_file._source != CPPFile::S_local ||
      in_ignorefile(function->_file._filename_as_referenced)) {
    // The function is defined in some other package or in an
    // ignorable file.
    return;
  }

  if (function->_vis > min_vis) {
    // The function is not marked to be exported.
    return;
  }

  if ((function->_storage_class & CPPInstance::SC_static) != 0) {
    // The function is static, so can't be exported.
    return;
  }

  if (TypeManager::involves_protected(ftype)) {
    // We can't export the function because it involves parameter
    // types that are protected or private.
    return;
  }

  if (in_ignoreinvolved(ftype)) {
    // The function or its parameters involves something that the
    // user requested we ignore.
    return;
  }

  get_function(function, "",
               (CPPStructType *)NULL, scope,
               InterrogateFunction::F_global);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_struct_type
//       Access: Private
//  Description: Adds the indicated struct type to the database, if
//               warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_struct_type(CPPStructType *type) {
  if (type == (CPPStructType *)NULL) {
    return;
  }

  if (type->is_template()) {
    // The type is a template declaration, not a true type.
    return;
  }

  if (type->_file.is_c_file()) {
    // This type declaration appears in a .C file.  We can only export
    // types defined in a .h file.
    return;
  }

  if (type->_file._source != CPPFile::S_local ||
      in_ignorefile(type->_file._filename_as_referenced)) {
    // The type is defined in some other package or in an
    // ignorable file.
    return;
  }

  // Check if any of the members are exported.  If none of them are,
  // and the type itself is not marked for export, then never mind.
  if (type->_vis > min_vis) {
    CPPScope *scope = type->_scope;

    bool any_exported = false;
    CPPScope::Declarations::const_iterator di;
    for (di = scope->_declarations.begin();
         di != scope->_declarations.end() && !any_exported;
         ++di) {
      if ((*di)->_vis <= min_vis) {
        any_exported = true;
        break;
      }
    }

    if (!any_exported) {
      return;
    }
  }

  get_type(type, true);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_enum_type
//       Access: Private
//  Description: Adds the indicated enum type to the database, if
//               warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_enum_type(CPPEnumType *type) {
  if (type == (CPPEnumType *)NULL) {
    return;
  }

  if (type->is_template()) {
    // The type is a template declaration, not a true type.
    return;
  }

  if (type->_file.is_c_file()) {
    // This type declaration appears in a .C file.  We can only export
    // types defined in a .h file.
    return;
  }

  if (type->_file._source != CPPFile::S_local ||
      in_ignorefile(type->_file._filename_as_referenced)) {
    // The type is defined in some other package or in an
    // ignorable file.
    return;
  }

  if (type->_vis > min_vis) {
    // The type is not marked to be exported.
    return;
  }

  get_type(type, true);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_typedef_type
//       Access: Private
//  Description: Adds the indicated typedef type to the database, if
//               warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_typedef_type(CPPTypedefType *type) {
  if (type == (CPPTypedefType *)NULL) {
    return;
  }

  // A typedef cannot be a template declaration.
  assert(!type->is_template());

  if (type->_file.is_c_file()) {
    // This type declaration appears in a .C file.  We can only export
    // types defined in a .h file.
    return;
  }

  if (type->_file._source != CPPFile::S_local ||
      in_ignorefile(type->_file._filename_as_referenced)) {
    // The type is defined in some other package or in an
    // ignorable file.
    return;
  }

  // Do we require explicitly placing BEGIN_PUBLISH/END_PUBLISH
  // blocks around typedefs for them to be exported?  My thinking is
  // that we shoudn't, for now, since we don't require it for structs
  // either (we only require it to have published methods).
  //if (type->_vis > min_vis) {
  //  // The wrapped type is not marked to be exported.
  //  return;
  //}

  // Find out what this typedef points to.
  CPPType *wrapped_type = type->_type;
  bool forced = in_forcetype(wrapped_type->get_local_name(&parser));

  while (wrapped_type->get_subtype() == CPPDeclaration::ST_typedef) {
    wrapped_type = wrapped_type->as_typedef_type()->_type;
    forced = forced || in_forcetype(wrapped_type->get_local_name(&parser));
  }

  CPPStructType *struct_type = wrapped_type->as_struct_type();
  if (struct_type == (CPPStructType *)NULL) {
    // We only export typedefs to structs, for now.
    return;
  }

  // Always export typedefs pointing to forced types.
  if (!forced) {
    if (wrapped_type->_file._source != CPPFile::S_local ||
        in_ignorefile(wrapped_type->_file._filename_as_referenced)) {
      // The wrapped type is defined in some other package or
      // in an ignorable file.
      return;
    }

    // Check if any of the wrapped type's members are published.
    // If none of them are, and the wrapped type itself is not
    // marked for export, then never mind.
    if (struct_type->_vis > min_vis) {
      CPPScope *scope = struct_type->_scope;

      bool any_exported = false;
      CPPScope::Declarations::const_iterator di;
      for (di = scope->_declarations.begin();
           di != scope->_declarations.end() && !any_exported;
           ++di) {
        if ((*di)->_vis <= min_vis) {
          any_exported = true;
          break;
        }
      }

      if (!any_exported) {
        return;
      }
    }
  }

  get_type(type, true);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_manifest
//       Access: Private
//  Description: Adds the indicated manifest constant to the database,
//               if warranted.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
scan_manifest(CPPManifest *manifest) {
  if (manifest == (CPPManifest *)NULL) {
    return;
  }

  if (manifest->_file.is_c_file()) {
    // This #define appears in a .C file.  We can only export
    // manifests defined in a .h file.
    return;
  }

  if (manifest->_file._source != CPPFile::S_local ||
      in_ignorefile(manifest->_file._filename_as_referenced)) {
    // The manifest is defined in some other package or in an
    // ignorable file.
    return;
  }

  if (manifest->_vis > min_vis) {
    // The manifest is not marked for export.
    return;
  }

  if (manifest->_has_parameters) {
    // We can't export manifest functions.
    return;
  }

  InterrogateManifest imanifest;
  imanifest._name = manifest->_name;
  imanifest._definition = manifest->expand();

  CPPType *type = manifest->determine_type();
  if (type != (CPPType *)NULL) {
    imanifest._flags |= InterrogateManifest::F_has_type;
    imanifest._type = get_type(type, false);

    CPPExpression *expr = manifest->_expr;
    CPPExpression::Result result = expr->evaluate();
    if (result._type == CPPExpression::RT_integer) {
      // We have an integer-valued expression.
      imanifest._flags |= InterrogateManifest::F_has_int_value;
      imanifest._int_value = result.as_integer();

    } else {
      // We have a more complex expression.  Generate a getter
      // function.
      FunctionIndex getter =
        get_getter(type, manifest->_name, (CPPStructType *)NULL, &parser,
                   (CPPInstance *)NULL);

      if (getter != 0) {
        imanifest._flags |= InterrogateManifest::F_has_getter;
        imanifest._getter = getter;
      }
    }
  }

  ManifestIndex index =
    InterrogateDatabase::get_ptr()->get_next_index();
  InterrogateDatabase::get_ptr()->add_manifest(index, imanifest);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::scan_element
//       Access: Private
//  Description: Adds the indicated data element to the database,
//               if warranted.
////////////////////////////////////////////////////////////////////
ElementIndex InterrogateBuilder::
scan_element(CPPInstance *element, CPPStructType *struct_type,
             CPPScope *scope) {
  if (element == (CPPInstance *)NULL) {
    return 0;
  }

  if (element->is_template()) {
    // The element is a template element, not a true element.
    return 0;
  }

  if (element->is_scoped()) {
    if (element->get_scope(scope, &parser) != scope) {
      // This is an element defined out-of-scope.  It's probably the
      // definition for a data member.  Ignore it.
      return 0;
    }
  }

  if (element->_file.is_c_file()) {
    // This element declaration appears in a .C file.  We can only
    // export elements declared in a .h file.
    return 0;
  }

  if (element->_file._source != CPPFile::S_local ||
      in_ignorefile(element->_file._filename_as_referenced)) {
    // The element is defined in some other package or in an
    // ignorable file.
    return 0;
  }

  if (element->_vis > min_vis) {
    // The element is not marked for export.
    return 0;
  }

  if ((element->_storage_class & CPPInstance::SC_static) != 0) {
    // The element is static, so can't be exported.
    return 0;
  }

  // Make sure the element knows what its scope is.
  if (element->_ident->_native_scope != scope) {
    element = new CPPInstance(*element);
    element->_ident = new CPPIdentifier(*element->_ident);
    element->_ident->_native_scope = scope;
  }

  CPPType *element_type = TypeManager::resolve_type(element->_type, scope);
  CPPType *parameter_type = element_type;

  InterrogateElement ielement;
  ielement._name = element->get_local_name(scope);
  ielement._scoped_name = descope(element->get_local_name(&parser));

  // See if there happens to be a comment before the element.
  if (element->_leading_comment != (CPPCommentBlock *)NULL) {
    ielement._comment = trim_blanks(element->_leading_comment->_comment);
  }

  ielement._type = get_type(TypeManager::unwrap_reference(element_type), false);
  if (ielement._type == 0) {
    // If we can't understand what type it is, forget it.
    return 0;
  }

  if (!TypeManager::involves_protected(element_type)) {
    // We can only generate a getter and a setter if we can talk about
    // the type it is.

    if (parameter_type->as_struct_type() != (CPPStructType *)NULL) {
      // Wrap the type in a const reference.
      parameter_type = TypeManager::wrap_const_reference(parameter_type);
    }

    // Generate a getter and setter function for the element.
    FunctionIndex getter =
      get_getter(parameter_type, element->get_local_name(scope),
                 struct_type, scope, element);
    if (getter != 0) {
      ielement._flags |= InterrogateElement::F_has_getter;
      ielement._getter = getter;
    }

    if (TypeManager::is_assignable(element_type)) {
      FunctionIndex setter =
        get_setter(parameter_type, element->get_local_name(scope),
                   struct_type, scope, element);
      if (setter != 0) {
        ielement._flags |= InterrogateElement::F_has_setter;
        ielement._setter = setter;
      }
    }
  }

  if (struct_type == (CPPStructType *)NULL) {
    // This is a global data element: not a data member.
    ielement._flags |= InterrogateElement::F_global;
  }

  ElementIndex index =
    InterrogateDatabase::get_ptr()->get_next_index();
  InterrogateDatabase::get_ptr()->add_element(index, ielement);

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_getter
//       Access: Private
//  Description: Adds a function to return the value for the indicated
//               expression.  Returns the new function index.
////////////////////////////////////////////////////////////////////
FunctionIndex InterrogateBuilder::
get_getter(CPPType *expr_type, string expression,
           CPPStructType *struct_type, CPPScope *scope,
           CPPInstance *element) {
  // Make up a name for the function.
  string fname = clean_identifier("get_" + expression);

  // Unroll the "const" from the expr_type, since that doesn't matter
  // for a return type.
  while (expr_type->as_const_type() != (CPPConstType *)NULL) {
    expr_type = expr_type->as_const_type()->_wrapped_around;
    assert(expr_type != (CPPType *)NULL);
  }

  // We can't return an array from a function, but we can decay it
  // into a pointer.
  while (expr_type->get_subtype() == CPPDeclaration::ST_array) {
    expr_type = CPPType::new_type(new CPPPointerType(expr_type->as_array_type()->_element_type));
  }

  // Make up a CPPFunctionType.
  CPPParameterList *params = new CPPParameterList;
  CPPFunctionType *ftype = new CPPFunctionType(expr_type, params, 0);

  // Now make up an instance for the function.
  CPPInstance *function = new CPPInstance(ftype, fname);
  function->_ident->_native_scope = scope;

  int getter_flags = InterrogateFunction::F_getter;

  if (struct_type != (CPPStructType *)NULL) {
    // This is a data member for some class.
    assert(element != (CPPInstance *)NULL);
    assert(scope != (CPPScope *)NULL);

    if ((element->_storage_class & CPPInstance::SC_static) != 0) {
      // This is a static data member; therefore, the synthesized
      // getter is also static.
      function->_storage_class |= CPPInstance::SC_static;

      // And the expression is fully scoped.
      expression = element->get_local_name(&parser);

    } else {
      // This is a non-static data member, so it has a const
      // synthesized getter method.
      ftype->_flags |= CPPFunctionType::F_const_method;

      // And the expression is locally scoped.
      expression = element->get_local_name(scope);
      getter_flags |= InterrogateFunction::F_method;
    }
  }

  // Now check to see if there's already a function matching this
  // name.  If there is, we can't define a getter, and we
  // shouldn't mistake this other function for a synthesized getter.
  string function_name = TypeManager::get_function_name(function);
  if (_functions_by_name.count(function_name) != 0) {
    return 0;
  }

  ostringstream desc;
  desc << "getter for ";
  if (element != (CPPInstance *)NULL) {
    element->_initializer = (CPPExpression *)NULL;
    element->output(desc, 0, &parser, false);
    desc << ";";
  } else {
    desc << expression;
  }
  string description = desc.str();

  // It's clear; make a getter.
  FunctionIndex index =
    get_function(function, description,
                 struct_type, scope,
                 getter_flags, expression);

  InterrogateDatabase::get_ptr()->update_function(index)._comment = description;
  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_setter
//       Access: Private
//  Description: Adds a function to return the value for the indicated
//               expression.  Returns the new function index.
////////////////////////////////////////////////////////////////////
FunctionIndex InterrogateBuilder::
get_setter(CPPType *expr_type, string expression,
           CPPStructType *struct_type, CPPScope *scope,
           CPPInstance *element) {
  // Make up a name for the function.
  string fname = clean_identifier("set_" + expression);

  // Make up a CPPFunctionType.
  CPPParameterList *params = new CPPParameterList;
  CPPInstance *param0 = new CPPInstance(expr_type, "value");
  params->_parameters.push_back(param0);
  CPPType *void_type = TypeManager::get_void_type();
  CPPFunctionType *ftype = new CPPFunctionType(void_type, params, 0);

  // Now make up an instance for the function.
  CPPInstance *function = new CPPInstance(ftype, fname);
  function->_ident->_native_scope = scope;

  int setter_flags = InterrogateFunction::F_setter;

  if (struct_type != (CPPStructType *)NULL) {
    // This is a data member for some class.
    assert(element != (CPPInstance *)NULL);
    assert(scope != (CPPScope *)NULL);

    if ((element->_storage_class & CPPInstance::SC_static) != 0) {
      // This is a static data member; therefore, the synthesized
      // setter is also static.
      function->_storage_class |= CPPInstance::SC_static;

      // And the expression is fully scoped.
      expression = element->get_local_name(&parser);

    } else {
      // This is a non-static data member.  The expression is locally
      // scoped.
      expression = element->get_local_name(scope);
      setter_flags |= InterrogateFunction::F_method;
    }
  }

  // Now check to see if there's already a function matching this
  // name.  If there is, we can't define a setter, and we
  // shouldn't mistake this other function for a synthesized setter.
  string function_name = TypeManager::get_function_name(function);
  if (_functions_by_name.count(function_name) != 0) {
    return 0;
  }

  ostringstream desc;
  desc << "setter for ";
  if (element != (CPPInstance *)NULL) {
    element->_initializer = (CPPExpression *)NULL;
    element->output(desc, 0, &parser, false);
    desc << ";";
  } else {
    desc << expression;
  }
  string description = desc.str();

  // It's clear; make a setter.
  FunctionIndex index =
    get_function(function, description,
                 struct_type, scope,
                 setter_flags, expression);

  InterrogateDatabase::get_ptr()->update_function(index)._comment = description;
  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_cast_function
//       Access: Private
//  Description: Adds a function to cast from a pointer of the
//               indicated type to a pointer of the indicated type to
//               the database.  Returns the new function index.
////////////////////////////////////////////////////////////////////
FunctionIndex InterrogateBuilder::
get_cast_function(CPPType *to_type, CPPType *from_type,
                  const string &prefix) {
  CPPInstance *function;
  CPPStructType *struct_type = from_type->as_struct_type();
  CPPScope *scope = &parser;

  if (struct_type != (CPPStructType *)NULL) {
    // We'll make this a method of the from type.
    scope = struct_type->get_scope();

    // Make up a name for the method.
    string fname =
      clean_identifier(prefix + "_to_" + get_preferred_name(to_type));

    // Make up a CPPFunctionType.
    CPPType *to_ptr_type = CPPType::new_type(new CPPPointerType(to_type));

    CPPParameterList *params = new CPPParameterList;
    CPPFunctionType *ftype = new CPPFunctionType(to_ptr_type, params, 0);

    // Now make up an instance for the function.
    function = new CPPInstance(ftype, fname);

  } else {
    // The from type isn't a struct or a class, so this has to be an
    // external function.

    // Make up a name for the function.
    string fname =
      clean_identifier(prefix + "_" + get_preferred_name(from_type) +
                       "_to_" + get_preferred_name(to_type));

    // Make up a CPPFunctionType.
    CPPType *from_ptr_type = CPPType::new_type(new CPPPointerType(from_type));
    CPPType *to_ptr_type = CPPType::new_type(new CPPPointerType(to_type));

    CPPInstance *param0 = new CPPInstance(from_ptr_type, "this");
    CPPParameterList *params = new CPPParameterList;
    params->_parameters.push_back(param0);
    CPPFunctionType *ftype = new CPPFunctionType(to_ptr_type, params, 0);

    // Now make up an instance for the function.
    function = new CPPInstance(ftype, fname);
  }

  ostringstream desc;
  desc << prefix << " from " << *from_type << " to " << *to_type;
  string description = desc.str();

  FunctionIndex index =
    get_function(function, description,
                 struct_type, scope,
                 InterrogateFunction::F_typecast);

  InterrogateDatabase::get_ptr()->update_function(index)._comment = description;
  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_function
//       Access: Private
//  Description: Adds the indicated function to the database, if it is
//               not already present.  In either case, returns the
//               FunctionIndex of the function within the database.
////////////////////////////////////////////////////////////////////
FunctionIndex InterrogateBuilder::
get_function(CPPInstance *function, string description,
             CPPStructType *struct_type,
             CPPScope *scope, int flags,
             const string &expression) {

  // Get a unique function signature.  Make sure we tell the function
  // where its native scope is, so we get a fully-scoped signature.

  if (function->_ident->_native_scope != scope) {
    function = new CPPInstance(*function);
    function->_ident = new CPPIdentifier(*function->_ident);
    function->_ident->_native_scope = scope;
  }
  CPPFunctionType *ftype =
    function->_type->resolve_type(scope, &parser)->as_function_type();
  function->_type = ftype;

  if ((ftype->_flags & CPPFunctionType::F_constructor) &&
      struct_type != (CPPStructType *)NULL &&
      struct_type->is_abstract()) {
    // This is a constructor for an abstract class; forget it.
    return 0;
  }

  TypeIndex class_index = 0;
  if (struct_type != (CPPStructType *)NULL) {
    class_index = get_type(struct_type, false);
  }

  string function_name = TypeManager::get_function_name(function);
  string function_signature = TypeManager::get_function_signature(function);

  if (ftype->_flags & CPPFunctionType::F_unary_op) {
    // This is a unary operator function.  Name it differently so we
    // don't consider it an overloaded version of a similarly-named
    // binary operator.
    function_name += "unary";
  }

  // First, check to see if it's already there.
  FunctionsByName::const_iterator tni =
    _functions_by_name.find(function_name);
  if (tni != _functions_by_name.end()) {
    FunctionIndex index = (*tni).second;

    // It's already here, so update the flags.
    InterrogateFunction &ifunction =
      InterrogateDatabase::get_ptr()->update_function(index);

    nassertr(&ifunction != NULL, 0);

    ifunction._flags |= flags;

    // Also, make sure this particular signature is defined.
    pair<InterrogateFunction::Instances::iterator, bool> result =
      ifunction._instances->insert(InterrogateFunction::Instances::value_type(function_signature, function));

    InterrogateFunction::Instances::iterator ii = result.first;
    bool inserted = result.second;

    if (inserted) {
      // If we just added the new signature, append the prototype.
      ostringstream prototype;
      function->output(prototype, 0, &parser, false);
      prototype << ";";
      ifunction._prototype += "\n" + prototype.str();
    }

    // Also set the comment.
    if (function->_leading_comment != (CPPCommentBlock *)NULL) {
      string comment = trim_blanks(function->_leading_comment->_comment);
      if (!ifunction._comment.empty()) {
        ifunction._comment += "\n\n";
      }
      ifunction._comment += comment;

      // And update the particular wrapper comment.
      if ((*ii).second->_leading_comment == NULL ||
          function->_leading_comment->_comment.length() >
          (*ii).second->_leading_comment->_comment.length()) {
        (*ii).second->_leading_comment = function->_leading_comment;
      }
    }

    return index;
  }

  // It isn't here, so we'll have to define it.
  FunctionIndex index =
    InterrogateDatabase::get_ptr()->get_next_index();
  _functions_by_name[function_name] = index;

  InterrogateFunction *ifunction = new InterrogateFunction;
  ifunction->_name = function->get_local_name(scope);
  ifunction->_scoped_name = descope(function->get_local_name(&parser));
  ifunction->_instances = new InterrogateFunction::Instances;

  if (function->_leading_comment != (CPPCommentBlock *)NULL) {
    ifunction->_comment = trim_blanks(function->_leading_comment->_comment);
  }

  ostringstream prototype;
  function->output(prototype, 0, &parser, false);
  prototype << ";";
  ifunction->_prototype = prototype.str();

  if (struct_type != (CPPStructType *)NULL) {
    // The function is a method.
    ifunction->_flags |= InterrogateFunction::F_method;
    ifunction->_class = class_index;
  }

  if (ftype->_flags & CPPFunctionType::F_unary_op) {
    // This is a special unary function.
    ifunction->_flags |= InterrogateFunction::F_unary_op;
  }

  if (ftype->_flags & CPPFunctionType::F_operator_typecast) {
    // This is a special typecast operator.
    ifunction->_flags |= InterrogateFunction::F_operator_typecast;
  }

  if (function->_storage_class & CPPInstance::SC_virtual) {
    // This is a virtual function.
    ifunction->_flags |= InterrogateFunction::F_virtual;
  }

  ifunction->_flags |= flags;
  ifunction->_instances->insert(InterrogateFunction::Instances::value_type(function_signature, function));
  ifunction->_expression = expression;

  InterrogateDatabase::get_ptr()->add_function(index, ifunction);

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_make_property
//       Access: Private
//  Description: Adds the indicated make_property to the database,
//               if it is not already present.  In either case,
//               returns the MakeSeqIndex of the make_seq within the
//               database.
////////////////////////////////////////////////////////////////////
ElementIndex InterrogateBuilder::
get_make_property(CPPMakeProperty *make_property, CPPStructType *struct_type) {
  string property_name = make_property->get_local_name(&parser);

  // First, check to see if it's already there.
  PropertiesByName::const_iterator tni =
    _properties_by_name.find(property_name);
  if (tni != _properties_by_name.end()) {
    ElementIndex index = (*tni).second;
    return index;
  }

  // Find the getter so we can get its return type.
  CPPInstance *getter = NULL;
  CPPType *return_type = NULL;

  CPPFunctionGroup *fgroup = make_property->_getter;
  if (fgroup != NULL) {
    CPPFunctionGroup::Instances::const_iterator fi;
    for (fi = fgroup->_instances.begin(); fi != fgroup->_instances.end(); ++fi) {
      CPPInstance *function = (*fi);
      CPPFunctionType *ftype =
        function->_type->as_function_type();
      if (ftype != NULL && ftype->_parameters->_parameters.size() == 0) {
        getter = function;
        return_type = ftype->_return_type;

        // The return type of the non-const method probably better represents
        // the type of the property we are creating.
        if ((ftype->_flags & CPPFunctionType::F_const_method) == 0) {
          break;
        }
      }
    }

    if (getter == NULL || return_type == NULL) {
      cerr << "No instance of getter '"
           << fgroup->_name << "' is suitable!\n";
      return 0;
    }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  // It isn't here, so we'll have to define it.
  ElementIndex index = idb->get_next_index();
  _properties_by_name[property_name] = index;

  InterrogateElement iproperty;
  iproperty._name = make_property->get_simple_name();
  iproperty._scoped_name = descope(make_property->get_local_name(&parser));

  if (return_type != NULL) {
    iproperty._type = get_type(return_type, false);
  } else {
    iproperty._type = 0;
  }

  if (getter != NULL) {
    iproperty._flags |= InterrogateElement::F_has_getter;
    iproperty._getter = get_function(getter, "", struct_type,
                                     struct_type->get_scope(), 0);
  }

  // See if there happens to be a comment before the MAKE_PROPERTY macro.
  if (make_property->_leading_comment != (CPPCommentBlock *)NULL) {
    iproperty._comment = trim_blanks(make_property->_leading_comment->_comment);
  }

  // Now look for setters.
  fgroup = make_property->_setter;
  if (fgroup != NULL) {
    CPPFunctionGroup::Instances::const_iterator fi;
    for (fi = fgroup->_instances.begin(); fi != fgroup->_instances.end(); ++fi) {
      CPPInstance *function = (*fi);
      iproperty._flags |= InterrogateElement::F_has_setter;
      iproperty._setter = get_function(function, "", struct_type,
                                       struct_type->get_scope(), 0);
    }
  }

  idb->add_element(index, iproperty);

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_make_seq
//       Access: Private
//  Description: Adds the indicated make_seq to the database, if it is
//               not already present.  In either case, returns the
//               MakeSeq of the make_seq within the database.
////////////////////////////////////////////////////////////////////
MakeSeqIndex InterrogateBuilder::
get_make_seq(CPPMakeSeq *make_seq, CPPStructType *struct_type) {
  string make_seq_name = struct_type->get_local_name(&parser) + "::MakeSeq_" + make_seq->_seq_name;

  // First, check to see if it's already there.
  MakeSeqsByName::const_iterator tni =
    _make_seqs_by_name.find(make_seq_name);
  if (tni != _make_seqs_by_name.end()) {
    MakeSeqIndex index = (*tni).second;
    return index;
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  // It isn't here, so we'll have to define it.
  MakeSeqIndex index = idb->get_next_index();
  _make_seqs_by_name[make_seq_name] = index;

  InterrogateMakeSeq imake_seq;
  imake_seq._class = get_type(struct_type, false);
  imake_seq._seq_name = make_seq->_seq_name;
  imake_seq._num_name = make_seq->_num_name;
  imake_seq._element_name = make_seq->_element_name;

  idb->add_make_seq(index, imake_seq);

  InterrogateType &itype = idb->update_type(imake_seq._class);
  itype._make_seqs.push_back(index);

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_atomic_string_type
//       Access: Private
//  Description: Returns a TypeIndex for the "atomic string" type,
//               which is a bogus type that might be used if -string
//               is passed to interrogate.  It means to translate
//               basic_string<char> and char * to whatever atomic
//               string type is native to the particular the scripting
//               language we happen to be generating wrappers for.
////////////////////////////////////////////////////////////////////
TypeIndex InterrogateBuilder::
get_atomic_string_type() {
  // Make up a true name that can't possibly clash with an actual C++
  // type name.
  string true_name = "atomic string";

  TypesByName::const_iterator tni = _types_by_name.find(true_name);
  if (tni != _types_by_name.end()) {
    return (*tni).second;
  }

  // This is the first time the atomic string has been requested;
  // define it now.

  TypeIndex index = InterrogateDatabase::get_ptr()->get_next_index();
  _types_by_name[true_name] = index;

  InterrogateType itype;
  itype._flags |= InterrogateType::F_atomic;
  itype._atomic_token = AT_string;
  itype._true_name = true_name;
  itype._scoped_name = true_name;
  itype._name = true_name;

  InterrogateDatabase::get_ptr()->add_type(index, itype);

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::get_type
//       Access: Private
//  Description: Adds the indicated type to the database, if it is not
//               already present.  In either case, returns the
//               TypeIndex of the type within the database.
////////////////////////////////////////////////////////////////////
TypeIndex InterrogateBuilder::
get_type(CPPType *type, bool global) {
  if (type->is_template()) {
    // Can't do anything with a template type.
    return 0;
  }

  TypeIndex index = 0;

  // First, check to see if it's already there.
  string true_name = type->get_local_name(&parser);

  if (true_name.empty()) {
    // Whoops, it's an anonymous type.  That's okay, because we'll
    // usually only encounter them once anyway, so let's go ahead and
    // define it without checking _types_by_name first.

  } else {
    TypesByName::const_iterator tni = _types_by_name.find(true_name);
    if (tni != _types_by_name.end()) {
      // It's already here, so update the global flag.
      index = (*tni).second;
      if (index == 0) {
        // This is an invalid type; we don't know anything about it.
        return 0;
      }

      InterrogateType &itype = InterrogateDatabase::get_ptr()->update_type(index);
      if (global) {
        itype._flags |= InterrogateType::F_global;
      }

      if ((itype._flags & InterrogateType::F_fully_defined) != 0) {
        return index;
      }

      // But wait--it's not fully defined yet!  We'll go ahead and
      // define it now.
    }
  }

  bool forced = in_forcetype(true_name);

  if (index == 0) {
    // It isn't already there, so we have to define it.
    index = InterrogateDatabase::get_ptr()->get_next_index();
    if (!true_name.empty()) {
      _types_by_name[true_name] = index;
    }

    InterrogateType itype;
    if (global) {
      itype._flags |= InterrogateType::F_global;
    }

    InterrogateDatabase::get_ptr()->add_type(index, itype);
  }

  InterrogateType &itype =
    InterrogateDatabase::get_ptr()->update_type(index);

  itype._name = get_preferred_name(type);

  int num_alt_names = type->get_num_alt_names();
  if (num_alt_names != 0) {
    itype._alt_names.clear();
    for (int i = 0; i < num_alt_names; ++i) {
      string alt_name = type->get_alt_name(i);
      itype._alt_names.push_back(alt_name);
    }
  }

  itype._scoped_name = true_name;
  itype._true_name = true_name;
  itype._cpptype = type;

  if (type->_declaration != (CPPTypeDeclaration *)NULL) {
    // This type has a declaration; does the declaration have a
    // comment?
    CPPTypeDeclaration *decl = type->_declaration;
    if (decl->_leading_comment != (CPPCommentBlock *)NULL) {
      itype._comment = trim_blanks(decl->_leading_comment->_comment);
    }
  }

  CPPScope *scope = NULL;
  // If it's an extension type or typedef, it might be scoped.
  if (CPPTypedefType *td_type = type->as_typedef_type()) {
    scope = td_type->_ident->get_scope(&parser, &parser);

  } else if (CPPExtensionType *ext_type = type->as_extension_type()) {
    if (ext_type->_ident != (CPPIdentifier *)NULL) {
      scope = ext_type->_ident->get_scope(&parser, &parser);

    } else if (CPPEnumType *enum_type = ext_type->as_enum_type()) {
      // Special case for anonymous enums.
      scope = enum_type->_parent_scope;
    }

  }

  if (scope != (CPPScope *)NULL) {
    while (scope->as_template_scope() != (CPPTemplateScope *)NULL) {
      assert(scope->get_parent_scope() != scope);
      scope = scope->get_parent_scope();
      assert(scope != (CPPScope *)NULL);
    }
    itype._cppscope = scope;

    if (scope != &parser) {
      // We're scoped!
      itype._scoped_name =
        descope(scope->get_local_name(&parser) + "::" + itype._name);
      CPPStructType *struct_type = scope->get_struct_type();

      if (struct_type != (CPPStructType *)NULL) {
        itype._flags |= InterrogateType::F_nested;
        itype._outer_class = get_type(struct_type, false);
      }
    }
  }

  if (forced || !in_ignoretype(true_name)) {
    itype._flags |= InterrogateType::F_fully_defined;

    if (type->as_simple_type() != (CPPSimpleType *)NULL) {
      define_atomic_type(itype, type->as_simple_type());

    } else if (type->as_pointer_type() != (CPPPointerType *)NULL) {
      define_wrapped_type(itype, type->as_pointer_type());

    } else if (type->as_const_type() != (CPPConstType *)NULL) {
      define_wrapped_type(itype, type->as_const_type());

    } else if (type->as_struct_type() != (CPPStructType *)NULL) {
      define_struct_type(itype, type->as_struct_type(), index, forced);

    } else if (type->as_enum_type() != (CPPEnumType *)NULL) {
      define_enum_type(itype, type->as_enum_type());

    } else if (type->as_extension_type() != (CPPExtensionType *)NULL) {
      define_extension_type(itype, type->as_extension_type());

    } else if (type->as_typedef_type() != (CPPTypedefType *)NULL) {
      define_typedef_type(itype, type->as_typedef_type());

    } else if (type->as_array_type() != (CPPArrayType *)NULL) {
      define_array_type(itype, type->as_array_type());

    } else {
      nout << "Attempt to define invalid type " << *type
           << " (subtype " << type->get_subtype() << ")\n";

      // Remove the type from the database.
      InterrogateDatabase::get_ptr()->remove_type(index);
      if (!true_name.empty()) {
        _types_by_name[true_name] = 0;
      }
      index = 0;
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_atomic_type
//       Access: Private
//  Description: Builds up a definition for the indicated atomic type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_atomic_type(InterrogateType &itype, CPPSimpleType *cpptype) {
  itype._flags |= InterrogateType::F_atomic;

  switch (cpptype->_type) {
  case CPPSimpleType::T_bool:
    itype._atomic_token = AT_bool;
    break;

  case CPPSimpleType::T_char:
    itype._atomic_token = AT_char;
    break;

  case CPPSimpleType::T_wchar_t:
    itype._atomic_token = AT_int;
    break;

  case CPPSimpleType::T_char16_t:
    itype._flags |= InterrogateType::F_unsigned;
    itype._atomic_token = AT_int;
    break;

  case CPPSimpleType::T_char32_t:
    itype._flags |= InterrogateType::F_unsigned;
    itype._atomic_token = AT_int;
    break;

  case CPPSimpleType::T_int:
    if ((cpptype->_flags & CPPSimpleType::F_longlong) != 0) {
      itype._atomic_token = AT_longlong;
    } else {
      itype._atomic_token = AT_int;
    }
    break;

  case CPPSimpleType::T_float:
    itype._atomic_token = AT_float;
    break;

  case CPPSimpleType::T_double:
    itype._atomic_token = AT_double;
    break;

  case CPPSimpleType::T_void:
    itype._atomic_token = AT_void;
    break;

  default:
    nout << "Type \"" << *cpptype << "\" has invalid CPPSimpleType: "
         << (int)cpptype->_type << "\n";
    itype._atomic_token = AT_not_atomic;
  }

  if ((cpptype->_flags & CPPSimpleType::F_longlong) != 0) {
    itype._flags |= InterrogateType::F_longlong;

  } else if ((cpptype->_flags & CPPSimpleType::F_long) != 0) {
    itype._flags |= InterrogateType::F_long;
  }

  if ((cpptype->_flags & CPPSimpleType::F_short) != 0) {
    itype._flags |= InterrogateType::F_short;
  }
  if ((cpptype->_flags & CPPSimpleType::F_unsigned) != 0) {
    itype._flags |= InterrogateType::F_unsigned;
  }
  if ((cpptype->_flags & CPPSimpleType::F_signed) != 0) {
    itype._flags |= InterrogateType::F_signed;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_wrapped_type
//       Access: Private
//  Description: Builds up a definition for the indicated wrapped type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_wrapped_type(InterrogateType &itype, CPPPointerType *cpptype) {
  itype._flags |= (InterrogateType::F_wrapped | InterrogateType::F_pointer);
  itype._wrapped_type = get_type(cpptype->_pointing_at, false);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_wrapped_type
//       Access: Private
//  Description: Builds up a definition for the indicated wrapped type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_wrapped_type(InterrogateType &itype, CPPConstType *cpptype) {
  itype._flags |= (InterrogateType::F_wrapped | InterrogateType::F_const);
  itype._wrapped_type = get_type(cpptype->_wrapped_around, false);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_struct_type
//       Access: Private
//  Description: Builds up a definition for the indicated struct type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_struct_type(InterrogateType &itype, CPPStructType *cpptype,
                   TypeIndex type_index, bool forced) {
  if (cpptype->get_simple_name().empty()) {
    // If the type has no name, forget it.  We don't export anonymous
    // structs.
    return;
  }

  cpptype = TypeManager::resolve_type(cpptype)->as_struct_type();
  assert(cpptype != (CPPStructType *)NULL);
  bool has_virt_methods = cpptype->check_virtual();

  switch (cpptype->_type) {
  case CPPExtensionType::T_class:
    itype._flags |= InterrogateType::F_class;
    break;

  case CPPExtensionType::T_struct:
    itype._flags |= InterrogateType::F_struct;
    break;

  case CPPExtensionType::T_union:
    itype._flags |= InterrogateType::F_union;
    break;

  default:
    break;
  }

  if (cpptype->_file.is_c_file()) {
    // This type declaration appears in a .C file.  We can only export
    // types defined in a .h file.
    return;
  }

  if (!forced &&
      (cpptype->_file._source != CPPFile::S_local ||
       in_ignorefile(cpptype->_file._filename_as_referenced))) {
    // The struct type is defined in some other package or in an
    // ignorable file, so don't try to output it.

    // This means we also don't gather any information about its
    // derivations or determine if an implicit destructor is
    // necessary.  However, this is not important, and it causes
    // problems if we do (how many implicit destructors do we need,
    // anyway?).
    itype._flags &= ~InterrogateType::F_fully_defined;
    return;
  }

  // Make sure the class declaration within its parent scope isn't
  // private or protected.  If it is, we can't export any of its
  // members.
  if (TypeManager::involves_unpublished(cpptype)) {
    itype._flags &= ~InterrogateType::F_fully_defined;
    itype._flags |= InterrogateType::F_unpublished;
    return;
  }
  if (TypeManager::involves_protected(cpptype)) {
    itype._flags &= ~InterrogateType::F_fully_defined;
    return;
  }

  // A struct type should always be global.
  itype._flags |= InterrogateType::F_global;

  CPPScope *scope = cpptype->_scope;

  CPPStructType::Derivation::const_iterator bi;
  for (bi = cpptype->_derivation.begin();
       bi != cpptype->_derivation.end();
       ++bi) {

    const CPPStructType::Base &base = (*bi);
    if (base._vis <= V_public) {
      CPPType *base_type = TypeManager::resolve_type(base._base, scope);
      TypeIndex base_index = get_type(base_type, true);

      if (base_index == 0) {
        if (base_type != NULL) {
          nout << *cpptype << " reports a derivation from invalid type " << *base_type << ".\n";
        } else {
          nout << *cpptype << " reports a derivation from an invalid type.\n";
        }

      } else {
        InterrogateType::Derivation d;
        d._flags = 0;
        d._base = base_index;
        d._upcast = 0;
        d._downcast = 0;

        // Do we need to synthesize upcast and downcast functions?
        bool generate_casts = false;

        // Function To Generate all castsss Posible..
        if (base._is_virtual) {
          // We do in the presence of virtual inheritance.
          generate_casts = true;

        } else if (bi != cpptype->_derivation.begin()) {
          // Or if we're not talking about the leftmost fork of multiple
          // inheritance.
          generate_casts = true;

        } else if (cpptype->_derivation.size() != 1 &&
                   left_inheritance_requires_upcast) {
          // Or even if we are the leftmost fork of multiple
          // inheritance, if the flag is set indicating that this
          // requires a pointer change.  (For many compilers, this does
          // not require a pointer change.)
          generate_casts = true;

        } else if (has_virt_methods && (base_type->as_struct_type() == (CPPStructType *)NULL || !base_type->as_struct_type()->check_virtual())) {
          // Finally, if this class has virtual methods, but its
          // parent doesn't, then we have to upcast (because this
          // class will require space for a virtual function table
          // pointer, while the parent class won't).
          generate_casts = true;
        }

        if (generate_casts) {
          d._upcast = get_cast_function(base_type, cpptype, "upcast");
          d._flags |= InterrogateType::DF_upcast;

          if (base._is_virtual) {
            // If this is a virtual inheritance, we can't write a
            // downcast.
            d._flags |= InterrogateType::DF_downcast_impossible;
          } else {
            d._downcast = get_cast_function(cpptype, base_type, "downcast");
            d._flags |= InterrogateType::DF_downcast;
          }
        }

        itype._derivations.push_back(d);
      }
    }
  }

  CPPScope::Declarations::const_iterator di;
  for (di = scope->_declarations.begin();
       di != scope->_declarations.end();
       ++di) {

    if ((*di)->get_subtype() == CPPDeclaration::ST_instance) {
      CPPInstance *inst = (*di)->as_instance();
      if (inst->_type->get_subtype() == CPPDeclaration::ST_function) {
        // Here's a method declaration.
        define_method(inst, itype, cpptype, scope);

      } else {
        // Here's a data member declaration.
        ElementIndex data_member = scan_element(inst, cpptype, scope);
        if (data_member != 0) {
          itype._elements.push_back(data_member);
        }
      }

    } else if ((*di)->get_subtype() == CPPDeclaration::ST_type_declaration) {
      CPPType *type = (*di)->as_type_declaration()->_type;

      if ((*di)->_vis <= min_vis || in_forcetype(type->get_local_name(&parser))) {
        if (type->as_struct_type() != (CPPStructType *)NULL ||
            type->as_enum_type() != (CPPEnumType *)NULL) {
          // Here's a nested class or enum definition.
          type->_vis = (*di)->_vis;

          CPPExtensionType *nested_type = type->as_extension_type();
          assert(nested_type != (CPPExtensionType *)NULL);

          // For now, we don't allow anonymous structs.
          if (nested_type->_ident != (CPPIdentifier *)NULL ||
              nested_type->as_enum_type() != (CPPEnumType *)NULL) {
            TypeIndex nested_index = get_type(nested_type, false);
            itype._nested_types.push_back(nested_index);
          }
        }
      }
    } else if ((*di)->get_subtype() == CPPDeclaration::ST_enum) {
      CPPType *type = (*di)->as_enum_type();

      // An anonymous enum type.
      if (type->_vis <= min_vis) {
        TypeIndex nested_index = get_type(type, false);
        itype._nested_types.push_back(nested_index);
      }

    } else if ((*di)->get_subtype() == CPPDeclaration::ST_typedef) {
      CPPTypedefType *type = (*di)->as_typedef_type();

      // A nested typedef.  Unwrap it to find out what it's pointing to.
      CPPType *wrapped_type = type->_type;

      while (wrapped_type->get_subtype() == CPPDeclaration::ST_typedef) {
        wrapped_type = wrapped_type->as_typedef_type()->_type;
      }

      CPPStructType *struct_type = wrapped_type->as_struct_type();
      if (struct_type != (CPPStructType *)NULL) {
        // We only export typedefs to structs, for now.

        if (type->_vis <= min_vis) {
          TypeIndex nested_index = get_type(type, false);
          itype._nested_types.push_back(nested_index);
        }
      }

    } else if ((*di)->get_subtype() == CPPDeclaration::ST_make_property) {
      ElementIndex element_index = get_make_property((*di)->as_make_property(), cpptype);
      itype._elements.push_back(element_index);
    }
  }

  if ((itype._flags & InterrogateType::F_inherited_destructor) != 0) {
    // If we have inherited our virtual destructor from our base
    // class, go ahead and assign the same function index.
    assert(!itype._derivations.empty());
    TypeIndex base_type_index = itype._derivations.front()._base;
    InterrogateType &base_type = InterrogateDatabase::get_ptr()->
      update_type(base_type_index);

    itype._destructor = base_type._destructor;

  } else if ((itype._flags &
              (InterrogateType::F_true_destructor |
               InterrogateType::F_private_destructor |
               InterrogateType::F_inherited_destructor |
               InterrogateType::F_implicit_destructor)) == 0) {
    // If we didn't get a destructor at all, we should make a wrapper
    // for one anyway.
    string function_name = "~" + cpptype->get_simple_name();

    // Make up a CPPFunctionType.
    CPPType *void_type = TypeManager::get_void_type();
    CPPParameterList *params = new CPPParameterList;
    CPPFunctionType *ftype = new CPPFunctionType(void_type, params, 0);
    ftype->_flags |= CPPFunctionType::F_destructor;

    // Now make up an instance for the destructor.
    CPPInstance *function = new CPPInstance(ftype, function_name);

    itype._destructor = get_function(function, "",
                                     cpptype, cpptype->get_scope(),
                                     0);
    itype._flags |= InterrogateType::F_implicit_destructor;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::update_function_comment
//       Access: Private
//  Description: Updates the function definition in the database to
//               include whatever comment is associated with this
//               declaration.  This is called when we encounted a
//               method definition outside of the class or function
//               definition in a C++ file; the only new information
//               this might include for us is the comment.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
update_function_comment(CPPInstance *function, CPPScope *scope) {
  if (function->_leading_comment == (CPPCommentBlock *)NULL) {
    // No comment anyway.  Forget it.
    return;
  }

  // Get a function name so we can look this method up.
  if (function->_ident->_native_scope != scope) {
    function = new CPPInstance(*function);
    function->_ident = new CPPIdentifier(*function->_ident);
    function->_ident->_native_scope = scope;
  }
  CPPFunctionType *ftype =
    function->_type->resolve_type(scope, &parser)->as_function_type();
  function->_type = ftype;

  string function_name = TypeManager::get_function_name(function);
  string function_signature = TypeManager::get_function_signature(function);

  if (ftype->_flags & CPPFunctionType::F_unary_op) {
    // This is a unary operator function.  Name it differently so we
    // don't consider it an overloaded version of a similarly-named
    // binary operator.
    function_name += "unary";
  }

  // Now look it up.
  FunctionsByName::const_iterator tni =
    _functions_by_name.find(function_name);
  if (tni != _functions_by_name.end()) {
    FunctionIndex index = (*tni).second;

    // Here it is!
    InterrogateFunction &ifunction =
      InterrogateDatabase::get_ptr()->update_function(index);

    // Update the comment.
    string comment = trim_blanks(function->_leading_comment->_comment);
    if (!ifunction._comment.empty()) {
      ifunction._comment += "\n\n";
    }
    ifunction._comment += comment;

    // Also update the particular wrapper comment.
    InterrogateFunction::Instances::iterator ii =
      ifunction._instances->find(function_signature);
    if (ii != ifunction._instances->end()) {
      if ((*ii).second->_leading_comment == NULL ||
          function->_leading_comment->_comment.length() >
          (*ii).second->_leading_comment->_comment.length()) {
        (*ii).second->_leading_comment = function->_leading_comment;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_method
//       Access: Private
//  Description: Adds the indicated member function to the struct type,
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_method(CPPFunctionGroup *fgroup, InterrogateType &itype,
              CPPStructType *struct_type, CPPScope *scope) {
  CPPFunctionGroup::Instances::const_iterator fi;
  for (fi = fgroup->_instances.begin(); fi != fgroup->_instances.end(); ++fi) {
    CPPInstance *function = (*fi);
    define_method(function, itype, struct_type, scope);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_method
//       Access: Private
//  Description: Adds the indicated member function to the struct type,
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_method(CPPInstance *function, InterrogateType &itype,
              CPPStructType *struct_type, CPPScope *scope) {
  assert(function != (CPPInstance *)NULL);
  assert(function->_type != (CPPType *)NULL &&
         function->_type->as_function_type() != (CPPFunctionType *)NULL);
  CPPFunctionType *ftype =
    function->_type->resolve_type(scope, &parser)->as_function_type();

  if (function->is_template()) {
    // The function is a template function, not a true function.
    return;
  }

  // As a special kludgey extension, we consider a public static
  // method called "get_class_type()" to be marked published, even if
  // it is not.  This allows us to export all of the TypeHandle system
  // stuff without having to specifically flag get_class_type() as
  // published.
  bool force_publish = false;
  if (function->get_simple_name() == "get_class_type" &&
      (function->_storage_class && CPPInstance::SC_static) != 0 &&
      function->_vis <= V_public) {
    force_publish = true;
  }

  if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
    // A destructor is a special case.  If it's public, we export it
    // (even if it's not published), but if it's protected or private,
    // we don't export it, and we flag it so we don't try to
    // synthesize one later.
    if (function->_vis > V_public) {
      itype._flags |= InterrogateType::F_private_destructor;
      return;
    }
    force_publish = true;
  }

  if (!force_publish && function->_vis > min_vis) {
    // The function is not marked to be exported.
    return;
  }

  if (TypeManager::involves_protected(ftype)) {
    // We can't export the function because it involves parameter
    // types that are protected or private.
    return;
  }

  if (in_ignoreinvolved(ftype)) {
    // The function or its parameters involves something that the
    // user requested we ignore.
    if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      itype._flags |= InterrogateType::F_private_destructor;
    }
    return;
  }

  if (in_ignoremember(function->get_simple_name())) {
    // The user requested us to ignore members of this name.
    if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      itype._flags |= InterrogateType::F_private_destructor;
    }
    return;
  }

  if ((function->_storage_class & CPPInstance::SC_inherited_virtual) != 0 &&
      struct_type->_derivation.size() == 1 &&
      struct_type->_derivation[0]._vis <= V_public &&
      !struct_type->_derivation[0]._is_virtual) {
    // If this function is a virtual function whose first appearance
    // is in some base class, we don't need to repeat its definition
    // here, since we're already inheriting it properly.  However, we
    // may need to make an exception in the presence of multiple
    // inheritance.
    if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      itype._flags |= InterrogateType::F_inherited_destructor;
      return;
    }

    // Let's make sure the that first appearance of the function is
    // actually declared published.
    if (is_inherited_published(function, struct_type)) {
      return;
    }
    // If it isn't, we should publish this method anyway.
  }


  FunctionIndex index = get_function(function, "", struct_type, scope, 0);
  if (index != 0) {
    if ((ftype->_flags & CPPFunctionType::F_constructor) != 0) {
      if (find(itype._constructors.begin(), itype._constructors.end(),
               index) == itype._constructors.end()) {
        itype._constructors.push_back(index);
      }

    } else if ((ftype->_flags & CPPFunctionType::F_destructor) != 0) {
      itype._flags |= InterrogateType::F_true_destructor;
      itype._destructor = index;

    } else if ((ftype->_flags & CPPFunctionType::F_operator_typecast) != 0) {
      if (find(itype._casts.begin(), itype._casts.end(),
               index) == itype._casts.end()) {
        itype._casts.push_back(index);
      }

    } else {
      if (find(itype._methods.begin(), itype._methods.end(),
               index) == itype._methods.end()) {
        itype._methods.push_back(index);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_enum_type
//       Access: Private
//  Description: Builds up a definition for the indicated enum type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_enum_type(InterrogateType &itype, CPPEnumType *cpptype) {
  itype._flags |= InterrogateType::F_enum;

  CPPScope *scope = cpptype->_parent_scope;
  if (cpptype->_ident != (CPPIdentifier *)NULL) {
    scope = cpptype->_ident->get_scope(&parser, &parser);
  }

  // Make sure the enum declaration within its parent scope isn't
  // private or protected.  If it is, we can't export any of its
  // members.
  if (TypeManager::involves_unpublished(cpptype)) {
    itype._flags &= ~InterrogateType::F_fully_defined;
    itype._flags |= InterrogateType::F_unpublished;
    return;
  }

  int next_value = 0;

  CPPEnumType::Elements::const_iterator ei;
  for (ei = cpptype->_elements.begin();
       ei != cpptype->_elements.end();
       ++ei) {
    CPPInstance *element = (*ei);

    // Tell the enum element where its native scope is, so we can get
    // a properly scoped name.

    if (element->_ident->_native_scope != scope) {
      element = new CPPInstance(*element);
      element->_ident = new CPPIdentifier(*element->_ident);
      element->_ident->_native_scope = scope;
    }

    InterrogateType::EnumValue evalue;
    evalue._name = element->get_simple_name();
    evalue._scoped_name = descope(element->get_local_name(&parser));

    if (element->_leading_comment != (CPPCommentBlock *)NULL) {
      evalue._comment = trim_blanks(element->_leading_comment->_comment);
    }

    if (element->_initializer != (CPPExpression *)NULL) {
      CPPExpression::Result result = element->_initializer->evaluate();

      if (result._type == CPPExpression::RT_error) {
        nout << "enum value ";
        element->output(nout, 0, &parser, true);
        nout << " has invalid definition!\n";
        return;
      } else {
        next_value = result.as_integer();
      }
    }
    evalue._value = next_value;
    itype._enum_values.push_back(evalue);

    next_value++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_typedef_type
//       Access: Private
//  Description: Builds up a definition for the indicated typedef.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_typedef_type(InterrogateType &itype, CPPTypedefType *cpptype) {
  itype._flags |= InterrogateType::F_typedef;
  itype._wrapped_type = get_type(cpptype->_type, false);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_array_type
//       Access: Private
//  Description: Builds up a definition for the indicated wrapped type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_array_type(InterrogateType &itype, CPPArrayType *cpptype) {
  itype._flags |= InterrogateType::F_array;
  itype._wrapped_type = get_type(cpptype->_element_type, false);

  if (cpptype->_bounds == NULL) {
    // This indicates an unsized array.
    itype._array_size = -1;
  } else {
    itype._array_size = cpptype->_bounds->evaluate().as_integer();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::define_extension_type
//       Access: Private
//  Description: Builds up a definition for the indicated extension type.
////////////////////////////////////////////////////////////////////
void InterrogateBuilder::
define_extension_type(InterrogateType &itype, CPPExtensionType *cpptype) {
  // An "extension type" as returned by CPPParser is really a forward
  // reference to an undefined struct or class type.
  itype._flags &= ~InterrogateType::F_fully_defined;

  // But we can at least indicate which of the various extension types
  // it is.
  switch (cpptype->_type) {
  case CPPExtensionType::T_enum:
    itype._flags |= InterrogateType::F_enum;
    break;

  case CPPExtensionType::T_class:
    itype._flags |= InterrogateType::F_class;
    break;

  case CPPExtensionType::T_struct:
    itype._flags |= InterrogateType::F_struct;
    break;

  case CPPExtensionType::T_union:
    itype._flags |= InterrogateType::F_union;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateBuilder::trim_blanks
//       Access: Private, Static
//  Description:
////////////////////////////////////////////////////////////////////
string InterrogateBuilder::
trim_blanks(const string &str) {
  size_t start = 0;
  while (start < str.length() && isspace(str[start])) {
    start++;
  }

  size_t end = str.length();
  while (end > start && isspace(str[end - 1])) {
    end--;
  }

  return str.substr(start, end - start);
}
