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
  LVector3f old_vel=vel;
  LVector3f adjustment=force;
  adjustment.normalize();
  adjustment*=adjustment.dot(vel);
  #if 0 //[
  float initialVelMag=vel.length();
  float temp=((vel-c)*friction).length();
  if ((vel-c)[2]) {
    cerr<<"\n\napply_linear_force"
        <<"\n  old_vel "<<old_vel<<" len "<<old_vel.length()
        <<"\n  force "<<force<<" len "<<force.length()
        <<"\n  adjustment "<<adjustment<<" len"<<adjustment.length()
        <<"\n  vel "<<vel-c<<" len "<<(vel-c).length()
        <<"\n  friction "<<friction
        <<"\n  vel "<<((vel-c)*friction)<<" len "<<temp
        <<"\n  initialVelLen > "<<(initialVelMag>temp)  
        <<endl;
    if (initialVelMag<temp) {
      cerr<<"\n*************************************"<<endl;
    }
  }
  #endif //]
  float angle=adjustment.dot(vel);
  vel-=adjustment;
  if (angle<=0.0f) {
    // ...avoid amplifying the velocity by checking to see
    // that the adjustment and the velocity are more than 
    // right-angles (i.e. obtuse angle).
    float almostStationary=1.0f;
    if (vel.dot(vel)>almostStationary) {
      friction*=0.01f; cerr<<"not almostStationary"<<endl;
    }
    //vel*=1.0f-friction;
  }

  LVector3f new_vel=vel;
  if (vel.length() > old_vel.length()) {
    cerr<<"\nvel.length() > old_vel.length()  "<<vel.length()<<" > "<<old_vel.length()<<endl;
  }
  if (vel.length() > 10.0f) {
    cerr<<"\nvel.length() > 10.0f  "<<vel.length()<<endl;
  }

  actor->set_contact_vector(force);
  actor->get_physics_object()->set_velocity(vel);
}
