// Filename: dcMolecularField.h
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

#ifndef DCMOLECULARFIELD_H
#define DCMOLECULARFIELD_H

#include "dcbase.h"
#include "dcField.h"

class DCAtomicField;
class DCParameter;

////////////////////////////////////////////////////////////////////
//       Class : DCMolecularField
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
  DCMolecularField(const string &name);

  void add_atomic(DCAtomicField *atomic);

  virtual void write(ostream &out, bool brief, int indent_level) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

  virtual DCPackerInterface *get_nested_field(int n) const;

private:
  // These members define the primary interface to the molecular field
  // definition as read from the file.
  typedef pvector<DCAtomicField *> Fields;
  Fields _fields;

  DCParameter *get_next_pack_element();

  typedef pvector<DCPackerInterface *> NestedFields;
  NestedFields _nested_fields;
};

#endif


