/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppNamespace.h
 * @author drose
 * @date 1999-11-16
 */

#ifndef CPPNAMESPACE_H
#define CPPNAMESPACE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPScope;

/**
 *
 */
class CPPNamespace : public CPPDeclaration {
public:
  CPPNamespace(CPPIdentifier *ident, CPPScope *scope,
               const CPPFile &file);

  std::string get_simple_name() const;
  std::string get_local_name(CPPScope *scope = nullptr) const;
  std::string get_fully_scoped_name() const;
  CPPScope *get_scope() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPNamespace *as_namespace();

  // We can't call this _inline since that would clash with an MSVC built-in
  // keyword declaration.
  bool _is_inline;

private:
  CPPIdentifier *_ident;
  CPPScope *_scope;
};

#endif
