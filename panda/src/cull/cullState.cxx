// Filename: cullState.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullState.h"
#include "config_cull.h"

#include <indent.h>
#include <graphicsStateGuardian.h>
#include <allAttributesWrapper.h>

////////////////////////////////////////////////////////////////////
//     Function: CullState::check_currency
//       Access: Public
//  Description: Returns true if the CullState is fresh enough to still
//               apply to the indicated node and its associated cached
//               transition, false if it should be recomputed.  If
//               false, the node entry is removed.
////////////////////////////////////////////////////////////////////
bool CullState::
check_currency(Node *node, const AllTransitionsWrapper &,
	       UpdateSeq as_of) {
  // First, check the verified time stamp.
  Verified::iterator vi;
  vi = _verified.find(node);
  if (vi == _verified.end()) {
    // We have never seen this node before.
    return false;
  }

  UpdateSeq verified_stamp = (*vi).second;

  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Checking currency for " << *node << ", verified_stamp = "
      << verified_stamp << " as_of = " << as_of << "\n";
  }

  if (as_of <= verified_stamp && !verified_stamp.is_fresh()) {
    return true;
  }

  // Now we should verify the set of transitions individually.  Skip
  // that for now.

  // So we now know this node is no longer appropriate for this state.
  // Remove any record we ever had of the node.
  _verified.erase(vi);

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullState::
output(ostream &out) const {
  out << count_current_nodes() << " nodes: { " << _trans << " }";
}

////////////////////////////////////////////////////////////////////
//     Function: CullState::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullState::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _current_geom_nodes.size() << " geom nodes";

  if (!_current_geom_nodes.empty()) {
    CurrentGeomNodes::const_iterator ci;
    ci = _current_geom_nodes.begin();
    out << " (" << (*ci);
    ++ci;
    while (ci != _current_geom_nodes.end()) {
      out << ", " << (*ci);
      ++ci;
    }
    out << ")";
  }

  out << ", " << _current_direct_nodes.size() << " direct nodes";

  if (!_current_direct_nodes.empty()) {
    CurrentDirectNodes::const_iterator ci;
    ci = _current_direct_nodes.begin();
    out << " (" << (*ci);
    ++ci;
    while (ci != _current_direct_nodes.end()) {
      out << ", " << (*ci);
      ++ci;
    }
    out << ")";
  }
  out << ":\n";

  _trans.write(out, indent_level + 2);
}
