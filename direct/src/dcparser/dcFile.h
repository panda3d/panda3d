// Filename: dcFile.h
// Created by:  drose (05Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DCFILE_H
#define DCFILE_H

#include "dcbase.h"
#include "dcClass.h"

class HashGenerator;

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

  int get_num_classes();
  DCClass *get_class(int n);
  DCClass *get_class_by_name(const string &name);

  int get_num_import_modules() const;
  string get_import_module(int n) const;
  int get_num_import_symbols(int n) const;
  string get_import_symbol(int n, int i) const;

  unsigned long get_hash() const;

public:
  void generate_hash(HashGenerator &hash) const;
  bool add_class(DCClass *dclass);
  void add_import_module(const string &import_module);
  void add_import_symbol(const string &import_symbol);

public:
  // This vector is the primary interface to the distributed classes
  // read from the file.
  typedef pvector<DCClass *> Classes;
  Classes _classes;

  typedef pvector<string> ImportSymbols;
  class Import {
  public:
    string _module;
    ImportSymbols _symbols;
  };

  typedef pvector<Import> Imports;
  Imports _imports;

public:
  // This map is built up during parsing for the convenience of the parser.
  typedef pmap<string, DCClass *> ClassesByName;
  ClassesByName _classes_by_name;
};

#endif


