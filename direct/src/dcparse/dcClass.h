// Filename: dcClass.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcAtomicField.h"
#include "dcMolecularField.h"

#include <vector>
#include <map>

////////////////////////////////////////////////////////////////////
// 	 Class : DCClass
// Description : Defines a particular DistributedClass as read from an
//               input .dc file.
////////////////////////////////////////////////////////////////////
class DCClass {
PUBLISHED:
  int get_number() const;
  string get_name() const;

  bool has_parent() const;
  DCClass *get_parent() const;

  int get_num_atomics();
  DCAtomicField *get_atomic(int n);
  DCAtomicField *get_atomic_by_name(const string &name);

  int get_num_inherited_atomics();
  DCAtomicField *get_inherited_atomic(int n);

  int get_num_moleculars();
  DCMolecularField *get_molecular(int n);
  DCMolecularField *get_molecular_by_name(const string &name);

  int get_num_inherited_moleculars();
  DCMolecularField *get_inherited_molecular(int n);

public:
  DCClass();
  ~DCClass();

  void write(ostream &out, int indent_level = 0) const;
  bool add_field(DCAtomicField *field);
  bool add_field(DCMolecularField *field);

public:
  // These members define the primary interface to the distributed
  // class as read from the file.
  int _number;
  string _name;

  typedef vector<DCClass *> Parents;
  Parents _parents;

  typedef vector<DCAtomicField *> AtomicFields;
  AtomicFields _atomic_fields;

  typedef vector<DCMolecularField *> MolecularFields;
  MolecularFields _molecular_fields;

public:
  // These members are built up during parsing for the convenience of
  // the parser.
  typedef map<string, DCAtomicField *> AtomicsByName;
  AtomicsByName _atomics_by_name;

  typedef map<string, DCMolecularField *> MolecularsByName;
  MolecularsByName _moleculars_by_name;
};

#endif
