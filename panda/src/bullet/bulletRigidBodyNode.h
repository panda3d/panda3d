// Filename: bulletRigidBodyNode.h
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

#ifndef __BULLET_RIGID_BODY_NODE_H__
#define __BULLET_RIGID_BODY_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletBodyNode.h"

#include "pandaNode.h"
#include "collideMask.h"

class BulletShape;

////////////////////////////////////////////////////////////////////
//       Class : BulletRigidBodyNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletRigidBodyNode : public BulletBodyNode {

PUBLISHED:
  BulletRigidBodyNode(const char *name="rigid");
  INLINE ~BulletRigidBodyNode();

  // Mass
  LPoint3f get_center_of_mass_pos() const;
  float get_mass() const;
  void set_center_of_mass_pos(const LPoint3f &pos);
  void set_mass(float mass);

  // Velocity
  LVector3f get_linear_velocity() const;
  LVector3f get_angular_velocity() const;
  void set_linear_velocity(const LVector3f &velocity);
  void set_angular_velocity(const LVector3f &velocity);

  // Damping
  INLINE float get_linear_damping() const;
  INLINE float get_angular_damping() const;
  INLINE void set_linear_damping(float value);
  INLINE void set_angular_damping(float value);

  // Forces
  void clear_forces();
  void apply_force(const LVector3f &force, const LPoint3f &pos);
  void apply_central_force(const LVector3f &force);
  void apply_impulse(const LVector3f &impulse, const LPoint3f &pos);
  void apply_central_impulse(const LVector3f &impulse);
  void apply_torque(const LVector3f &torque);
  void apply_torque_impulse(const LVector3f &torque);

  // Deactivation thresholds
  float get_linear_sleep_threshold() const;
  float get_angular_sleep_threshold() const;
  void set_linear_sleep_threshold(float threshold);
  void set_angular_sleep_threshold(float threshold);

  // Gravity
  void set_gravity(const LVector3f &gravity);
  LVector3f get_gravity() const;

public:
  virtual btCollisionObject *get_object() const;

protected:
  virtual void parents_changed();
  virtual void transform_changed();

private:
  virtual void shape_changed();

  class MotionState : public btMotionState {

  public:
    MotionState(BulletRigidBodyNode *node) : _node(node) {};
    ~MotionState() {};

    virtual void getWorldTransform(btTransform &trans) const;
    virtual void setWorldTransform(const btTransform &trans);

  private:
    BulletRigidBodyNode *_node;
  };

  btRigidBody *_body;
  MotionState *_motion;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletBodyNode::init_type();
    register_type(_type_handle, "BulletRigidBodyNode", 
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

#include "bulletRigidBodyNode.I"

#endif // __BULLET_RIGID_BODY_NODE_H__

