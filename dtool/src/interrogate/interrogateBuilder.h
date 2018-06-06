/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateBuilder.h
 * @author drose
 * @date 2000-08-01
 */

#ifndef INTERROGATEBUILDER_H
#define INTERROGATEBUILDER_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"

#include <map>
#include <set>
#include <vector>

class CPPFunctionGroup;
class CPPInstance;
class CPPType;
class CPPSimpleType;
class CPPPointerType;
class CPPConstType;
class CPPExtensionType;
class CPPStructType;
class CPPEnumType;
class CPPTypedefType;
class CPPArrayType;
class CPPFunctionType;
class CPPScope;
class CPPIdentifier;
class CPPNameComponent;
class CPPManifest;
class CPPMakeProperty;
class CPPMakeSeq;
class InterrogateType;
class InterrogateFunction;
class FunctionRemap;
class InterfaceMaker;

/**
 * This class builds up the InterrogateDatabase based on the data indicated by
 * CPPParser after reading the source code.
 */
class InterrogateBuilder {
public:
  void add_source_file(const std::string &filename);
  void read_command_file(std::istream &in);
  void do_command(const std::string &command, const std::string &params);
  void build();
  void write_code(std::ostream &out_code, std::ostream *out_include, InterrogateModuleDef *def);
  InterrogateModuleDef *make_module_def(int file_identifier);

  static std::string clean_identifier(const std::string &name);
  static std::string descope(const std::string &name);
  FunctionIndex get_destructor_for(CPPType *type);

  std::string get_preferred_name(CPPType *type);
  static std::string hash_string(const std::string &name, int shift_offset);
  TypeIndex get_type(CPPType *type, bool global);

public:
  typedef std::set<std::string> Commands;
  typedef std::map<std::string, std::string> CommandParams;
  void insert_param_list(InterrogateBuilder::Commands &commands,
                         const std::string &params);

  bool in_forcetype(const std::string &name) const;
  std::string in_renametype(const std::string &name) const;
  bool in_ignoretype(const std::string &name) const;
  std::string in_defconstruct(const std::string &name) const;
  bool in_ignoreinvolved(const std::string &name) const;
  bool in_ignoreinvolved(CPPType *type) const;
  bool in_ignorefile(const std::string &name) const;
  bool in_ignoremember(const std::string &name) const;
  bool in_noinclude(const std::string &name) const;
  bool should_include(const std::string &filename) const;

  bool is_inherited_published(CPPInstance *function, CPPStructType *struct_type);

  void remap_indices(std::vector<FunctionRemap *> &remaps);
  void scan_function(CPPFunctionGroup *fgroup);
  void scan_function(CPPInstance *function);
  void scan_struct_type(CPPStructType *type);
  void scan_enum_type(CPPEnumType *type);
  void scan_typedef_type(CPPTypedefType *type);
  void scan_manifest(CPPManifest *manifest);
  ElementIndex scan_element(CPPInstance *element, CPPStructType *struct_type,
                            CPPScope *scope);

  FunctionIndex get_getter(CPPType *expr_type, std::string expression,
                           CPPStructType *struct_type, CPPScope *scope,
                           CPPInstance *element);
  FunctionIndex get_setter(CPPType *expr_type, std::string expression,
                           CPPStructType *struct_type, CPPScope *scope,
                           CPPInstance *element);
  FunctionIndex get_cast_function(CPPType *to_type, CPPType *from_type,
                                  const std::string &prefix);
  FunctionIndex
  get_function(CPPInstance *function, std::string description,
               CPPStructType *struct_type, CPPScope *scope,
               int flags, const std::string &expression = std::string());

  ElementIndex
  get_make_property(CPPMakeProperty *make_property, CPPStructType *struct_type, CPPScope *scope);

  MakeSeqIndex
  get_make_seq(CPPMakeSeq *make_seq, CPPStructType *struct_type);

  TypeIndex get_atomic_string_type();

  void define_atomic_type(InterrogateType &itype, CPPSimpleType *cpptype);
  void define_wrapped_type(InterrogateType &itype, CPPPointerType *cpptype);
  void define_wrapped_type(InterrogateType &itype, CPPConstType *cpptype);
  void define_struct_type(InterrogateType &itype, CPPStructType *cpptype,
                          TypeIndex type_index, bool forced);
  void update_function_comment(CPPInstance *function, CPPScope *scope);
  void define_method(CPPFunctionGroup *fgroup, InterrogateType &itype,
                     CPPStructType *struct_type, CPPScope *scope);
  void define_method(CPPInstance *function, InterrogateType &itype,
                     CPPStructType *struct_type, CPPScope *scope);
  void define_enum_type(InterrogateType &itype, CPPEnumType *cpptype);
  void define_typedef_type(InterrogateType &itype, CPPTypedefType *cpptype);
  void define_array_type(InterrogateType &itype, CPPArrayType *cpptype);
  void define_extension_type(InterrogateType &itype,
                             CPPExtensionType *cpptype);

  static std::string trim_blanks(const std::string &str);

  typedef std::map<std::string, TypeIndex> TypesByName;
  typedef std::map<std::string, FunctionIndex> FunctionsByName;
  typedef std::map<std::string, MakeSeqIndex> MakeSeqsByName;
  typedef std::map<std::string, ElementIndex> PropertiesByName;

  TypesByName _types_by_name;
  FunctionsByName _functions_by_name;
  MakeSeqsByName _make_seqs_by_name;
  PropertiesByName _properties_by_name;

  typedef std::map<std::string, char> IncludeFiles;
  IncludeFiles _include_files;

  Commands _forcetype;
  CommandParams _renametype;
  Commands _ignoretype;
  CommandParams _defconstruct;
  Commands _ignoreinvolved;
  Commands _ignorefile;
  Commands _ignoremember;
  Commands _noinclude;

  std::string _library_hash_name;

  friend class FunctionRemap;
};

extern InterrogateBuilder builder;

#endif
