// Filename: physicsCollisionHandler.cxx
// Created by:  drose (16Mar02)
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

#include "physicsCollisionHandler.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "config_collide.h"
#include "dcast.h"

TypeHandle PhysicsCollisionHandler::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysicsCollisionHandler::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PhysicsCollisionHandler::
PhysicsCollisionHandler() {
  _almost_stationary_speed = 0.1f;
  _static_friction_coef=0.9f;
  _dynamic_friction_coef=0.5f;
  set_horizontal(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysicsCollisionHandler::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PhysicsCollisionHandler::
~PhysicsCollisionHandler() {
}

////////////////////////////////////////////////////////////////////
//     Function: PhysicsCollisionHandler::apply_friction
//       Access: 
//  Description: The vel parameter will be modified in place to
//               account for friction.
////////////////////////////////////////////////////////////////////
void PhysicsCollisionHandler::
apply_friction(ColliderDef &def, LVector3f& vel, const LVector3f& force, float angle) {
  if (vel!=LVector3f::zero()) {
    float friction_coefficient=0.0f;
    // Determine the friction:
    if (fabs(force.dot(LVector3f::unit_z())<0.5f)) {
      // ...The force is nearly horizontal, it is probably a wall.
      physics_debug("  wall friction");
      friction_coefficient=0.0f;
    } else if (vel.length()<_almost_stationary_speed) {
      physics_debug("  static friction");
      friction_coefficient=_static_friction_coef;
    } else {
      physics_debug("  dynamic friction");
      friction_coefficient=_dynamic_friction_coef;
    }
    // Apply the friction:
    physics_debug("  vel pre  friction "<<vel<<" len "<<vel.length());
    float friction=friction_coefficient*angle;
    physics_debug("  friction "<<friction);
    if (friction<0.0f && friction>1.0f) {
      cerr<<"\n\nfriction error "<<friction<<endl;
      friction=1.0f;
    }
    #if 0
    float dt=ClockObject::get_global_clock()->get_dt();
    vel *= (1.0f-friction) * dt * dt;
    #else
    vel *= 1.0f-friction;
    #endif
    physics_debug("  vel post friction "<<vel<<" len "<<vel.length());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysicsCollisionHandler::apply_linear_force
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysicsCollisionHandler::
apply_linear_force(ColliderDef &def, const LVector3f &force) {
}
////////////////////////////////////////////////////////////////////
//     Function: PhysicsCollisionHandler::apply_net_shove
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysicsCollisionHandler::
apply_net_shove(ColliderDef &def, const LVector3f& net_shove, const LVector3f &force) {
  CollisionHandlerPusher::apply_net_shove(def, net_shove, force);
  if (force == LVector3f::zero()) {
    return;
  }
  if (def._target.is_empty()) {
    return;
  }
  ActorNode *actor;
  DCAST_INTO_V(actor, def._target.node());
  LVector3f vel=actor->get_physics_object()->get_velocity();
  if (vel == LVector3f::zero()) {
    return;
  }
  physics_debug("apply_linear_force() {");
  physics_debug("  vel            "<<vel<<" len "<<vel.length());
  physics_debug("  net_shove      "<<net_shove<<" len "<<net_shove.length());
  physics_debug("  force          "<<force<<" len "<<force.length());
  LVector3f old_vel=vel;

  // Copy the force vector while translating it 
  // into the physics object coordinate system:
  LVector3f adjustment=force;
  physics_debug("  adjustment set "<<adjustment<<" len "<<adjustment.length());

  //NodePath np(def._node);
  //CPT(TransformState) trans = np.get_net_transform();
  //adjustment=adjustment*trans->get_mat();
  //physics_debug("  adjustment trn "<<adjustment<<" len "<<adjustment.length());

  adjustment=adjustment*actor->get_physics_object()->get_lcs();
  physics_debug("  adjustment lcs "<<adjustment<<" len "<<adjustment.length());

  adjustment.normalize();
  physics_debug("  adjustment nrm "<<adjustment<<" len "<<adjustment.length());

  float adjustmentLength=-(adjustment.dot(vel));
  physics_debug("  adjustmentLength "<<adjustmentLength);
  float angle=-normalize(old_vel).dot(normalize(force));
  physics_debug("  angle "<<angle);
  // Are we in contact with something:
  if (angle>0.0f) {
    physics_debug("  positive contact");
    adjustment*=adjustmentLength;
    physics_debug("  adjustment mul "<<adjustment<<" len "<<adjustment.length());
    
    // This adjustment to our velocity will not reflect us off the surface,
    // but will deflect us parallel (or tangent) to the surface:
    if (normalize(net_shove).dot(LVector3f::unit_z())>0.5) {
        vel=LVector3f::zero();
    }
    //vel+=adjustment;
    physics_debug("  vel+adj "<<vel<<" len "<<vel.length());
    
    //apply_friction(def, vel, force, angle);
  } else if (adjustmentLength==0.0f) {
    physics_debug("  brushing contact");
  } else {
    physics_debug("  negative contact");
  }

  #ifndef NDEBUG //[
  if (IS_THRESHOLD_EQUAL(vel.length(), old_vel.length(), 0.0001f)) {
    // This is a check to see if vel is staying the same:
    physics_debug("  vel is about the same length:  "<<vel.length()<<" ~ "<<old_vel.length());
  } else if (vel.length() > old_vel.length()) {
    // This is a check to avoid adding engergy:
    physics_debug("  vel got larger  "<<vel.length()<<" > "<<old_vel.length());
  } else {
    // This is a check to avoid loosing engergy:
    physics_debug("  vel got smaller  "<<vel.length()<<" < "<<old_vel.length());
  }
  if (vel.length() > 10.0f) {
    // This is a check to see if the velocity is higher than I expect it
    // to go.  The check value is arbitrary.
    physics_debug("  vel.length() > 10.0f  "<<vel.length());
  }
  #endif //]

  physics_debug("  force "<<force<<" len "<<force.length());
  physics_debug("  vel "<<vel<<" len "<<vel.length());
  physics_debug("}");
  actor->set_contact_vector(adjustment);
  actor->get_physics_object()->set_velocity(vel);
}
