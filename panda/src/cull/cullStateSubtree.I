// Filename: cullStateSubtree.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: CullStateSubtree::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullStateSubtree::
CullStateSubtree(CullStateLookup *parent,
		 const AllTransitionsWrapper &trans, 
		 Node *top_subtree,
		 UpdateSeq now) :
  _parent(parent),
  _trans(trans),
  _top_subtree(top_subtree),
  _verified(now)
{
  _parent->compose_trans(_trans, _trans_from_root);
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateSubtree::update
//       Access: Public
//  Description: Indicates a possibly new set of transitions for the
//               top node of this subtree.  Updates all internal state
//               to reflect the new set, or does nothing if the set is
//               unchanged.
////////////////////////////////////////////////////////////////////
INLINE void CullStateSubtree::
update(const AllTransitionsWrapper &trans, Node *top_subtree, UpdateSeq now) {
  // We shouldn't *need* to call clear().  This will unnecessarily
  // blow out the cache for all of our children.  We do it for now
  // just to cut down on things that can go wrong, but this should
  // come out of here before long.
  //  clear();

  _trans = trans;
  _parent->compose_trans(_trans, _trans_from_root);
  _top_subtree = top_subtree;
  _verified = now;
}
