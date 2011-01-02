// Filename: cppInstance.h
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

#ifndef CPPINSTANCE_H
#define CPPINSTANCE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppType.h"
#include "cppIdentifier.h"
#include "cppTemplateParameterList.h"

class CPPInstanceIdentifier;
class CPPIdentifier;
class CPPParameterList;
class CPPScope;
class CPPExpression;

///////////////////////////////////////////////////////////////////
//       Class : CPPInstance
// Description :
////////////////////////////////////////////////////////////////////
class CPPInstance : public CPPDeclaration {
public:
  // Some of these flags clearly only make sense in certain contexts,
  // e.g. for a function or method.
  enum StorageClass {
    SC_static       = 0x0001,
    SC_extern       = 0x0002,
    SC_c_binding    = 0x0004,
    SC_virtual      = 0x0008,
    SC_inline       = 0x0010,
    SC_explicit     = 0x0020,
    SC_register     = 0x0040,
    SC_pure_virtual = 0x0080,
    SC_volatile     = 0x0100,
    SC_mutable      = 0x0200,

    // This bit is only set by CPPStructType::check_virtual().
    SC_inherited_virtual = 0x0400,

    // This is a special "storage class" for methods tagged with the
    // BLOCKING macro (i.e. the special __blocking keyword).  These
    // are methods that might block and therefore need to release
    // Python threads for their duration.
    SC_blocking     = 0x0800,

    // And this is for methods tagged with __extension, which declares
    // extension methods defined separately from the source code.
    SC_extension    = 0x1000,
  };

  CPPInstance(CPPType *type, const string &name, int storage_class = 0);
  CPPInstance(CPPType *type, CPPIdentifier *ident, int storage_class = 0);
  CPPInstance(CPPType *type, CPPInstanceIdentifier *ii,
              int storage_class, const CPPFile &file);
  CPPInstance(const CPPInstance &copy);
  ~CPPInstance();

  static CPPInstance *
  make_typecast_function(CPPInstance *inst, CPPIdentifier *ident,
                         CPPParameterList *parameters, int function_flags);

  bool operator == (const CPPInstance &other) const;
  bool operator != (const CPPInstance &other) const;
  bool operator < (const CPPInstance &other) const;

  void set_initializer(CPPExpression *initializer);

  bool is_scoped() const;
  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = NULL) const;

  string get_simple_name() const;
  string get_local_name(CPPScope *scope = NULL) const;
  string get_fully_scoped_name() const;

  void check_for_constructor(CPPScope *current_scope, CPPScope *global_scope);

  virtual CPPDeclaration *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = NULL) const;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  void output(ostream &out, int indent_level, CPPScope *scope,
              bool complete, int num_default_parameters) const;
  virtual SubType get_subtype() const;

  virtual CPPInstance *as_instance();

  CPPType *_type;
  CPPIdentifier *_ident;
  CPPExpression *_initializer;

  int _storage_class;

private:
  typedef map<const CPPTemplateParameterList *, CPPInstance *, CPPTPLCompare> Instantiations;
  Instantiations _instantiations;
};

#endif

