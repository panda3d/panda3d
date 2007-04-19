// Filename: collisionLevelStateBase.h
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

#ifndef COLLISIONLEVELSTATEBASE_H
#define COLLISIONLEVELSTATEBASE_H

#include "pandabase.h"

#include "luse.h"
#include "pointerToArray.h"
#include "geometricBoundingVolume.h"
#include "nodePath.h"
#include "workingNodePath.h"
#include "pointerTo.h"
#include "plist.h"
#include "pStatCollector.h"
#include "bitMask.h"

class CollisionSolid;
class CollisionNode;

////////////////////////////////////////////////////////////////////
//       Class : CollisionLevelStateBase
// Description : This is the state information the
//               CollisionTraverser retains for each level during
//               traversal.
//
//               The CollisionLevelStateBase is the non-template base
//               class.  The template version further specifies this
//               on CurrentMask type.
////////////////////////////////////////////////////////////////////
class CollisionLevelStateBase {
public:
  class ColliderDef {
  public:
    CPT(CollisionSolid) _collider;
    CollisionNode *_node;
    NodePath _node_path;
  };

  INLINE CollisionLevelStateBase(const NodePath &node_path);
  INLINE CollisionLevelStateBase(const CollisionLevelStateBase &parent, 
                             PandaNode *child);
  INLINE CollisionLevelStateBase(const CollisionLevelStateBase &copy);
  INLINE void operator = (const CollisionLevelStateBase &copy);

  void clear();
  void reserve(int num_colliders);
  void prepare_collider(const ColliderDef &def, const NodePath &root);
  
  INLINE NodePath get_node_path() const;
  INLINE PandaNode *node() const;

  INLINE int get_num_colliders() const;

  INLINE const CollisionSolid *get_collider(int n) const;
  INLINE CollisionNode *get_collider_node(int n) const;
  INLINE NodePath get_collider_node_path(int n) const;
  INLINE const GeometricBoundingVolume *get_local_bound(int n) const;
  INLINE const GeometricBoundingVolume *get_parent_bound(int n) const;

  INLINE void set_include_mask(CollideMask include_mask);
  INLINE CollideMask get_include_mask() const;

protected:
  WorkingNodePath _node_path;

  typedef PTA(ColliderDef) Colliders;
  Colliders _colliders;
  CollideMask _include_mask;

  typedef PTA(CPT(GeometricBoundingVolume)) BoundingVolumes;
  BoundingVolumes _local_bounds;
  BoundingVolumes _parent_bounds;

  static PStatCollector _node_volume_pcollector;

  friend class CollisionTraverser;
};

#include "collisionLevelStateBase.I"

#endif


