// Filename: config_collide.cxx
// Created by:  drose (24Apr00)
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

#include "config_collide.h"
#include "collisionEntry.h"
#include "collisionHandler.h"
#include "collisionHandlerEvent.h"
#include "collisionHandlerFloor.h"
#include "collisionHandlerPhysical.h"
#include "collisionHandlerPusher.h"
#include "collisionHandlerQueue.h"
#include "collisionNode.h"
#include "collisionPlane.h"
#include "collisionPolygon.h"
#include "collisionRay.h"
#include "collisionRecorder.h"
#include "collisionSegment.h"
#include "collisionSolid.h"
#include "collisionSphere.h"
#include "collisionTube.h"
#include "collisionVisualizer.h"
#include "dconfig.h"

Configure(config_collide);
NotifyCategoryDef(collide, "");

ConfigureFn(config_collide) {
  init_libcollide();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libcollide
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libcollide() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CollisionEntry::init_type();
  CollisionHandler::init_type();
  CollisionHandlerEvent::init_type();
  CollisionHandlerFloor::init_type();
  CollisionHandlerPhysical::init_type();
  CollisionHandlerPusher::init_type();
  CollisionHandlerQueue::init_type();
  CollisionNode::init_type();
  CollisionPlane::init_type();
  CollisionPolygon::init_type();
  CollisionRay::init_type();
  CollisionSegment::init_type();
  CollisionSolid::init_type();
  CollisionSphere::init_type();
  CollisionTube::init_type();

#ifdef DO_COLLISION_RECORDING
  CollisionRecorder::init_type();
  CollisionVisualizer::init_type();
#endif

  // Registration of writeable object's creation
  // functions with BamReader's factory
  CollisionNode::register_with_read_factory();
  CollisionPlane::register_with_read_factory();
  CollisionPolygon::register_with_read_factory();
  CollisionRay::register_with_read_factory();
  CollisionSegment::register_with_read_factory();
  CollisionSphere::register_with_read_factory();
  CollisionTube::register_with_read_factory();
}
