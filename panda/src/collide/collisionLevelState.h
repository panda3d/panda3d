// Filename: collisionLevelState.h
// Created by:  drose (24Apr00)
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

#ifndef COLLISIONLEVELSTATE_H
#define COLLISIONLEVELSTATE_H

#include <pandabase.h>

#include <luse.h>
#include <pointerToArray.h>
#include <geometricBoundingVolume.h>
#include <arcChain.h>

#include "plist.h"

class CollisionSolid;
class CollisionNode;

////////////////////////////////////////////////////////////////////
//       Class : CollisionLevelState
// Description : This is the state information the
//               CollisionTraverser retains for each level during
//               traversal.
////////////////////////////////////////////////////////////////////
class CollisionLevelState {
public:
  class ColliderDef {
  public:
    CollisionSolid *_collider;
    CollisionNode *_node;
    LMatrix4f _space;
    LMatrix4f _inv_space;
  };

  INLINE CollisionLevelState(const ArcChain &arc_chain);
  INLINE CollisionLevelState(const CollisionLevelState &copy);
  INLINE void operator = (const CollisionLevelState &copy);

  void clear();
  void reserve(int max_colliders);
  void prepare_collider(const ColliderDef &def);
  void xform(const LMatrix4f &mat);

  INLINE void forward_arc(NodeRelation *arc);
  INLINE const ArcChain &get_arc_chain() const;

  INLINE int get_num_colliders() const;
  INLINE bool has_collider(int n) const;
  INLINE bool has_collider_with_geom(int n) const;
  INLINE bool has_any_collider() const;
  INLINE bool has_any_collide_geom() const;

  INLINE void reached_collision_node();

  INLINE CollisionSolid *get_collider(int n) const;
  INLINE CollisionNode *get_node(int n) const;
  INLINE const LMatrix4f &get_space(int n) const;
  INLINE const LMatrix4f &get_inv_space(int n) const;
  INLINE const GeometricBoundingVolume *get_local_bound(int n) const;

  INLINE void omit_collider(int n);

private:
  typedef int ColliderMask;

  INLINE ColliderMask get_mask(int n) const;

  ArcChain _arc_chain;

  typedef PTA(ColliderDef) Colliders;
  Colliders _colliders;
  ColliderMask _current;
  ColliderMask _colliders_with_geom;

  typedef PTA(CPT(GeometricBoundingVolume)) BoundingVolumes;
  BoundingVolumes _local_bounds;
};

#include "collisionLevelState.I"

#endif


