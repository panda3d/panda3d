// Filename: findApproxLevel.h
// Created by:  drose (18Feb00)
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

#ifndef FINDAPPROXLEVEL_H
#define FINDAPPROXLEVEL_H

#include <pandabase.h>

#include "findApproxPath.h"
#include "nodePath.h"

#include <string>

class Node;
class FindApproxLevel;
class NodeFinder;
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
  INLINE FindApproxLevelEntry(const NodePath &node_path,
                              FindApproxPath &approx_path);
  INLINE FindApproxLevelEntry(const FindApproxLevelEntry &copy);
  INLINE void operator = (const FindApproxLevelEntry &copy);

  INLINE bool next_is_stashed() const;

  void consider_node(NodePathCollection &result, FindApproxLevel &next_level,
                     int max_matches, TypeHandle graph_type) const;
  void consider_next_step(NodePathCollection &result,
                          NodeRelation *arc, FindApproxLevel &next_level,
                          int max_matches, TypeHandle graph_type) const;
  INLINE bool is_solution() const;

  void output(ostream &out) const;

  // _node_path represents the most recent node that we have
  // previously accepted as being a partial solution.
  NodePath _node_path;

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
