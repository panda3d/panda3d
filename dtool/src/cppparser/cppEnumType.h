/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppEnumType.h
 * @author drose
 * @date 1999-10-25
 */

#ifndef CPPENUMTYPE_H
#define CPPENUMTYPE_H

#include "dtoolbase.h"

#include "cppBisonDefs.h"
#include "cppExtensionType.h"

#include <vector>

class CPPExpression;
class CPPInstance;
class CPPScope;


/**
 *
 */
class CPPEnumType : public CPPExtensionType {
public:
  CPPEnumType(Type type, CPPIdentifier *ident, CPPScope *current_scope,
              CPPScope *scope, const CPPFile &file);
  CPPEnumType(Type type, CPPIdentifier *ident, CPPType *element_type,
              CPPScope *current_scope, CPPScope *scope, const CPPFile &file);

  bool is_scoped() const;
  CPPType *get_underlying_type();

  CPPInstance *add_element(const std::string &name, CPPExpression *value,
                           CPPPreprocessor *preprocessor, const cppyyltype &pos);

  virtual bool is_incomplete() const;

  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPEnumType *as_enum_type();

  CPPScope *_parent_scope;
  CPPScope *_scope;
  CPPType *_element_type;

  typedef std::vector<CPPInstance *> Elements;
  Elements _elements;
  CPPExpression *_last_value;
};


#endif
