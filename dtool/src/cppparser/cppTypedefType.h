// Filename: cppTypedefType.h
// Created by:  rdb (01Aug14)
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

#ifndef CPPTYPEDEFTYPE_H
#define CPPTYPEDEFTYPE_H

#include "dtoolbase.h"
#include "cppType.h"

class CPPIdentifier;
class CPPInstanceIdentifier;

///////////////////////////////////////////////////////////////////
//       Class : CPPTypedefType
// Description :
////////////////////////////////////////////////////////////////////
class CPPTypedefType : public CPPType {
public:
  CPPTypedefType(CPPType *type, const string &name, CPPScope *current_scope);
  CPPTypedefType(CPPType *type, CPPIdentifier *ident, CPPScope *current_scope);
  CPPTypedefType(CPPType *type, CPPInstanceIdentifier *ii,
                 CPPScope *current_scope, const CPPFile &file);

  bool is_scoped() const;
  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = NULL) const;

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;

  virtual bool is_incomplete() const;
  virtual bool is_tbd() const;
  virtual bool is_trivial() const;

  virtual bool is_fully_specified() const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_equivalent(const CPPType &other) const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypedefType *as_typedef_type();

  CPPType *_type;
  CPPIdentifier *_ident;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;

  bool _subst_decl_recursive_protect;
  typedef vector<CPPTypeProxy *> Proxies;
  Proxies _proxies;
};

#endif
