// Filename: dcClass.h
// Created by:  drose (05Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcField.h"

class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCClass
// Description : Defines a particular DistributedClass as read from an
//               input .dc file.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCClass {
PUBLISHED:
  int get_number() const;
  string get_name() const;

  bool has_parent() const;
  DCClass *get_parent() const;

  int get_num_fields();
  DCField *get_field(int n);
  DCField *get_field_by_name(const string &name);

  int get_num_inherited_fields();
  DCField *get_inherited_field(int n);

public:
  DCClass();
  ~DCClass();

  void write(ostream &out, bool brief, int indent_level) const;
  void generate_hash(HashGenerator &hash) const;

  bool add_field(DCField *field);

public:
  // These members define the primary interface to the distributed
  // class as read from the file.
  int _number;
  string _name;

  typedef pvector<DCClass *> Parents;
  Parents _parents;

  typedef pvector<DCField *> Fields;
  Fields _fields;

public:
  // These members are built up during parsing for the convenience of
  // the parser.
  typedef pmap<string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;
};

#endif
