// Filename: dcAtomicField.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCATOMICFIELD_H
#define DCATOMICFIELD_H

#include "dcbase.h"
#include "dcField.h"
#include "dcSubatomicType.h"

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : DCAtomicField
// Description : A single atomic field of a Distributed Class, as read
//               from a .dc file.  This defines an interface to the
//               Distributed Class, and is always implemented as a
//               remote procedure method.
////////////////////////////////////////////////////////////////////
class DCAtomicField : public DCField {
PUBLISHED:
  virtual DCAtomicField *as_atomic_field();

  int get_num_elements() const;
  DCSubatomicType get_element_type(int n) const;
  int get_element_divisor(int n) const;

  bool is_required() const;
  bool is_broadcast() const;
  bool is_p2p() const;
  bool is_ram() const;
  bool is_db() const;
  bool is_clsend() const;
  bool is_clrecv() const;
  bool is_ownsend() const;

public:
  DCAtomicField();
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  // These members define the primary interface to the atomic field
  // definition as read from the file.
  class ElementType {
  public:
    DCSubatomicType _type;
    int _divisor;
  };

  typedef vector<ElementType> Elements;
  Elements _elements;

  enum Flags {
    F_required        = 0x0001,
    F_broadcast       = 0x0002,
    F_p2p             = 0x0004,
    F_ram             = 0x0008,
    F_db              = 0x0010,
    F_clsend          = 0x0020,
    F_clrecv          = 0x0040,
    F_ownsend         = 0x0080,
  };

  int _flags;  // A bitmask union of any of the above values.
};

#endif
