// Filename: dcDeclaration.h
// Created by:  drose (18Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DCDECLARATION_H
#define DCDECLARATION_H

#include "dcbase.h"

class DCClass;
class DCSwitch;

////////////////////////////////////////////////////////////////////
//       Class : DCDeclaration
// Description : This is a common interface for a declaration in a DC
//               file.  Currently, this is either a class or a typedef
//               declaration (import declarations are still collected
//               together at the top, and don't inherit from this
//               object).  Its only purpose is so that classes and
//               typedefs can be stored in one list together so they
//               can be ordered correctly on output.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCDeclaration {
public:
  virtual ~DCDeclaration();

PUBLISHED:
  virtual DCClass *as_class();
  virtual DCSwitch *as_switch();

public:
  virtual void write(ostream &out, bool brief, int indent_level) const=0;
};

#endif

