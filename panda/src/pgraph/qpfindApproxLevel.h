// Filename: qpfindApproxLevel.h
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef qpFINDAPPROXLEVEL_H
#define qpFINDAPPROXLEVEL_H

#include "pandabase.h"

#include "qpfindApproxLevelEntry.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : qpFindApproxLevel
// Description : This class is local to this package only; it doesn't
//               get exported.  It maintains the list of nodes
//               find_approx() considers for each level of the scene
//               graph it visits, in its breadth-first search.
////////////////////////////////////////////////////////////////////
class qpFindApproxLevel {
public:
  INLINE void add_entry(const qpFindApproxLevelEntry &entry);

  void write(ostream &out) const;

  typedef pvector<qpFindApproxLevelEntry> Vec;
  Vec _v;
};

#include "qpfindApproxLevel.I"

#endif
