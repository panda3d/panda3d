/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTemplateParameterList.h
 * @author drose
 * @date 1999-10-28
 */

#ifndef CPPTEMPLATEPARAMETERLIST_H
#define CPPTEMPLATEPARAMETERLIST_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <vector>
#include <string>

class CPPScope;

/**
 * This class serves to store the parameter list for a template function or
 * class, both for the formal parameter list (given when the template is
 * defined) and for the actual parameter list (given when the template is
 * instantiated).
 */
class CPPTemplateParameterList {
public:
  CPPTemplateParameterList();

  std::string get_string() const;
  void build_subst_decl(const CPPTemplateParameterList &formal_params,
                        CPPDeclaration::SubstDecl &subst,
                        CPPScope *current_scope, CPPScope *global_scope) const;

  bool is_fully_specified() const;
  bool is_tbd() const;

  bool operator == (const CPPTemplateParameterList &other) const;
  bool operator != (const CPPTemplateParameterList &other) const;
  bool operator < (const CPPTemplateParameterList &other) const;

  CPPTemplateParameterList *substitute_decl(CPPDeclaration::SubstDecl &subst,
                                            CPPScope *current_scope,
                                            CPPScope *global_scope);

  void output(std::ostream &out, CPPScope *scope) const;
  void write_formal(std::ostream &out, CPPScope *scope) const;

  typedef std::vector<CPPDeclaration *> Parameters;
  Parameters _parameters;
};

inline std::ostream &
operator << (std::ostream &out, const CPPTemplateParameterList &plist) {
  plist.output(out, nullptr);
  return out;
}


// This is an STL function object used to uniquely order
// CPPTemplateParameterList pointers.
class CPPTPLCompare {
public:
  bool operator () (const CPPTemplateParameterList *a,
                    const CPPTemplateParameterList *b) const {
    return (*a) < (*b);
  }
};

#endif
