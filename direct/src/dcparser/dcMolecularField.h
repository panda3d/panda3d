// Filename: dcMolecularField.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCMOLECULARFIELD_H
#define DCMOLECULARFIELD_H

#include "dcbase.h"
#include "dcField.h"

#include <vector>

class DCAtomicField;

////////////////////////////////////////////////////////////////////
// 	 Class : DCMolecularField
// Description : A single molecular field of a Distributed Class, as
//               read from a .dc file.  This represents a combination
//               of two or more related atomic fields, that will often
//               be treated as a unit.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCMolecularField : public DCField {
PUBLISHED:
  virtual DCMolecularField *as_molecular_field();

  int get_num_atomics() const;
  DCAtomicField *get_atomic(int n) const;

public:
  DCMolecularField();
  virtual void write(ostream &out, int indent_level = 0) const;
  virtual void generate_hash(HashGenerator &hash) const;

public:
  // These members define the primary interface to the molecular field
  // definition as read from the file.
  typedef vector<DCAtomicField *> Fields;
  Fields _fields;
};

#endif

  
