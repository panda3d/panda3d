/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypeDeclaration.cxx
 * @author drose
 * @date 2000-08-14
 */

#include "cppTypeDeclaration.h"

/**
 * Constructs a new CPPTypeDeclaration object for the given type.
 */
CPPTypeDeclaration::
CPPTypeDeclaration(CPPType *type) :
  CPPInstance(type, (CPPIdentifier *)NULL)
{
  assert(_type != NULL);
  if (_type->_declaration == (CPPTypeDeclaration *)NULL) {
    _type->_declaration = this;
  }
}

/**
 *
 */
CPPDeclaration *CPPTypeDeclaration::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPDeclaration *decl =
    CPPInstance::substitute_decl(subst, current_scope, global_scope);
  assert(decl != NULL);
  if (decl->as_type_declaration()) {
    return decl;
  }
  assert(decl->as_instance() != NULL);
  return new CPPTypeDeclaration(decl->as_instance()->_type);
}

/**
 *
 */
void CPPTypeDeclaration::
output(ostream &out, int indent_level, CPPScope *scope, bool) const {
  _type->output(out, indent_level, scope, true);
}

/**
 *
 */
CPPDeclaration::SubType CPPTypeDeclaration::
get_subtype() const {
  return ST_type_declaration;
}

/**
 *
 */
CPPTypeDeclaration *CPPTypeDeclaration::
as_type_declaration() {
  return this;
}
