// Filename: config_collide.cxx
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "config_collide.h"
#include "collisionNode.h"
#include "collisionSolid.h"
#include "collisionSphere.h"
#include "collisionPlane.h"
#include "collisionPolygon.h"
#include "collisionRay.h"
#include "collisionEntry.h"
#include "collisionHandler.h"
#include "collisionHandlerEvent.h"
#include "collisionHandlerPhysical.h"
#include "collisionHandlerPusher.h"
#include "collisionHandlerFloor.h"
#include "collisionHandlerQueue.h"

#include <dconfig.h>

Configure(config_collide);
NotifyCategoryDef(collide, "");

ConfigureFn(config_collide) {
  CollisionNode::init_type();
  CollisionSolid::init_type();
  CollisionSphere::init_type();
  CollisionPlane::init_type();
  CollisionPolygon::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  CollisionNode::register_with_read_factory();
  CollisionPlane::register_with_read_factory();
  CollisionPolygon::register_with_read_factory();
  CollisionSphere::register_with_read_factory();
  CollisionRay::init_type();
  CollisionEntry::init_type();
  CollisionHandler::init_type();
  CollisionHandlerEvent::init_type();
  CollisionHandlerPhysical::init_type();
  CollisionHandlerPusher::init_type();
  CollisionHandlerFloor::init_type();
  CollisionHandlerQueue::init_type();
}

