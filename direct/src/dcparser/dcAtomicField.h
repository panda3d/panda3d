// Filename: dcAtomicField.h
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

#ifndef DCATOMICFIELD_H
#define DCATOMICFIELD_H

#include "dcbase.h"
#include "dcField.h"
#include "dcSubatomicType.h"
#include "dcParameter.h"

// Must use math.h instead of cmath.h so this can compile outside of
// Panda.
#include <math.h>

////////////////////////////////////////////////////////////////////
//       Class : DCAtomicField
// Description : A single atomic field of a Distributed Class, as read
//               from a .dc file.  This defines an interface to the
//               Distributed Class, and is always implemented as a
//               remote procedure method.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCAtomicField : public DCField {
public:
  DCAtomicField(const string &name);
  virtual ~DCAtomicField();

PUBLISHED:
  virtual DCAtomicField *as_atomic_field();

  int get_num_elements() const;
  DCParameter *get_element(int n) const;

  // These five methods are deprecated and will be removed soon.
  string get_element_default(int n) const;
  bool has_element_default(int n) const;
  string get_element_name(int n) const;
  DCSubatomicType get_element_type(int n) const;
  int get_element_divisor(int n) const;
  
public:
  void add_element(DCParameter *element);

  virtual void output(ostream &out, bool brief) const;
  virtual void write(ostream &out, bool brief, int indent_level) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

  virtual DCPackerInterface *get_nested_field(int n) const;

private:
  void output_element(ostream &out, bool brief, DCParameter *element) const;

  typedef pvector<DCParameter *> Elements;
  Elements _elements;
};

#endif
