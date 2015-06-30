// Filename: interrogateManifest.h
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

#ifndef INTERROGATEMANIFEST_H
#define INTERROGATEMANIFEST_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

class IndexRemapper;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateManifest
// Description : An internal representation of a manifest constant.
////////////////////////////////////////////////////////////////////
class EXPCL_INTERROGATEDB InterrogateManifest : public InterrogateComponent {
public:
  INLINE InterrogateManifest(InterrogateModuleDef *def = NULL);
  INLINE InterrogateManifest(const InterrogateManifest &copy);
  INLINE void operator = (const InterrogateManifest &copy);

  INLINE const string &get_definition() const;
  INLINE bool has_type() const;
  INLINE TypeIndex get_type() const;
  INLINE bool has_getter() const;
  INLINE FunctionIndex get_getter() const;
  INLINE bool has_int_value() const;
  INLINE int get_int_value() const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_has_type        = 0x0001,
    F_has_getter      = 0x0002,
    F_has_int_value   = 0x0004
  };

  int _flags;
  string _definition;
  int _int_value;
  TypeIndex _type;
  FunctionIndex _getter;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateManifest &manifest);
INLINE istream &operator >> (istream &in, InterrogateManifest &manifest);

#include "interrogateManifest.I"

#endif
