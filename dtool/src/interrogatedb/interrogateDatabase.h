/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateDatabase.h
 * @author drose
 * @date 2000-08-01
 */

#ifndef INTERROGATEDATABASE_H
#define INTERROGATEDATABASE_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogateType.h"
#include "interrogateFunction.h"
#include "interrogateFunctionWrapper.h"
#include "interrogateManifest.h"
#include "interrogateElement.h"
#include "interrogateMakeSeq.h"
#include "interrogate_request.h"

#include <map>

class IndexRemapper;

/**
 * This stores all of the interrogate data and handles reading the data from a
 * disk file when necessary.
 */
class EXPCL_INTERROGATEDB InterrogateDatabase {
private:
  InterrogateDatabase();

public:
  static InterrogateDatabase *get_ptr();
  void request_module(InterrogateModuleDef *def);

public:
  // Functions to read the database.
  bool get_error_flag();

  int get_num_global_types();
  TypeIndex get_global_type(int n);
  int get_num_all_types();
  TypeIndex get_all_type(int n);
  int get_num_global_functions();
  FunctionIndex get_global_function(int n);
  int get_num_all_functions();
  FunctionIndex get_all_function(int n);
  int get_num_global_manifests();
  ManifestIndex get_global_manifest(int n);
  int get_num_global_elements();
  ElementIndex get_global_element(int n);

  const InterrogateType &get_type(TypeIndex type);
  const InterrogateFunction &get_function(FunctionIndex function);
  const InterrogateFunctionWrapper &get_wrapper(FunctionWrapperIndex wrapper);
  const InterrogateManifest &get_manifest(ManifestIndex manifest);
  const InterrogateElement &get_element(ElementIndex element);
  const InterrogateMakeSeq &get_make_seq(MakeSeqIndex element);

  INLINE TypeIndex lookup_type_by_name(const std::string &name);
  INLINE TypeIndex lookup_type_by_scoped_name(const std::string &name);
  INLINE TypeIndex lookup_type_by_true_name(const std::string &name);
  INLINE ManifestIndex lookup_manifest_by_name(const std::string &name);
  INLINE ElementIndex lookup_element_by_name(const std::string &name);
  INLINE ElementIndex lookup_element_by_scoped_name(const std::string &name);

  void remove_type(TypeIndex type);

  void *get_fptr(FunctionWrapperIndex wrapper);

  FunctionWrapperIndex get_wrapper_by_unique_name(const std::string &unique_name);

  static int get_file_major_version();
  static int get_file_minor_version();
  static int get_current_major_version();
  static int get_current_minor_version();

public:
  // Functions to build the database.
  void set_error_flag(bool error_flag);

  int get_next_index();
  void add_type(TypeIndex index, const InterrogateType &type);
  void add_function(FunctionIndex index, InterrogateFunction *function);
  void add_wrapper(FunctionWrapperIndex index,
                   const InterrogateFunctionWrapper &wrapper);
  void add_manifest(ManifestIndex index, const InterrogateManifest &manifest);
  void add_element(ElementIndex index, const InterrogateElement &element);
  void add_make_seq(MakeSeqIndex index, const InterrogateMakeSeq &make_seq);

  InterrogateType &update_type(TypeIndex type);
  InterrogateFunction &update_function(FunctionIndex function);
  InterrogateFunctionWrapper &update_wrapper(FunctionWrapperIndex wrapper);
  InterrogateManifest &update_manifest(ManifestIndex manifest);
  InterrogateElement &update_element(ElementIndex element);
  InterrogateMakeSeq &update_make_seq(MakeSeqIndex make_seq);

  int remap_indices(int first_index);
  int remap_indices(int first_index, IndexRemapper &remap);

  void write(std::ostream &out, InterrogateModuleDef *def) const;
  bool read(std::istream &in, InterrogateModuleDef *def);

private:
  INLINE void check_latest();
  void load_latest();

  bool read_new(std::istream &in, InterrogateModuleDef *def);
  void merge_from(const InterrogateDatabase &other);

  bool find_module(FunctionWrapperIndex wrapper,
                   InterrogateModuleDef *&def, int &module_index);
  int binary_search_module(int begin, int end, FunctionIndex function);
  int binary_search_wrapper_hash(InterrogateUniqueNameDef *begin,
                                 InterrogateUniqueNameDef *end,
                                 const std::string &wrapper_hash_name);

  // This data is loaded from the various database files.
  typedef std::map<TypeIndex, InterrogateType> TypeMap;
  TypeMap _type_map;
  typedef std::map<FunctionIndex, InterrogateFunction *> FunctionMap;
  FunctionMap _function_map;
  typedef std::map<FunctionWrapperIndex, InterrogateFunctionWrapper> FunctionWrapperMap;
  FunctionWrapperMap _wrapper_map;

  typedef std::map<ManifestIndex, InterrogateManifest> ManifestMap;
  ManifestMap _manifest_map;
  typedef std::map<ElementIndex, InterrogateElement> ElementMap;
  ElementMap _element_map;

  typedef std::map<MakeSeqIndex, InterrogateMakeSeq> MakeSeqMap;
  MakeSeqMap _make_seq_map;

  typedef std::vector<TypeIndex> GlobalTypes;
  GlobalTypes _global_types;
  GlobalTypes _all_types;
  typedef std::vector<FunctionIndex> GlobalFunctions;
  GlobalFunctions _global_functions;
  GlobalFunctions _all_functions;
  typedef std::vector<ManifestIndex> GlobalManifests;
  GlobalManifests _global_manifests;
  typedef std::vector<ElementIndex> GlobalElements;
  GlobalElements _global_elements;

  // This data is compiled in directly to the shared libraries that we link
  // with.
  typedef std::vector<InterrogateModuleDef *> Modules;
  Modules _modules;
  typedef std::map<std::string, InterrogateModuleDef *> ModulesByHash;
  ModulesByHash _modules_by_hash;

  // This records the set of database files that are still to be loaded.
  typedef std::vector<InterrogateModuleDef *> Requests;
  Requests _requests;

  bool _error_flag;
  int _next_index;

  enum LookupType {
    LT_type_name           = 0x001,
    LT_type_scoped_name    = 0x002,
    LT_type_true_name      = 0x004,
    LT_manifest_name       = 0x008,
    LT_element_name        = 0x010,
    LT_element_scoped_name = 0x020,
  };

  int _lookups_fresh;
  typedef std::map<std::string, int> Lookup;
  Lookup _types_by_name;
  Lookup _types_by_scoped_name;
  Lookup _types_by_true_name;
  Lookup _manifests_by_name;
  Lookup _elements_by_name;
  Lookup _elements_by_scoped_name;

  void freshen_types_by_name();
  void freshen_types_by_scoped_name();
  void freshen_types_by_true_name();
  void freshen_manifests_by_name();
  void freshen_elements_by_name();
  void freshen_elements_by_scoped_name();

  int lookup(const std::string &name,
             Lookup &lookup, LookupType type,
             void (InterrogateDatabase::*freshen)());

  static InterrogateDatabase *_global_ptr;
  static int _file_major_version;
  static int _file_minor_version;
  static int _current_major_version;
  static int _current_minor_version;
};

#include "interrogateDatabase.I"

#endif
