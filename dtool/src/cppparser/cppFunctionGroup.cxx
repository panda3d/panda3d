/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppFunctionGroup.cxx
 * @author drose
 * @date 1999-11-11
 */

#include "cppFunctionGroup.h"
#include "cppFunctionType.h"
#include "cppInstance.h"
#include "indent.h"

/**
 *
 */
CPPFunctionGroup::
CPPFunctionGroup(const std::string &name) :
  CPPDeclaration(CPPFile()),
  _name(name)
{
}

/**
 *
 */
CPPFunctionGroup::
~CPPFunctionGroup() {
}

/**
 * If all the functions that share this name have the same return type,
 * returns that type.  Otherwise, if some functions have different return
 * types, returns NULL.
 */
CPPType *CPPFunctionGroup::
get_return_type() const {
  CPPType *return_type = nullptr;

  if (!_instances.empty()) {
    Instances::const_iterator ii = _instances.begin();
    return_type = (*ii)->_type->as_function_type()->_return_type;
    ++ii;
    while (ii != _instances.end()) {
      if ((*ii)->_type->as_function_type()->_return_type != return_type) {
        return nullptr;
      }
      ++ii;
    }
  }

  return return_type;
}

/**
 *
 */
void CPPFunctionGroup::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!_instances.empty()) {
    Instances::const_iterator ii = _instances.begin();
    (*ii)->output(out, indent_level, scope, complete);
    ++ii;
    while (ii != _instances.end()) {
      out << ";\n";
      indent(out, indent_level);
      (*ii)->output(out, indent_level, scope, complete);
      ++ii;
    }
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPFunctionGroup::
get_subtype() const {
  return ST_function_group;
}

/**
 *
 */
CPPFunctionGroup *CPPFunctionGroup::
as_function_group() {
  return this;
}
