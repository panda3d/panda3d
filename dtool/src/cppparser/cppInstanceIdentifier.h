// Filename: cppInstanceIdentifier.h
// Created by:  drose (21Oct99)
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

#ifndef CPPINSTANCEIDENTIFIER_H
#define CPPINSTANCEIDENTIFIER_H

#include "dtoolbase.h"

#include <vector>
#include <string>

using namespace std;

class CPPIdentifier;
class CPPParameterList;
class CPPType;
class CPPExpression;
class CPPScope;
class CPPPreprocessor;

enum CPPInstanceIdentifierType {
  IIT_pointer,
  IIT_reference,
  IIT_scoped_pointer,
  IIT_array,
  IIT_const,
  IIT_paren,
  IIT_func,
};

///////////////////////////////////////////////////////////////////
//       Class : CPPInstanceIdentifier
// Description : This class is used in parser.y to build up a variable
//               instance definition.  An instance is something like
//               'int *&a'; the InstanceIdentifier stores everything
//               to the right of the typename.  Later this can be
//               passed to make_instance() to construct a CPPInstance.
////////////////////////////////////////////////////////////////////
class CPPInstanceIdentifier {
public:
  CPPInstanceIdentifier(CPPIdentifier *ident);

  CPPType *unroll_type(CPPType *start_type);

  void add_modifier(CPPInstanceIdentifierType type);
  void add_func_modifier(CPPParameterList *params, int flags);
  void add_scoped_pointer_modifier(CPPIdentifier *scoping);
  void add_array_modifier(CPPExpression *expr);

  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = NULL) const;

  CPPIdentifier *_ident;

  class Modifier {
  public:
    Modifier(CPPInstanceIdentifierType type);
    static Modifier func_type(CPPParameterList *params, int flags);
    static Modifier array_type(CPPExpression *expr);
    static Modifier scoped_pointer_type(CPPIdentifier *scoping);

    CPPInstanceIdentifierType _type;
    CPPParameterList *_func_params;
    int _func_flags;
    CPPIdentifier *_scoping;
    CPPExpression *_expr;
  };
  typedef vector<Modifier> Modifiers;
  Modifiers _modifiers;

private:
  CPPType *
  r_unroll_type(CPPType *start_type, Modifiers::const_iterator mi);
};

#endif
