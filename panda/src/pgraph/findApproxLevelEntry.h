// Filename: findApproxLevelEntry.h
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

#ifndef FINDAPPROXLEVELENTRY_H
#define FINDAPPROXLEVELENTRY_H

#include "pandabase.h"

#include "findApproxPath.h"
#include "workingNodePath.h"

class FindApproxLevel;
class NodePathCollection;

////////////////////////////////////////////////////////////////////
//       Class : FindApproxLevelEntry
// Description : This class is local to this package only; it doesn't
//               get exported.  It represents a single node under
//               consideration for matching at a single point in the
//               breadth-first search.
////////////////////////////////////////////////////////////////////
class FindApproxLevelEntry {
public:
  INLINE FindApproxLevelEntry(const WorkingNodePath &node_path,
                              FindApproxPath &approx_path);
  INLINE FindApproxLevelEntry(const FindApproxLevelEntry &copy, int increment = 0);
  INLINE void operator = (const FindApproxLevelEntry &copy);

  INLINE bool next_is_stashed(int increment) const;

  void consider_node(NodePathCollection &result, FindApproxLevel &next_level,
                     int max_matches, int increment) const;
  void consider_next_step(NodePathCollection &result,
                          PandaNode *child_node, FindApproxLevel &next_level,
                          int max_matches, int increment) const;
  INLINE bool is_solution(int increment) const;

  void output(ostream &out) const;

  // _node_path represents the most recent node that we have
  // previously accepted as being a partial solution.
  WorkingNodePath _node_path;

  // _i represents the next component in the approx_path that must be
  // matched against all of the children of _node_path, above.  If _i
  // refers to the end of the approx_path, then _node_path is a
  // solution.
  int _i;
  FindApproxPath &_approx_path;
};

INLINE ostream &
operator << (ostream &out, const FindApproxLevelEntry &entry) {
  entry.output(out);
  return out;
}

#include "findApproxLevelEntry.I"

#endif
