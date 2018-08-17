/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcMolecularField.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCMOLECULARFIELD_H
#define DCMOLECULARFIELD_H

#include "dcbase.h"
#include "dcField.h"

class DCAtomicField;
class DCParameter;

/**
 * A single molecular field of a Distributed Class, as read from a .dc file.
 * This represents a combination of two or more related atomic fields, that
 * will often be treated as a unit.
 */
class EXPCL_DIRECT_DCPARSER DCMolecularField : public DCField {
public:
  DCMolecularField(const std::string &name, DCClass *dclass);

PUBLISHED:
  virtual DCMolecularField *as_molecular_field();
  virtual const DCMolecularField *as_molecular_field() const;

  int get_num_atomics() const;
  DCAtomicField *get_atomic(int n) const;

public:
  void add_atomic(DCAtomicField *atomic);

  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

  virtual DCPackerInterface *get_nested_field(int n) const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const;
  virtual bool do_check_match_molecular_field(const DCMolecularField *other) const;

private:
  // These members define the primary interface to the molecular field
  // definition as read from the file.
  typedef pvector<DCAtomicField *> Fields;
  Fields _fields;
  bool _got_keywords;

  DCParameter *get_next_pack_element();

  typedef pvector<DCPackerInterface *> NestedFields;
  NestedFields _nested_fields;
};

#endif
