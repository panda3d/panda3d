// Filename: findApproxLevel.h
// Created by:  drose (13Mar02)
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

#ifndef FINDAPPROXLEVEL_H
#define FINDAPPROXLEVEL_H

#include "pandabase.h"

#include "findApproxLevelEntry.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : FindApproxLevel
// Description : This class is local to this package only; it doesn't
//               get exported.  It maintains the list of nodes
//               find_approx() considers for each level of the scene
//               graph it visits, in its breadth-first search.
////////////////////////////////////////////////////////////////////
class FindApproxLevel {
public:
  INLINE void add_entry(const FindApproxLevelEntry &entry);

  void write(ostream &out) const;

  typedef pvector<FindApproxLevelEntry> Vec;
  Vec _v;
};

#include "findApproxLevel.I"

#endif
