/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTBDType.h
 * @author drose
 * @date 1999-11-05
 */

#ifndef CPPTBDTYPE_H
#define CPPTBDTYPE_H

#include "dtoolbase.h"

#include "cppType.h"

class CPPIdentifier;

/**
 * This represents a type whose exact meaning is still to-be-determined.  It
 * happens when a typename is referenced in a template class (especially using
 * the 'typename' keyword) but the actual type cannot be known until the class
 * is instantiated.
 */
class CPPTBDType : public CPPType {
public:
  CPPTBDType(CPPIdentifier *ident);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPTBDType *as_tbd_type();

  CPPIdentifier *_ident;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;

private:
  bool _subst_decl_recursive_protect;
};

#endif
