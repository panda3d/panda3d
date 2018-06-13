/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcFile.cxx
 * @author drose
 * @date 2000-10-05
 */

#include "dcFile.h"
#include "dcClass.h"
#include "dcSwitch.h"
#include "dcParserDefs.h"
#include "dcLexerDefs.h"
#include "dcTypedef.h"
#include "dcKeyword.h"
#include "hashGenerator.h"

#ifdef WITHIN_PANDA
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "executionEnvironment.h"
#include "configVariableList.h"
#endif

using std::cerr;
using std::string;


/**
 *
 */
DCFile::
DCFile() {
  _all_objects_valid = true;
  _inherited_fields_stale = false;

  setup_default_keywords();
}

/**
 *
 */
DCFile::
~DCFile() {
  clear();
}

/**
 * Removes all of the classes defined within the DCFile and prepares it for
 * reading a new file.
 */
void DCFile::
clear() {
  Declarations::iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    delete (*di);
  }
  for (di = _things_to_delete.begin(); di != _things_to_delete.end(); ++di) {
    delete (*di);
  }

  _classes.clear();
  _imports.clear();
  _things_by_name.clear();
  _typedefs.clear();
  _typedefs_by_name.clear();
  _keywords.clear_keywords();
  _declarations.clear();
  _things_to_delete.clear();
  setup_default_keywords();

  _all_objects_valid = true;
  _inherited_fields_stale = false;
}

#ifdef WITHIN_PANDA

/**
 * This special method reads all of the .dc files named by the "dc-file"
 * config.prc variable, and loads them into the DCFile namespace.
 */
bool DCFile::
read_all() {
  static ConfigVariableList dc_files
    ("dc-file", PRC_DESC("The list of dc files to load."));

  if (dc_files.size() == 0) {
    cerr << "No files specified via dc-file Config.prc variable!\n";
    return false;
  }

  int size = dc_files.size();

  // Load the DC files in opposite order, because we want to load the least-
  // important (most fundamental) files first.
  for (int i = size - 1; i >= 0; --i) {
    string dc_file = ExecutionEnvironment::expand_string(dc_files[i]);
    Filename filename = Filename::from_os_specific(dc_file);
    if (!read(filename)) {
      return false;
    }
  }

  return true;
}

#endif  // WITHIN_PANDA

/**
 * Opens and reads the indicated .dc file by name.  The distributed classes
 * defined in the file will be appended to the set of distributed classes
 * already recorded, if any.
 *
 * Returns true if the file is successfully read, false if there was an error
 * (in which case the file might have been partially read).
 */
bool DCFile::
read(Filename filename) {
#ifdef WITHIN_PANDA
  filename.set_text();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *in = vfs->open_read_file(filename, true);
  if (in == nullptr) {
    cerr << "Cannot open " << filename << " for reading.\n";
    return false;
  }
  bool okflag = read(*in, filename);

  // For some reason--compiler bug in gcc 3.2?--explicitly deleting the in
  // pointer does not call the appropriate global delete function; instead
  // apparently calling the system delete function.  So we call the delete
  // function by hand instead.
  vfs->close_read_file(in);

  return okflag;

#else  // WITHIN_PANDA

  pifstream in;
  in.open(filename.c_str());

  if (!in) {
    cerr << "Cannot open " << filename << " for reading.\n";
    return false;
  }

  return read(in, filename);

#endif  // WITHIN_PANDA
}

/**
 * Parses the already-opened input stream for distributed class descriptions.
 * The filename parameter is optional and is only used when reporting errors.
 *
 * The distributed classes defined in the file will be appended to the set of
 * distributed classes already recorded, if any.
 *
 * Returns true if the file is successfully read, false if there was an error
 * (in which case the file might have been partially read).
 */
bool DCFile::
read(std::istream &in, const string &filename) {
  cerr << "DCFile::read of " << filename << "\n";
  dc_init_parser(in, filename, *this);
  dcyyparse();
  dc_cleanup_parser();

  return (dc_error_count() == 0);
}

