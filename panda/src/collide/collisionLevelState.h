// Filename: collisionLevelState.h
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONLEVELSTATE_H
#define COLLISIONLEVELSTATE_H

#include "pandabase.h"

#include "collisionLevelStateBase.h"
#include "collisionNode.h"
#include "bitMask.h"
#include "bitArray.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionLevelState
// Description : This is the state information the
//               CollisionTraverser retains for each level during
//               traversal.
//
//               This is the template class that specifies the
//               CurrentMask type: the type of bitmask that is used to
//               keep track of the set of active colliders for each
//               node.
////////////////////////////////////////////////////////////////////
template<class MaskType>
class CollisionLevelState : public CollisionLevelStateBase {
public:
  INLINE CollisionLevelState(const NodePath &node_path);
  INLINE CollisionLevelState(const CollisionLevelState<MaskType> &parent, 
                             PandaNode *child);
  INLINE CollisionLevelState(const CollisionLevelState<MaskType> &copy);
  INLINE void operator = (const CollisionLevelState<MaskType> &copy);

  INLINE void clear();
  INLINE void prepare_collider(const ColliderDef &def, const NodePath &root);

  bool any_in_bounds();
  void apply_transform();

  INLINE static bool has_max_colliders();
  INLINE static int get_max_colliders();

  INLINE bool has_collider(int n) const;
  INLINE bool has_any_collider() const;

  INLINE void omit_collider(int n);

private:
  // CurrentMask here is a locally-defined value that simply serves
  // to keep track of the colliders that are still interested in the
  // current node.  Don't confuse it with CollideMask, which is a set
  // of user-defined bits that specify which CollisionSolids may
  // possibly intersect with each other.
  typedef MaskType CurrentMask;
  CurrentMask _current;

  friend class CollisionTraverser;
};

#include "collisionLevelState.I"

// Now instantiate a pair of implementations of CollisionLevelState:
// one that uses a word-at-a-time bitmask to track the active
// colliders (and thus is limited to handling 32 or 64 colliders in a
// given pass), and another that uses an infinite BitArray to track
// these colliders (and thus has no particular limit).

typedef CollisionLevelState<BitMaskNative> CollisionLevelStateWord;
typedef CollisionLevelState<BitArray> CollisionLevelStateArray;

#endif


