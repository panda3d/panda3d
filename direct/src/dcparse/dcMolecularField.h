// Filename: dcMolecularField.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCMOLECULARFIELD_H
#define DCMOLECULARFIELD_H

#include "dcbase.h"

#include <vector>

class DCAtomicField;

////////////////////////////////////////////////////////////////////
// 	 Class : DCMolecularField
// Description : A single molecular field of a Distributed Class, as
//               read from a .dc file.  This represents a combination
//               of two or more related atomic fields, that will often
//               be treated as a unit.
////////////////////////////////////////////////////////////////////
class DCMolecularField {
PUBLISHED:
  int get_number() const;
  string get_name() const;

  int get_num_atomics() const;
  DCAtomicField *get_atomic(int n) const;

public:
  DCMolecularField();
  void write(ostream &out, int indent_level = 0) const;

public:
  // These members define the primary interface to the molecular field
  // definition as read from the file.
  int _number;
  string _name;

  typedef vector<DCAtomicField *> Fields;
  Fields _fields;
};

#endif

  
