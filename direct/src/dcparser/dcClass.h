// Filename: dcClass.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcField.h"

#include <vector>
#include <map>

////////////////////////////////////////////////////////////////////
// 	 Class : DCClass
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

  void write(ostream &out, int indent_level = 0) const;
  bool add_field(DCField *field);

public:
  // These members define the primary interface to the distributed
  // class as read from the file.
  int _number;
  string _name;

  typedef vector<DCClass *> Parents;
  Parents _parents;

  typedef vector<DCField *> Fields;
  Fields _fields;

public:
  // These members are built up during parsing for the convenience of
  // the parser.
  typedef map<string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;
};

#endif
