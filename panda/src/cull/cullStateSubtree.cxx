// Filename: cullStateSubtree.cxx
// Created by:  drose (09Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullStateSubtree.h"
#include "config_cull.h"


////////////////////////////////////////////////////////////////////
//     Function: CullStateSubtree::Constructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullStateSubtree::
~CullStateSubtree() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateSubtree::check_currency
//       Access: Public
//  Description: Returns true if the CullStateSubtree is fresh enough to
//               still apply to the indicated cached transition, false
//               if it should be recomputed.
////////////////////////////////////////////////////////////////////
bool CullStateSubtree::
check_currency(const AllTransitionsWrapper &, Node *top_subtree, 
	       UpdateSeq now) {
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Checking currency for subtree " << (void *)this
      << ", _verified = " << _verified << " now = " << now << "\n";
  }

  // Make sure we've still got the same top_subtree node.
  if (_top_subtree != top_subtree) {
    return false;
  }

  // First, check the verified time stamp.
  if (_verified == now && !_verified.is_fresh()) {
    return true;
  }

  // Now we should verify the set of transitions individually.  Skip
  // that for now.

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::get_top_subtree
//       Access: Public, Virtual
//  Description: Returns the node that represents the top of the
//               subtree for this particular CullStateSubtree.
////////////////////////////////////////////////////////////////////
Node *CullStateSubtree::
get_top_subtree() const {
  return _top_subtree;
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateSubtree::compose_trans
//       Access: Public, Virtual
//  Description: Composes the transitions given in "from" with the net
//               transitions that appear above the top node of this
//               subtree, and returns the result in "to".  This is a
//               pass-thru for CullStateLookup; it has effect only for
//               CullStateSubtree.
////////////////////////////////////////////////////////////////////
void CullStateSubtree::
compose_trans(const AllTransitionsWrapper &from,
	      AllTransitionsWrapper &to) const {
  to = _trans_from_root;
  to.compose_in_place(from);
}
