// Filename: qpfindApproxLevelEntry.h
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

#ifndef qpFINDAPPROXLEVELENTRY_H
#define qpFINDAPPROXLEVELENTRY_H

#include "pandabase.h"

#include "qpfindApproxPath.h"
#include "qpnodePath.h"

class qpFindApproxLevel;
class qpNodePathCollection;

////////////////////////////////////////////////////////////////////
//       Class : qpFindApproxLevelEntry
// Description : This class is local to this package only; it doesn't
//               get exported.  It represents a single node under
//               consideration for matching at a single point in the
//               breadth-first search.
////////////////////////////////////////////////////////////////////
class qpFindApproxLevelEntry {
public:
  INLINE qpFindApproxLevelEntry(const qpNodePath &node_path,
                              qpFindApproxPath &approx_path);
  INLINE qpFindApproxLevelEntry(const qpFindApproxLevelEntry &copy);
  INLINE void operator = (const qpFindApproxLevelEntry &copy);

  INLINE bool next_is_stashed() const;

  void consider_node(qpNodePathCollection &result, qpFindApproxLevel &next_level,
                     int max_matches) const;
  void consider_next_step(qpNodePathCollection &result,
                          PandaNode *child_node, qpFindApproxLevel &next_level,
                          int max_matches) const;
  INLINE bool is_solution() const;

  void output(ostream &out) const;

  // _node_path represents the most recent node that we have
  // previously accepted as being a partial solution.
  qpNodePath _node_path;

  // _i represents the next component in the approx_path that must be
  // matched against all of the children of _node_path, above.  If _i
  // refers to the end of the approx_path, then _node_path is a
  // solution.
  int _i;
  qpFindApproxPath &_approx_path;
};

INLINE ostream &
operator << (ostream &out, const qpFindApproxLevelEntry &entry) {
  entry.output(out);
  return out;
}

#include "qpfindApproxLevelEntry.I"

#endif
