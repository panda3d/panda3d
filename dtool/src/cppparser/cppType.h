// Filename: cppType.h
// Created by:  drose (19Oct99)
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

#ifndef CPPTYPE_H
#define CPPTYPE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <set>

class CPPType;
class CPPTypedef;
class CPPTypeDeclaration;


// This is an STL function object used to uniquely order CPPType
// pointers.
class CPPTypeCompare {
public:
  bool operator () (CPPType *a, CPPType *b) const;
};

///////////////////////////////////////////////////////////////////
//       Class : CPPType
// Description :
////////////////////////////////////////////////////////////////////
class CPPType : public CPPDeclaration {
public:
  typedef vector<CPPTypedef *> Typedefs;
  Typedefs _typedefs;

  CPPType(const CPPFile &file);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;
  virtual bool is_parameter_expr() const;

  bool has_typedef_name() const;
  string get_typedef_name(CPPScope *scope = NULL) const;

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;
  virtual string get_preferred_name() const;

  virtual bool is_incomplete() const;
  virtual bool is_equivalent(const CPPType &other) const;

  void output_instance(ostream &out, const string &name,
                       CPPScope *scope) const;
  virtual void output_instance(ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const string &prename,
                               const string &name) const;

  virtual CPPType *as_type();


  static CPPType *new_type(CPPType *type);

  static void record_preferred_name_for(const CPPType *type, const string &name);
  static string get_preferred_name_for(const CPPType *type);

  CPPTypeDeclaration *_declaration;

protected:
  typedef set<CPPType *, CPPTypeCompare> Types;
  static Types _types;

  typedef map<string, string> PreferredNames;
  static PreferredNames _preferred_names;
};

#endif
