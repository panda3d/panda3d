// Filename: cppTypeDeclaration.h
// Created by:  drose (14Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CPPTYPEDECLARATION_H
#define CPPTYPEDECLARATION_H

#include <dtoolbase.h>

#include "cppInstance.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPTypeDeclaration
// Description : A CPPTypeDeclaration is a special declaration that
//               represents the top-level declaration of a type in a
//               source file.  Typically this is the first appearance
//               of the type.
////////////////////////////////////////////////////////////////////
class CPPTypeDeclaration : public CPPInstance {
public:
  CPPTypeDeclaration(CPPType *type);

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTypeDeclaration *as_type_declaration();
};

#endif

