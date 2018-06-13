/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppUsing.cxx
 * @author drose
 * @date 1999-11-16
 */

#include "cppUsing.h"
#include "cppIdentifier.h"

/**
 *
 */
CPPUsing::
CPPUsing(CPPIdentifier *ident, bool full_namespace, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident), _full_namespace(full_namespace)
{
}

/**
 *
 */
void CPPUsing::
output(std::ostream &out, int, CPPScope *, bool) const {
  out << "using ";
  if (_full_namespace) {
    out << "namespace ";
  }
  out << *_ident;
}

/**
 *
 */
CPPDeclaration::SubType CPPUsing::
get_subtype() const {
  return ST_using;
}

/**
 *
 */
CPPUsing *CPPUsing::
as_using() {
  return this;
}
