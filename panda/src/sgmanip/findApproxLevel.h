// Filename: findApproxLevel.h
// Created by:  drose (18Feb00)
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

  void consider_next_step(NodePathCollection &result,
			  NodeRelation *arc, FindApproxLevel &next_level, 
			  int max_matches) const;
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

  typedef vector<FindApproxLevelEntry> Vec;
  Vec _v;
};

#include "findApproxLevel.I"

#endif
