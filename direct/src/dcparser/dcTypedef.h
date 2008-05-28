// Filename: dcTypedef.h
// Created by:  drose (17Jun04)
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

#ifndef DCTYPEDEF_H
#define DCTYPEDEF_H

#include "dcbase.h"
#include "dcDeclaration.h"

class DCParameter;

////////////////////////////////////////////////////////////////////
//       Class : DCTypedef
// Description : This represents a single typedef declaration in the
//               dc file.  It assigns a particular type to a new name,
//               just like a C typedef.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCTypedef : public DCDeclaration {
public:
  DCTypedef(DCParameter *parameter, bool implicit = false);
  DCTypedef(const string &name);
  virtual ~DCTypedef();

PUBLISHED:
  int get_number() const;
  const string &get_name() const;
  string get_description() const;

  bool is_bogus_typedef() const;
  bool is_implicit_typedef() const;

public:
  DCParameter *make_new_parameter() const;

  void set_number(int number);
  virtual void output(ostream &out, bool brief) const;
  virtual void write(ostream &out, bool brief, int indent_level) const;

private:
  DCParameter *_parameter;
  bool _bogus_typedef;
  bool _implicit_typedef;
  int _number;
};

#endif
