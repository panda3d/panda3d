// Filename: cullStateSubtree.I
// Created by:  drose (13Apr00)
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
