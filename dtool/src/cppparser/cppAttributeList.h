/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppAttributeList.h
 * @author rdb
 * @date 2022-10-23
 */

#ifndef CPPATTRIBUTELIST_H
#define CPPATTRIBUTELIST_H

#include "dtoolbase.h"

#include <vector>

class CPPExpression;
class CPPIdentifier;
class CPPScope;
class CPPType;

/**
 * A list of square-bracket attributes and/or alignas specifiers.
 */
class CPPAttributeList {
public:
  bool is_empty() const;
  bool has_attribute(const std::string &name) const;

  bool operator == (const CPPAttributeList &other) const;
  bool operator != (const CPPAttributeList &other) const;
  bool operator < (const CPPAttributeList &other) const;

  void add_attribute(CPPIdentifier *ident);
  void add_attribute(CPPIdentifier *ident, std::string reason);

  void add_alignas(int size);
  void add_alignas(CPPType *type);
  void add_alignas(CPPExpression *expr);

  void add_attributes_from(const CPPAttributeList &other);

  struct Attribute {
    CPPIdentifier *_ident;
    std::string _reason;
  };

  typedef std::vector<Attribute> Attributes;
  Attributes _attributes;
  CPPExpression *_alignas = nullptr;

  void output(std::ostream &out, CPPScope *scope) const;
};

inline std::ostream &
operator << (std::ostream &out, const CPPAttributeList &alist) {
  alist.output(out, nullptr);
  return out;
}

#endif
