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
//     Function: PhysicsCollisionHandler::apply_linear_force
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysicsCollisionHandler::
apply_linear_force(ColliderDef &def, const LVector3f &force) {
  CollisionHandlerPusher::apply_linear_force(def, force);
  if (force == LVector3f::zero()) {
    return;
  }
  if (!def._node) {
    return;
  }
  ActorNode *actor=DCAST(ActorNode, def._node);
  float friction=0.9f;
  LVector3f vel=actor->get_physics_object()->get_velocity();
  physics_debug("apply_linear_force() {");
  physics_debug("  vel "<<vel<<" len "<<vel.length());
  physics_debug("  force "<<force<<" len "<<force.length());
  LVector3f old_vel=vel;
  LVector3f adjustment=force;
  adjustment.normalize();
  adjustment*=adjustment.dot(vel);
  physics_debug("  adjustment "<<adjustment<<" len "<<adjustment.length());
  float angle=adjustment.dot(vel);
  vel-=adjustment;
  physics_debug("  vel "<<vel<<" len "<<vel.length());
  physics_debug("  angle "<<angle);
  if (angle<=0.0f) {
    // ...avoid amplifying the velocity by checking to see
    // that the adjustment and the velocity are more than 
    // right-angles (i.e. obtuse angle).
    float almostStationary=0.1f;
    if (vel.length()>almostStationary) {
      physics_debug("  vel > almostStationary");
      friction*=0.01f;
    }
    //vel*=1.0f-friction;
  }

  #ifndef NDEBUG //[
  if (vel.length() > old_vel.length()) {
    physics_debug("  vel.length() > old_vel.length()  "<<vel.length()<<" > "<<old_vel.length());
  }
  if (vel.length() > 10.0f) {
    physics_debug("  vel.length() > 10.0f  "<<vel.length());
  }
  #endif //]

  physics_debug("  force "<<force<<" len "<<force.length());
  physics_debug("  vel "<<vel<<" len "<<vel.length());
  physics_debug("}");
  actor->set_contact_vector(force);
  actor->get_physics_object()->set_velocity(vel);
}