/**
 * Opens the indicated filename for output and writes a parseable description
 * of all the known distributed classes to the file.
 *
 * Returns true if the description is successfully written, false otherwise.
 */
bool DCFile::
write(Filename filename, bool brief) const {
  pofstream out;

#ifdef WITHIN_PANDA
  filename.set_text();
  filename.open_write(out);
#else
  out.open(filename.c_str());
#endif

  if (!out) {
    cerr << "Can't open " << filename << " for output.\n";
    return false;
  }
  return write(out, brief);
}

/**
 * Writes a parseable description of all the known distributed classes to the
 * stream.
 *
 * Returns true if the description is successfully written, false otherwise.
 */
bool DCFile::
write(std::ostream &out, bool brief) const {
  if (!_imports.empty()) {
    Imports::const_iterator ii;
    for (ii = _imports.begin(); ii != _imports.end(); ++ii) {
      const Import &import = (*ii);
      if (import._symbols.empty()) {
        out << "import " << import._module << "\n";
      } else {
        out << "from " << import._module << " import ";
        ImportSymbols::const_iterator si = import._symbols.begin();
        out << *si;
        ++si;
        while (si != import._symbols.end()) {
          out << ", " << *si;
        ++si;
        }
        out << "\n";
      }
    }
    out << "\n";
  }

  Declarations::const_iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    (*di)->write(out, brief, 0);
    out << "\n";
  }

  return !out.fail();
}

/**
 * Returns the number of classes read from the .dc file(s).
 */
int DCFile::
get_num_classes() const {
  return _classes.size();
}

/**
 * Returns the nth class read from the .dc file(s).
 */
DCClass *DCFile::
get_class(int n) const {
  nassertr(n >= 0 && n < (int)_classes.size(), nullptr);
  return _classes[n];
}

/**
 * Returns the class that has the indicated name, or NULL if there is no such
 * class.
 */
DCClass *DCFile::
get_class_by_name(const string &name) const {
  ThingsByName::const_iterator ni;
  ni = _things_by_name.find(name);
  if (ni != _things_by_name.end()) {
    return (*ni).second->as_class();
  }

  return nullptr;
}

/**
 * Returns the switch that has the indicated name, or NULL if there is no such
 * switch.
 */
DCSwitch *DCFile::
get_switch_by_name(const string &name) const {
  ThingsByName::const_iterator ni;
  ni = _things_by_name.find(name);
  if (ni != _things_by_name.end()) {
    return (*ni).second->as_switch();
  }

  return nullptr;
}

/**
 * Returns a pointer to the one DCField that has the indicated index number,
 * of all the DCFields across all classes in the file.
 *
 * This method is only valid if dc-multiple-inheritance is set true in the
 * Config.prc file.  Without this setting, different DCFields may share the
 * same index number, so this global lookup is not possible.
 */
DCField *DCFile::
get_field_by_index(int index_number) const {
  nassertr(dc_multiple_inheritance, nullptr);

  if (index_number >= 0 && index_number < (int)_fields_by_index.size()) {
    return _fields_by_index[index_number];
  }

  return nullptr;
}

/**
 * Returns the number of import lines read from the .dc file(s).
 */
int DCFile::
get_num_import_modules() const {
  return _imports.size();
}

/**
 * Returns the module named by the nth import line read from the .dc file(s).
 */
string DCFile::
get_import_module(int n) const {
  nassertr(n >= 0 && n < (int)_imports.size(), string());
  return _imports[n]._module;
}

/**
 * Returns the number of symbols explicitly imported by the nth import line.
 * If this is 0, the line is "import modulename"; if it is more than 0, the
 * line is "from modulename import symbol, symbol ... ".
 */
int DCFile::
get_num_import_symbols(int n) const {
  nassertr(n >= 0 && n < (int)_imports.size(), 0);
  return _imports[n]._symbols.size();
}

/**
 * Returns the ith symbol named by the nth import line read from the .dc
 * file(s).
 */
