/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppUsing.h
 * @author drose
 * @date 1999-11-16
 */

#ifndef CPPUSING_H
#define CPPUSING_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPScope;

/**
 *
 */
class CPPUsing : public CPPDeclaration {
public:
  CPPUsing(CPPIdentifier *ident, bool full_namespace, const CPPFile &file);

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPUsing *as_using();

  CPPIdentifier *_ident;
  bool _full_namespace;
};

#endif
