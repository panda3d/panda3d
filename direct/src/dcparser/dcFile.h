// Filename: dcFile.h
// Created by:  drose (05Oct00)
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

////////////////////////////////////////////////////////////////////
//       Class : DCFile
// Description : Represents the complete list of Distributed Class
//               descriptions as read from a .dc file.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCFile {
PUBLISHED:
  DCFile();
  ~DCFile();

  void clear();

#ifdef WITHIN_PANDA
  bool read_all();
#endif

  bool read(Filename filename);
  bool read(istream &in, const string &filename = string());

  bool write(Filename filename, bool brief) const;
  bool write(ostream &out, bool brief) const;

  int get_num_classes() const;
  DCClass *get_class(int n) const;
  DCClass *get_class_by_name(const string &name) const;
  DCSwitch *get_switch_by_name(const string &name) const;

  DCField *get_field_by_index(int index_number) const;

  INLINE bool all_objects_valid() const;

  int get_num_import_modules() const;
  string get_import_module(int n) const;
  int get_num_import_symbols(int n) const;
  string get_import_symbol(int n, int i) const;

  int get_num_typedefs() const;
  DCTypedef *get_typedef(int n) const;
  DCTypedef *get_typedef_by_name(const string &name) const;

  int get_num_keywords() const;
  const DCKeyword *get_keyword(int n) const;
  const DCKeyword *get_keyword_by_name(const string &name) const;

  unsigned long get_hash() const;

public:
  void generate_hash(HashGenerator &hashgen) const;
  bool add_class(DCClass *dclass);
  bool add_switch(DCSwitch *dswitch);
  void add_import_module(const string &import_module);
  void add_import_symbol(const string &import_symbol);
  bool add_typedef(DCTypedef *dtypedef);
  bool add_keyword(const string &name);
  void add_thing_to_delete(DCDeclaration *decl);

  void set_new_index_number(DCField *field);
  INLINE void check_inherited_fields();
  INLINE void mark_inherited_fields_stale();

private:
  void setup_default_keywords();
  void rebuild_inherited_fields();

  typedef pvector<DCClass *> Classes;
  Classes _classes;

  typedef pmap<string, DCDeclaration *> ThingsByName;
  ThingsByName _things_by_name;

  typedef pvector<string> ImportSymbols;
  class Import {
  public:
    string _module;
    ImportSymbols _symbols;
  };

  typedef pvector<Import> Imports;
  Imports _imports;

  typedef pvector<DCTypedef *> Typedefs;
  Typedefs _typedefs;

  typedef pmap<string, DCTypedef *> TypedefsByName;
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


