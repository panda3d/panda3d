// Filename: qpfindApproxLevelEntry.cxx
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

#include "qpfindApproxLevelEntry.h"
#include "qpnodePathCollection.h"
#include "pandaNode.h"


////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::output
//       Access: Public
//  Description: Formats the entry for meaningful output.  For
//               debugging only.
////////////////////////////////////////////////////////////////////
void qpFindApproxLevelEntry::
output(ostream &out) const {
  out << "(" << _node_path << "):";
  if (is_solution()) {
    out << " solution!";
  } else {
    out << "(";
    _approx_path.output_component(out, _i);
    out << ")," << _i;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::consider_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void qpFindApproxLevelEntry::
consider_node(qpNodePathCollection &result, qpFindApproxLevel &next_level,
              int max_matches) const {
  nassertv(_i < _approx_path.get_num_components());

  if (_approx_path.is_component_match_many(_i)) {
    // Match any number, zero or more, levels of nodes.  This is the
    // tricky case that requires this whole nutty breadth-first thing.

    // This means we must reconsider our own entry with the next path
    // entry, before we consider the next entry--this supports
    // matching zero levels of nodes.
    qpFindApproxLevelEntry reconsider(*this);
    ++reconsider._i;

    if (reconsider.is_solution()) {
      // Does this now represent a solution?
      result.add_path(reconsider._node_path);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
        return;
      }
    } else {
      reconsider.consider_node(result, next_level, max_matches);
    }
  }

  PandaNode *this_node = _node_path.node();
  nassertv(this_node != (PandaNode *)NULL);

  bool stashed_only = next_is_stashed();

  if (!stashed_only) {
    // Check the normal list of children.
    int num_children = this_node->get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = this_node->get_child(i);
      
      consider_next_step(result, child_node, next_level, max_matches);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
        return;
      }
    }
  }

  if (_approx_path.return_stashed() || stashed_only) {
    // Also check the stashed list.
    int num_stashed = this_node->get_num_stashed();
    for (int i = 0; i < num_stashed; i++) {
      PandaNode *stashed_node = this_node->get_stashed(i);
      
      consider_next_step(result, stashed_node, next_level, max_matches);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
        return;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxLevelEntry::consider_next_step
//       Access: Public
//  Description: Compares the indicated child node (which is assumed
//               to be a child of _node_path) with the next component
//               of the path.  If it matches, generates whatever
//               additional entries are appropriate and stores them in
//               next_level.
//
//               If a complete solution is found, stores it in result.
////////////////////////////////////////////////////////////////////
void qpFindApproxLevelEntry::
consider_next_step(qpNodePathCollection &result, PandaNode *child_node,
                   qpFindApproxLevel &next_level, int max_matches) const {
  if (!_approx_path.return_hidden() &&
      child_node->get_draw_mask().is_zero()) {
    // If the approx path does not allow us to return hidden nodes,
    // and this node has indeed been completely hidden, then stop
    // here.
    return;
  }

  nassertv(_i < _approx_path.get_num_components());

  if (_approx_path.is_component_match_many(_i)) {
    // Match any number, zero or more, levels of nodes.  This is the
    // tricky case that requires this whole nutty breadth-first thing.

    // And now we just add the next entry without incrementing its
    // path entry.

    qpFindApproxLevelEntry next(*this);
    next._node_path = qpNodePath(_node_path, child_node);
    next_level.add_entry(next);

  } else {
    if (_approx_path.matches_component(_i, child_node)) {
      // That matched, and it consumes one path entry.
      qpFindApproxLevelEntry next(*this);
      ++next._i;
      next._node_path = qpNodePath(_node_path, child_node);
      next_level.add_entry(next);
    }
  }
}
