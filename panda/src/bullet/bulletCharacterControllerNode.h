// Filename: bulletCharacterControllerNode.h
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

#ifndef __BULLET_CHARACTER_CONTROLLER_NODE_H__
#define __BULLET_CHARACTER_CONTROLLER_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletShape.h"

#include "pandaNode.h"
#include "collideMask.h"
#include "luse.h"
#include "transformState.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletCharacterControllerNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletCharacterControllerNode : public PandaNode {

PUBLISHED:
  BulletCharacterControllerNode(BulletShape *shape, float step_height, const char *name="character");
  INLINE ~BulletCharacterControllerNode();

  void set_linear_velocity(const LVector3f &velocity, bool is_local);
  void set_angular_velocity(float omega);

  BulletShape *get_shape() const;

  float get_gravity() const;
  float get_max_slope() const;

  void set_fall_speed(float fall_speed);
  void set_jump_speed(float jump_speed);
  void set_max_jump_height(float max_jump_height);
  void set_max_slope(float max_slope);
  void set_gravity(float gravity);
  void set_use_ghost_sweep_test(bool value);

  bool is_on_ground() const;
  bool can_jump() const;
  void do_jump();

public:
  virtual CollideMask get_legal_collide_mask() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

  INLINE btPairCachingGhostObject *get_ghost() const;
  INLINE btKinematicCharacterController *get_character() const;

  void pre_step(float dt);
  void post_step();

protected:
  //virtual void parents_changed();
  //virtual void children_changed();
  virtual void transform_changed();

private:
  BulletUpAxis _up;

  btKinematicCharacterController *_character;
  btPairCachingGhostObject *_ghost;

  PT(BulletShape) _shape;

  LVector3f _linear_velocity;
  bool _linear_velocity_is_local;
  float _angular_velocity;

  bool _disable_transform_changed;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "BulletCharacterControllerNode", 
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

#include "bulletCharacterControllerNode.I"

#endif // __BULLET_CHARACTER_CONTROLLER_NODE_H__

