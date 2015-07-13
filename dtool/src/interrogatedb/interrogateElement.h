// Filename: interrogateElement.h
// Created by:  drose (11Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATEELEMENT_H
#define INTERROGATEELEMENT_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

class IndexRemapper;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateElement
// Description : An internal representation of a data element, like a
//               data member or a global variable.
////////////////////////////////////////////////////////////////////
class EXPCL_INTERROGATEDB InterrogateElement : public InterrogateComponent {
public:
  INLINE InterrogateElement(InterrogateModuleDef *def = NULL);
  INLINE InterrogateElement(const InterrogateElement &copy);
  INLINE void operator = (const InterrogateElement &copy);

  INLINE bool is_global() const;

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE bool has_comment() const;
  INLINE const string &get_comment() const;

  INLINE TypeIndex get_type() const;
  INLINE bool has_getter() const;
  INLINE FunctionIndex get_getter() const;
  INLINE bool has_setter() const;
  INLINE FunctionIndex get_setter() const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global          = 0x0001,
    F_has_getter      = 0x0002,
    F_has_setter      = 0x0004
  };

  int _flags;
  string _scoped_name;
  string _comment;
  TypeIndex _type;
  FunctionIndex _getter;
  FunctionIndex _setter;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateElement &element);
INLINE istream &operator >> (istream &in, InterrogateElement &element);

#include "interrogateElement.I"

#endif
