/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxLevelEntry.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef FINDAPPROXLEVELENTRY_H
#define FINDAPPROXLEVELENTRY_H

#include "pandabase.h"

#include "findApproxPath.h"
#include "workingNodePath.h"

class NodePathCollection;

/**
 * This class is local to this package only; it doesn't get exported.  It
 * represents a single node under consideration for matching at a single point
 * in the breadth-first search.
 */
class FindApproxLevelEntry {
public:
  INLINE FindApproxLevelEntry(const WorkingNodePath &node_path,
                              FindApproxPath &approx_path);
  INLINE FindApproxLevelEntry(const FindApproxLevelEntry &parent,
                              PandaNode *child_node, int i,
                              FindApproxLevelEntry *next);
  INLINE FindApproxLevelEntry(const FindApproxLevelEntry &copy);
  INLINE void operator = (const FindApproxLevelEntry &copy);
  ALLOC_DELETED_CHAIN(FindApproxLevelEntry);

  INLINE bool next_is_stashed(int increment) const;

  bool consider_node(NodePathCollection &result,
                     FindApproxLevelEntry *&next_level,
                     int max_matches, int increment) const;
  void consider_next_step(PandaNode *child_node,
                          FindApproxLevelEntry *&next_level,
                          int increment) const;
  INLINE bool is_solution(int increment) const;

  void output(std::ostream &out) const;
  void write_level(std::ostream &out, int indent_level) const;

  // _node_path represents the most recent node that we have previously
  // accepted as being a partial solution.
  WorkingNodePath _node_path;

  // _i represents the next component in the approx_path that must be matched
  // against all of the children of _node_path, above.  If _i refers to the
  // end of the approx_path, then _node_path is a solution.
  int _i;
  FindApproxPath &_approx_path;
  FindApproxLevelEntry *_next;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "FindApproxLevelEntry");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &
operator << (std::ostream &out, const FindApproxLevelEntry &entry) {
  entry.output(out);
  return out;
}

#include "findApproxLevelEntry.I"

#endif
