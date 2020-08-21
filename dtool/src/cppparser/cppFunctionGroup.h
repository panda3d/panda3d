/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppFunctionGroup.h
 * @author drose
 * @date 1999-11-11
 */

#ifndef CPPFUNCTIONGROUP_H
#define CPPFUNCTIONGROUP_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

class CPPInstance;

/**
 * This class is simply a container for one or more CPPInstances for functions
 * of the same name.  It's handy for storing in the CPPScope, so that
 * CPPScope::find_symbol() can return a single pointer to indicate all of the
 * functions that may share a given name.
 */
class CPPFunctionGroup : public CPPDeclaration {
public:
  CPPFunctionGroup(const std::string &name);
  ~CPPFunctionGroup();

  CPPType *get_return_type() const;

  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPFunctionGroup *as_function_group();

  typedef std::vector<CPPInstance *> Instances;
  Instances _instances;
  std::string _name;
};

#endif
