/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcAtomicField.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCATOMICFIELD_H
#define DCATOMICFIELD_H

#include "dcbase.h"
#include "dcField.h"
#include "dcSubatomicType.h"
#include "dcParameter.h"

// Must use math.h instead of cmath.h so this can compile outside of Panda.
#include <math.h>

/**
 * A single atomic field of a Distributed Class, as read from a .dc file.
 * This defines an interface to the Distributed Class, and is always
 * implemented as a remote procedure method.
 */
class EXPCL_DIRECT_DCPARSER DCAtomicField : public DCField {
public:
  DCAtomicField(const std::string &name, DCClass *dclass, bool bogus_field);
  virtual ~DCAtomicField();

PUBLISHED:
  virtual DCAtomicField *as_atomic_field();
  virtual const DCAtomicField *as_atomic_field() const;

  int get_num_elements() const;
  DCParameter *get_element(int n) const;

  // These five methods are deprecated and will be removed soon.
  vector_uchar get_element_default(int n) const;
  bool has_element_default(int n) const;
  std::string get_element_name(int n) const;
  DCSubatomicType get_element_type(int n) const;
  int get_element_divisor(int n) const;

public:
  void add_element(DCParameter *element);

  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

  virtual DCPackerInterface *get_nested_field(int n) const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const;
  virtual bool do_check_match_atomic_field(const DCAtomicField *other) const;

private:
  void output_element(std::ostream &out, bool brief, DCParameter *element) const;

  typedef pvector<DCParameter *> Elements;
  Elements _elements;
};

#include "dcAtomicField.I"

#endif
