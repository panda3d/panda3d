/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxLevelEntry.cxx
 * @author drose
 * @date 2002-03-13
 */

#include "findApproxLevelEntry.h"
#include "nodePathCollection.h"
#include "pandaNode.h"
#include "indent.h"

TypeHandle FindApproxLevelEntry::_type_handle;

/**
 * Formats the entry for meaningful output.  For debugging only.
 */
void FindApproxLevelEntry::
output(std::ostream &out) const {
  out << "(" << _node_path << "):";
  if (is_solution(0)) {
    out << " solution!";
  } else {
    out << "(";
    _approx_path.output_component(out, _i);
    out << ")," << _i;
  }
}

/**
 * Writes the entire level (a linked list of entries beginning at this entry).
 * For debugging only.
 */
void FindApproxLevelEntry::
write_level(std::ostream &out, int indent_level) const {
  for (const FindApproxLevelEntry *entry = this;
       entry != nullptr;
       entry = entry->_next) {
    indent(out, indent_level);
    out << *entry << "\n";
  }
}

/**
 * Considers the node represented by the entry for matching the find path.  If
 * a solution is found, it is added to result; if the children of this node
 * should be considered, the appropriate entries are added to next_level.
 *
 * The return value is true if result now contains max_matches solutions, or
 * false if we should keep looking.
 */
bool FindApproxLevelEntry::
consider_node(NodePathCollection &result, FindApproxLevelEntry *&next_level,
              int max_matches, int increment) const {
  if (is_solution(increment)) {
    // If the entry represents a solution, save it and we're done with the
    // entry.
    result.add_path(_node_path.get_node_path());
    if (max_matches > 0 && result.get_num_paths() >= max_matches) {
      return true;
    }

    return false;
  }

  // If the entry is not itself a solution, consider its children.

  if (_approx_path.is_component_match_many(_i + increment)) {
    // Match any number, zero or more, levels of nodes.  This is the tricky
    // case that requires this whole nutty breadth-first thing.

    // This means we must reconsider our own entry with the next path entry,
    // before we consider the next entry--this supports matching zero levels
    // of nodes.

    // We used to make a temporary copy of our own record, and then increment
    // _i on that copy, but we can't do that nowadays because the
    // WorkingNodePath object stores a pointer to each previous generation,
    // which means we can't use any temporary FindApproxLevelEntry objects.
    // Instead, we pass around the increment parameter, which increments _i on
    // the fly.

    if (consider_node(result, next_level, max_matches, increment + 1)) {
      return true;
    }
  }

  PandaNode *this_node = _node_path.node();
  nassertr(this_node != nullptr, false);

  bool stashed_only = next_is_stashed(increment);

  if (!stashed_only) {
    // Check the normal list of children.
    PandaNode::Children children = this_node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = children.get_child(i);

      consider_next_step(child_node, next_level, increment);
    }
  }

  if (_approx_path.return_stashed() || stashed_only) {
    // Also check the stashed list.
    int num_stashed = this_node->get_num_stashed();
    for (int i = 0; i < num_stashed; i++) {
      PandaNode *stashed_node = this_node->get_stashed(i);

      consider_next_step(stashed_node, next_level, increment);
    }
  }

  return false;
}

/**
 * Compares the indicated child node (which is assumed to be a child of
 * _node_path) with the next component of the path.  If it matches, generates
 * whatever additional entries are appropriate and stores them in next_level.
 */
void FindApproxLevelEntry::
consider_next_step(PandaNode *child_node, FindApproxLevelEntry *&next_level,
                   int increment) const {
  nassertv(child_node != _node_path.node());

  if (!_approx_path.return_hidden() && child_node->is_overall_hidden()) {
    // If the approx path does not allow us to return hidden nodes, and this
    // node has indeed been completely hidden, then stop here.
    return;
  }

  nassertv(_i + increment < _approx_path.get_num_components());

  if (_approx_path.is_component_match_many(_i + increment)) {
    // Match any number, zero or more, levels of nodes.  This is the tricky
    // case that requires this whole nutty breadth-first thing.

    // And now we just add the next entry without incrementing its path entry.

    next_level = new FindApproxLevelEntry
      (*this, child_node, _i + increment, next_level);

  } else {
    if (_approx_path.matches_component(_i + increment, child_node)) {
      // That matched, and it consumes one path entry.
      next_level = new FindApproxLevelEntry
        (*this, child_node, _i + increment + 1, next_level);
    }
  }
}
