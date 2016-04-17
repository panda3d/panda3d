/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLevelState.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONLEVELSTATE_H
#define COLLISIONLEVELSTATE_H

#include "pandabase.h"

#include "collisionLevelStateBase.h"
#include "collisionNode.h"
#include "bitMask.h"
#include "doubleBitMask.h"

/**
 * This is the state information the CollisionTraverser retains for each level
 * during traversal.
 *
 * This is the template class that specifies the CurrentMask type: the type of
 * bitmask that is used to keep track of the set of active colliders for each
 * node.
 */
template<class MaskType>
class CollisionLevelState : public CollisionLevelStateBase {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  INLINE CollisionLevelState(const NodePath &node_path);
  INLINE CollisionLevelState(const CollisionLevelState<MaskType> &parent,
                             PandaNode *child);
  INLINE CollisionLevelState(const CollisionLevelState<MaskType> &copy);
  INLINE void operator = (const CollisionLevelState<MaskType> &copy);

  INLINE void clear();
  INLINE void prepare_collider(const ColliderDef &def, const NodePath &root);

  bool any_in_bounds();
  bool apply_transform();

  INLINE static bool has_max_colliders();
  INLINE static int get_max_colliders();

  INLINE bool has_collider(int n) const;
  INLINE bool has_any_collider() const;

  INLINE void omit_collider(int n);

private:
  // CurrentMask here is a locally-defined value that simply serves to keep
  // track of the colliders that are still interested in the current node.
  // Don't confuse it with CollideMask, which is a set of user-defined bits
  // that specify which CollisionSolids may possibly intersect with each
  // other.
  typedef MaskType CurrentMask;
  CurrentMask _current;

  friend class CollisionTraverser;
#endif  // CPPPARSER
};

#include "collisionLevelState.I"

// Now instantiate a handful of implementations of CollisionLevelState: one
// that uses a word-at-a-time bitmask to track the active colliders, and a
// couple that use more words at a time.

typedef CollisionLevelState<BitMaskNative> CollisionLevelStateSingle;
typedef CollisionLevelState<DoubleBitMaskNative> CollisionLevelStateDouble;
typedef CollisionLevelState<QuadBitMaskNative> CollisionLevelStateQuad;

#endif
