// Filename: findApproxLevel.cxx
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "findApproxLevel.h"
#include "nodePathCollection.h"

#include <notify.h>
#include <node.h>
#include <namedNode.h>
#include <nodeRelation.h>


////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::output
//       Access: Public
//  Description: Formats the entry for meaningful output.  For
//               debugging only.
////////////////////////////////////////////////////////////////////
void FindApproxLevelEntry::
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
//     Function: FindApproxLevelEntry::consider_node
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FindApproxLevelEntry::
consider_node(NodePathCollection &result, FindApproxLevel &next_level, 
	      int max_matches, TypeHandle graph_type) const {
  nassertv(_i < _approx_path.get_num_components());

  if (_approx_path.is_component_match_many(_i)) {
    // Match any number, zero or more, levels of nodes.  This is the
    // tricky case that requires this whole nutty breadth-first thing.
    
    // This means we must reconsider our own entry with the next path
    // entry, before we consider the next entry--this supports
    // matching zero levels of nodes.
    FindApproxLevelEntry reconsider(*this);
    ++reconsider._i;

    if (reconsider.is_solution()) {
      // Does this now represent a solution?
      result.add_path(reconsider._node_path);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
	return;
      }
    } else {
      reconsider.consider_node(result, next_level, max_matches, graph_type);
    }
  }

  Node *bottom_node = _node_path.node();
  nassertv(bottom_node != (Node *)NULL);

  TypeHandle next_graph_type = graph_type;
  if (next_is_stashed()) {
    next_graph_type = NodeRelation::get_stashed_type();
  }
  
  DownRelations::const_iterator dri;
  dri = bottom_node->_children.find(next_graph_type);
  if (dri != bottom_node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      consider_next_step(result, arc, next_level, max_matches, graph_type);
      if (max_matches > 0 && result.get_num_paths() >= max_matches) {
	return;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevelEntry::consider_next_step
//       Access: Public
//  Description: Compares the indicated child node (which is assumed
//               to be a child of _node, or a parent of _node if we
//               happen to be searching upwards) with the next
//               component of the path.  If it matches, generates
//               whatever additional entries are appropriate and
//               stores them in next_level.
//
//               If a complete solution is found, stores it in result.
////////////////////////////////////////////////////////////////////
void FindApproxLevelEntry::
consider_next_step(NodePathCollection &result,
		   NodeRelation *arc, FindApproxLevel &next_level, 
		   int max_matches, TypeHandle graph_type) const {
  nassertv(_i < _approx_path.get_num_components());

  FindApproxLevelEntry next(*this);
  bool eb = next._node_path.extend_by(arc);
  nassertv(eb);

  if (_approx_path.is_component_match_many(_i)) {
    // Match any number, zero or more, levels of nodes.  This is the
    // tricky case that requires this whole nutty breadth-first thing.
    
    // And now we just add the next entry without incrementing its
    // path entry.
    next_level.add_entry(next);

  } else {
    if (_approx_path.matches_component(_i, arc)) {
      // That matched, and it consumes one path entry.
      ++next._i;
      next_level.add_entry(next);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevel::write
//       Access: Public
//  Description: Shows the entire contents of the level, one entry per
//               line.  For debugging only.
////////////////////////////////////////////////////////////////////
void FindApproxLevel::
write(ostream &out) const {
  Vec::const_iterator vi;
  for (vi = _v.begin(); vi != _v.end(); ++vi) {
    (*vi).output(out);
    out << "\n";
  }
}
