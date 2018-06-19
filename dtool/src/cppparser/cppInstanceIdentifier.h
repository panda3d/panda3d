/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppInstanceIdentifier.h
 * @author drose
 * @date 1999-10-21
 */

#ifndef CPPINSTANCEIDENTIFIER_H
#define CPPINSTANCEIDENTIFIER_H

#include "dtoolbase.h"

#include <vector>
#include <string>

class CPPIdentifier;
class CPPParameterList;
class CPPType;
class CPPExpression;
class CPPScope;
class CPPPreprocessor;

enum CPPInstanceIdentifierType {
  IIT_pointer,
  IIT_reference,
  IIT_rvalue_reference,
  IIT_scoped_pointer,
  IIT_array,
  IIT_const,
  IIT_volatile,
  IIT_paren,
  IIT_func,
  IIT_initializer,
};

/**
 * This class is used in parser.y to build up a variable instance definition.
 * An instance is something like 'int *&a'; the InstanceIdentifier stores
 * everything to the right of the typename.  Later this can be passed to
 * make_instance() to construct a CPPInstance.
 */
class CPPInstanceIdentifier {
public:
  CPPInstanceIdentifier(CPPIdentifier *ident);

  CPPType *unroll_type(CPPType *start_type);

  void add_modifier(CPPInstanceIdentifierType type);
  void add_func_modifier(CPPParameterList *params, int flags,
                         CPPType *trailing_return_type = nullptr);
  void add_scoped_pointer_modifier(CPPIdentifier *scoping);
  void add_array_modifier(CPPExpression *expr);
  void add_initializer_modifier(CPPParameterList *params);

  void add_trailing_return_type(CPPType *type);

  CPPParameterList *get_initializer() const;

  CPPScope *get_scope(CPPScope *current_scope, CPPScope *global_scope,
                      CPPPreprocessor *error_sink = nullptr) const;

  CPPIdentifier *_ident;

  class Modifier {
  public:
    Modifier(CPPInstanceIdentifierType type);
    static Modifier func_type(CPPParameterList *params, int flags,
                              CPPType *trailing_return_type);
    static Modifier array_type(CPPExpression *expr);
    static Modifier scoped_pointer_type(CPPIdentifier *scoping);
    static Modifier initializer_type(CPPParameterList *params);

    CPPInstanceIdentifierType _type;
    CPPParameterList *_func_params;
    int _func_flags;
    CPPIdentifier *_scoping;
    CPPExpression *_expr;
    CPPType *_trailing_return_type;
  };
  typedef std::vector<Modifier> Modifiers;
  Modifiers _modifiers;

  // If not -1, indicates a bitfield
  int _bit_width;

  // Indicates a parameter pack
  bool _packed;

private:
  CPPType *
  r_unroll_type(CPPType *start_type, Modifiers::const_iterator mi);
};

#endif
