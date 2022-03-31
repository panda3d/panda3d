/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcFile.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCFILE_H
#define DCFILE_H

#include "dcbase.h"
#include "dcKeywordList.h"

class DCClass;
class DCSwitch;
class DCField;
class HashGenerator;
class DCTypedef;
class DCKeyword;
class DCDeclaration;

/**
 * Represents the complete list of Distributed Class descriptions as read from
 * a .dc file.
 */
class EXPCL_DIRECT_DCPARSER DCFile {
PUBLISHED:
  DCFile();
  ~DCFile();

  void clear();

#ifdef WITHIN_PANDA
  bool read_all();
#endif

  bool read(Filename filename);
  bool read(std::istream &in, const std::string &filename = std::string());

  bool write(Filename filename, bool brief) const;
  bool write(std::ostream &out, bool brief) const;

  int get_num_classes() const;
  DCClass *get_class(int n) const;
  DCClass *get_class_by_name(const std::string &name) const;
  DCSwitch *get_switch_by_name(const std::string &name) const;

  DCField *get_field_by_index(int index_number) const;

  INLINE bool all_objects_valid() const;

  int get_num_import_modules() const;
  std::string get_import_module(int n) const;
  int get_num_import_symbols(int n) const;
  std::string get_import_symbol(int n, int i) const;

  int get_num_typedefs() const;
  DCTypedef *get_typedef(int n) const;
  DCTypedef *get_typedef_by_name(const std::string &name) const;

  int get_num_keywords() const;
  const DCKeyword *get_keyword(int n) const;
  const DCKeyword *get_keyword_by_name(const std::string &name) const;

  unsigned long get_hash() const;

public:
  void generate_hash(HashGenerator &hashgen) const;
  bool add_class(DCClass *dclass);
  bool add_switch(DCSwitch *dswitch);
  void add_import_module(const std::string &import_module);
  void add_import_symbol(const std::string &import_symbol);
  bool add_typedef(DCTypedef *dtypedef);
  bool add_keyword(const std::string &name);
  void add_thing_to_delete(DCDeclaration *decl);

  void set_new_index_number(DCField *field);
  INLINE void check_inherited_fields();
  INLINE void mark_inherited_fields_stale();

private:
  void setup_default_keywords();
  void rebuild_inherited_fields();

  typedef pvector<DCClass *> Classes;
  Classes _classes;

  typedef pmap<std::string, DCDeclaration *> ThingsByName;
  ThingsByName _things_by_name;

  typedef pvector<std::string> ImportSymbols;
  class Import {
  public:
    std::string _module;
    ImportSymbols _symbols;
  };

  typedef pvector<Import> Imports;
  Imports _imports;

  typedef pvector<DCTypedef *> Typedefs;
  Typedefs _typedefs;

  typedef pmap<std::string, DCTypedef *> TypedefsByName;
  TypedefsByName _typedefs_by_name;

  DCKeywordList _keywords;
  DCKeywordList _default_keywords;

  typedef pvector<DCDeclaration *> Declarations;
  Declarations _declarations;
  Declarations _things_to_delete;

  typedef pvector<DCField *> FieldsByIndex;
  FieldsByIndex _fields_by_index;

  bool _all_objects_valid;
  bool _inherited_fields_stale;
};

#include "dcFile.I"

#endif
