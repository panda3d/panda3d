/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypeDeclaration.h
 * @author drose
 * @date 2000-08-14
 */

#ifndef CPPTYPEDECLARATION_H
#define CPPTYPEDECLARATION_H

#include "dtoolbase.h"

#include "cppInstance.h"

/**
 * A CPPTypeDeclaration is a special declaration that represents the top-level
 * declaration of a type in a source file.  Typically this is the first
 * appearance of the type.
 */
class CPPTypeDeclaration : public CPPInstance {
public:
  CPPTypeDeclaration(CPPType *type);

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypeDeclaration *as_type_declaration();
};

#endif