string DCFile::
get_import_symbol(int n, int i) const {
  nassertr(n >= 0 && n < (int)_imports.size(), string());
  nassertr(i >= 0 && i < (int)_imports[n]._symbols.size(), string());
  return _imports[n]._symbols[i];
}

/**
 * Returns the number of typedefs read from the .dc file(s).
 */
int DCFile::
get_num_typedefs() const {
  return _typedefs.size();
}

/**
 * Returns the nth typedef read from the .dc file(s).
 */
DCTypedef *DCFile::
get_typedef(int n) const {
  nassertr(n >= 0 && n < (int)_typedefs.size(), nullptr);
  return _typedefs[n];
}

/**
 * Returns the typedef that has the indicated name, or NULL if there is no
 * such typedef name.
 */
DCTypedef *DCFile::
get_typedef_by_name(const string &name) const {
  TypedefsByName::const_iterator ni;
  ni = _typedefs_by_name.find(name);
  if (ni != _typedefs_by_name.end()) {
    return (*ni).second;
  }

  return nullptr;
}

/**
 * Returns the number of keywords read from the .dc file(s).
 */
int DCFile::
get_num_keywords() const {
  return _keywords.get_num_keywords();
}

/**
 * Returns the nth keyword read from the .dc file(s).
 */
const DCKeyword *DCFile::
get_keyword(int n) const {
  return _keywords.get_keyword(n);
}

/**
 * Returns the keyword that has the indicated name, or NULL if there is no
 * such keyword name.
 */
const DCKeyword *DCFile::
get_keyword_by_name(const string &name) const {
  const DCKeyword *keyword = _keywords.get_keyword_by_name(name);
  if (keyword == nullptr) {
    keyword = _default_keywords.get_keyword_by_name(name);
    if (keyword != nullptr) {
      // One of the historical default keywords was used, but wasn't defined.
      // Define it implicitly right now.
      ((DCFile *)this)->_keywords.add_keyword(keyword);
    }
  }

  return keyword;
}

/**
 * Returns a 32-bit hash index associated with this file.  This number is
 * guaranteed to be consistent if the contents of the file have not changed,
 * and it is very likely to be different if the contents of the file do
 * change.
 */
unsigned long DCFile::
get_hash() const {
  HashGenerator hashgen;
  generate_hash(hashgen);
  return hashgen.get_hash();
}

/**
 * Accumulates the properties of this file into the hash.
 */
void DCFile::
generate_hash(HashGenerator &hashgen) const {
  if (dc_virtual_inheritance) {
    // Just to make the hash number change in this case.
    if (dc_sort_inheritance_by_file) {
      hashgen.add_int(1);
    } else {
      hashgen.add_int(2);
    }
  }

  hashgen.add_int(_classes.size());
  Classes::const_iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->generate_hash(hashgen);
  }
}

/**
 * Adds the newly-allocated distributed class definition to the file.  The
 * DCFile becomes the owner of the pointer and will delete it when it
 * destructs.  Returns true if the class is successfully added, or false if
 * there was a name conflict.
 */
bool DCFile::
add_class(DCClass *dclass) {
  if (!dclass->get_name().empty()) {
    bool inserted = _things_by_name.insert
      (ThingsByName::value_type(dclass->get_name(), dclass)).second;

    if (!inserted) {
      return false;
    }
  }

  if (!dclass->is_struct()) {
    dclass->set_number(get_num_classes());
  }
  _classes.push_back(dclass);

  if (dclass->is_bogus_class()) {
    _all_objects_valid = false;
  }

  if (!dclass->is_bogus_class()) {
    _declarations.push_back(dclass);
  } else {
    _things_to_delete.push_back(dclass);
  }

  return true;
}

/**
 * Adds the newly-allocated switch definition to the file.  The DCFile becomes
 * the owner of the pointer and will delete it when it destructs.  Returns
 * true if the switch is successfully added, or false if there was a name
 * conflict.
 */
bool DCFile::
add_switch(DCSwitch *dswitch) {
  if (!dswitch->get_name().empty()) {
    bool inserted = _things_by_name.insert
      (ThingsByName::value_type(dswitch->get_name(), dswitch)).second;

    if (!inserted) {
      return false;
    }
  }

  _declarations.push_back(dswitch);

  return true;
}

