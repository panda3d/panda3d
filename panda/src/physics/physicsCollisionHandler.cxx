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
  LVector3f vel=actor->get_physics_object()->get_velocity();
  physics_debug("apply_linear_force() {");
  physics_debug("  vel "<<vel<<" len "<<vel.length());
  physics_debug("  force "<<force<<" len "<<force.length());
  LVector3f old_vel=vel;

  // Copy the force vector while translating it 
  // into the physics object coordinate system:
  LVector3f adjustment=force;
  physics_debug("  adjustment set "<<adjustment<<" len "<<adjustment.length());

  NodePath np(def._node);
  CPT(TransformState) trans = np.get_net_transform();
  adjustment=adjustment*trans->get_mat();
  physics_debug("  adjustment trn "<<adjustment<<" len "<<adjustment.length());

  adjustment=adjustment*actor->get_physics_object()->get_lcs();
  physics_debug("  adjustment lcs "<<adjustment<<" len "<<adjustment.length());

  adjustment.normalize();
  physics_debug("  adjustment nrm "<<adjustment<<" len "<<adjustment.length());

  float adjustmentLength=-(adjustment.dot(vel));
  physics_debug("  adjustmentLength "<<adjustmentLength);
  // Are we in contact with something:
  if (adjustmentLength>0.0f) {
    physics_debug("  positive contact");
    adjustment*=adjustmentLength;
    physics_debug("  adjustment mul "<<adjustment<<" len "<<adjustment.length());
    // This adjustment to our velocity will not reflect us off the surface,
    // but will deflect us parallel (or tangent) to the surface:
    vel+=adjustment;
    physics_debug("  vel "<<vel<<" len "<<vel.length());
    physics_debug("  angle "<<adjustmentLength);
    const float almostStationary=0.1f;
    float friction=0.9f;
    // This velocity is not very accurate, but it's better than using 
    // the pre-collision-tested velocity for now.  This may need to be 
    // better:
    LVector3f implicitVel=actor->get_physics_object()->get_implicit_velocity();
    physics_debug("  implicitVel "<<implicitVel<<" len "<<implicitVel.length());
    // Determine the friction:
    if (implicitVel.length()<almostStationary) {
      physics_debug("  static friction");
      friction=0.9f;
    } else {
      physics_debug("  dynamic friction");
      friction=0.1f;
    }
    // Apply the friction:
    vel*=1.0f-friction;
  } else if (adjustmentLength==0.0f) {
    physics_debug("  brushing contact");
  } else {
    physics_debug("  negative contact");
  }

  #ifndef NDEBUG //[
  if (vel.length() > old_vel.length()) {
    // This is a check to avoid adding engergy:
    physics_debug("  vel.length() > old_vel.length()  "<<vel.length()<<" > "<<old_vel.length());
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
