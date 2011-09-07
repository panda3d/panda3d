// Filename: bulletGhostNode.h
// Created by:  enn0x (19Nov10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __BULLET_GHOST_NODE_H__
#define __BULLET_GHOST_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletBodyNode.h"

#include "pandaNode.h"
#include "collideMask.h"

class BulletShape;

////////////////////////////////////////////////////////////////////
//       Class : BulletGhostNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletGhostNode : public BulletBodyNode {

PUBLISHED:
  BulletGhostNode(const char *name="ghost");
  INLINE ~BulletGhostNode();

  // Overlapping
  INLINE int get_num_overlapping_nodes() const;
  INLINE PandaNode *get_overlapping_node(int idx) const;
  MAKE_SEQ(get_overlapping_nodes, get_num_overlapping_nodes, get_overlapping_node);

public:
  virtual btCollisionObject *get_object() const;

  void sync_p2b();
  void sync_b2p();

protected:
  virtual void parents_changed();
  virtual void transform_changed();

private:
  CPT(TransformState) _sync;
  bool _sync_disable;
  bool _sync_local;

  btPairCachingGhostObject *_ghost;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletBodyNode::init_type();
    register_type(_type_handle, "BulletGhostNode", 
                  BulletBodyNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletGhostNode.I"

#endif // __BULLET_GHOST_NODE_H__

