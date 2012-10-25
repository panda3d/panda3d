// Filename: bulletBaseCharacterControllerNode.h
// Created by:  enn0x (21Nov10)
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

#ifndef __BULLET_BASE_CHARACTER_CONTROLLER_NODE_H__
#define __BULLET_BASE_CHARACTER_CONTROLLER_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "pandaNode.h"
#include "collideMask.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletBaseCharacterControllerNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletBaseCharacterControllerNode : public PandaNode {

PUBLISHED:
  BulletBaseCharacterControllerNode(const char *name="character");

public:
  virtual CollideMask get_legal_collide_mask() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

  virtual btPairCachingGhostObject *get_ghost() const = 0;
  virtual btCharacterControllerInterface *get_character() const = 0;

  virtual void sync_p2b(PN_stdfloat dt, int num_substeps) = 0;
  virtual void sync_b2p() = 0;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "BulletBaseCharacterControllerNode", 
                  PandaNode::get_class_type());
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

#include "bulletBaseCharacterControllerNode.I"

#endif // __BULLET_BASE_CHARACTER_CONTROLLER_NODE_H__

