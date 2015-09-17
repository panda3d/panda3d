// Filename: cppType.h
// Created by:  drose (19Oct99)
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

#ifndef CPPTYPE_H
#define CPPTYPE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <set>

class CPPType;
class CPPTypedefType;
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
  typedef vector<CPPTypedefType *> Typedefs;
  Typedefs _typedefs;

  CPPType(const CPPFile &file);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;
  virtual bool is_trivial() const;
  virtual bool is_parameter_expr() const;

  bool has_typedef_name() const;
  string get_typedef_name(CPPScope *scope = NULL) const;

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;
  virtual string get_preferred_name() const;
  int get_num_alt_names() const;
  string get_alt_name(int n) const;

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

  static void record_alt_name_for(const CPPType *type, const string &name);
  static string get_preferred_name_for(const CPPType *type);

  CPPTypeDeclaration *_declaration;
  bool _forcetype;

protected:
  typedef set<CPPType *, CPPTypeCompare> Types;
  static Types _types;

  typedef map<string, string> PreferredNames;
  static PreferredNames _preferred_names;

  typedef vector<string> Names;
  typedef map<string, Names> AltNames;
  static AltNames _alt_names;
};

#endif
