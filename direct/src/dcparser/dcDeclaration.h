/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcDeclaration.h
 * @author drose
 * @date 2004-06-18
 */

#ifndef DCDECLARATION_H
#define DCDECLARATION_H

#include "dcbase.h"

class DCClass;
class DCSwitch;

/**
 * This is a common interface for a declaration in a DC file.  Currently, this
 * is either a class or a typedef declaration (import declarations are still
 * collected together at the top, and don't inherit from this object).  Its
 * only purpose is so that classes and typedefs can be stored in one list
 * together so they can be ordered correctly on output.
 */
class EXPCL_DIRECT_DCPARSER DCDeclaration {
public:
  virtual ~DCDeclaration();

PUBLISHED:
  virtual DCClass *as_class();
  virtual const DCClass *as_class() const;
  virtual DCSwitch *as_switch();
  virtual const DCSwitch *as_switch() const;

  virtual void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

public:
  virtual void output(std::ostream &out, bool brief) const=0;
  virtual void write(std::ostream &out, bool brief, int indent_level) const=0;
};

INLINE std::ostream &operator << (std::ostream &out, const DCDeclaration &decl) {
  decl.output(out);
  return out;
}

#endif