/**
 * Adds a new name to the list of names of Python modules that are to be
 * imported by the client or AI to define the code that is associated with the
 * class interfaces named within the .dc file.
 */
void DCFile::
add_import_module(const string &import_module) {
  Import import;
  import._module = import_module;
  _imports.push_back(import);
}

/**
 * Adds a new name to the list of symbols that are to be explicitly imported
 * from the most-recently added module, e.g.  "from module_name import
 * symbol".  If the list of symbols is empty, the syntax is taken to be
 * "import module_name".
 */
void DCFile::
add_import_symbol(const string &import_symbol) {
  nassertv(!_imports.empty());
  _imports.back()._symbols.push_back(import_symbol);
}

/**
 * Adds the newly-allocated distributed typedef definition to the file.  The
 * DCFile becomes the owner of the pointer and will delete it when it
 * destructs.  Returns true if the typedef is successfully added, or false if
 * there was a name conflict.
 */
bool DCFile::
add_typedef(DCTypedef *dtypedef) {
  bool inserted = _typedefs_by_name.insert
    (TypedefsByName::value_type(dtypedef->get_name(), dtypedef)).second;

  if (!inserted) {
    return false;
  }

  dtypedef->set_number(get_num_typedefs());
  _typedefs.push_back(dtypedef);

  if (dtypedef->is_bogus_typedef()) {
    _all_objects_valid = false;
  }

  if (!dtypedef->is_bogus_typedef() && !dtypedef->is_implicit_typedef()) {
    _declarations.push_back(dtypedef);
  } else {
    _things_to_delete.push_back(dtypedef);
  }

  return true;
}

/**
 * Adds the indicated keyword string to the list of keywords known to the
 * DCFile.  These keywords may then be added to DCFields.  It is not an error
 * to add a particular keyword more than once.
 */
bool DCFile::
add_keyword(const string &name) {
  DCKeyword *keyword = new DCKeyword(name);
  bool added = _keywords.add_keyword(keyword);

  if (added) {
    _declarations.push_back(keyword);
  } else {
    delete keyword;
  }

  return added;
}

/**
 * Adds the indicated declaration to the list of declarations that are not
 * reported with the file, but will be deleted when the DCFile object
 * destructs.  That is, transfers ownership of the indicated pointer to the
 * DCFile.
 */
void DCFile::
add_thing_to_delete(DCDeclaration *decl) {
  _things_to_delete.push_back(decl);
}

/**
 * Sets the next sequential available index number on the indicated field.
 * This is only meant to be called
 */
void DCFile::
set_new_index_number(DCField *field) {
  field->set_number((int)_fields_by_index.size());
  _fields_by_index.push_back(field);
}

/**
 * Adds an entry for each of the default keywords that are defined for every
 * DCFile for legacy reasons.
 */
void DCFile::
setup_default_keywords() {
  struct KeywordDef {
    const char *name;
    int flag;
  };
  static KeywordDef default_keywords[] = {
    { "required", 0x0001 },
    { "broadcast", 0x0002 },
    { "ownrecv", 0x0004 },
    { "ram", 0x0008 },
    { "db", 0x0010 },
    { "clsend", 0x0020 },
    { "clrecv", 0x0040 },
    { "ownsend", 0x0080 },
    { "airecv", 0x0100 },
    { nullptr, 0 }
  };

  _default_keywords.clear_keywords();
  for (int i = 0; default_keywords[i].name != nullptr; ++i) {
    DCKeyword *keyword =
      new DCKeyword(default_keywords[i].name,
                    default_keywords[i].flag);

    _default_keywords.add_keyword(keyword);
    _things_to_delete.push_back(keyword);
  }
}

/**
 * Reconstructs the inherited fields table of all classes.
 */
void DCFile::
rebuild_inherited_fields() {
  _inherited_fields_stale = false;

  Classes::iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->clear_inherited_fields();
  }
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->rebuild_inherited_fields();
  }
}
