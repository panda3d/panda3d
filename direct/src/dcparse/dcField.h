// Filename: dcField.h
// Created by:  drose (11Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCFIELD_H
#define DCFIELD_H

#include "dcbase.h"

class DCAtomicField;
class DCMolecularField;

////////////////////////////////////////////////////////////////////
// 	 Class : DCField
// Description : A single field of a Distributed Class, either atomic
//               or molecular.
////////////////////////////////////////////////////////////////////
class DCField {
PUBLISHED:
  int get_number() const;
  const string &get_name() const;

  virtual DCAtomicField *as_atomic_field();
  virtual DCMolecularField *as_molecular_field();

public:
  virtual ~DCField();
  virtual void write(ostream &out, int indent_level = 0) const=0;

public:
  int _number;
  string _name;
};

#endif
