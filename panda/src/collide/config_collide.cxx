/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_collide.cxx
 * @author drose
 * @date 2000-04-24
 */

#include "config_collide.h"
#include "collisionBox.h"
#include "collisionCapsule.h"
#include "collisionEntry.h"
#include "collisionHandler.h"
#include "collisionHandlerEvent.h"
#include "collisionHandlerHighestEvent.h"
#include "collisionHandlerFloor.h"
#include "collisionHandlerGravity.h"
#include "collisionHandlerPhysical.h"
#include "collisionHandlerPusher.h"
#include "collisionHandlerFluidPusher.h"
#include "collisionHandlerQueue.h"
#include "collisionInvSphere.h"
#include "collisionLine.h"
#include "collisionLevelStateBase.h"
#include "collisionGeom.h"
#include "collisionNode.h"
#include "collisionParabola.h"
#include "collisionPlane.h"
#include "collisionPolygon.h"
#include "collisionFloorMesh.h"
#include "collisionRay.h"
#include "collisionRecorder.h"
#include "collisionSegment.h"
#include "collisionSolid.h"
#include "collisionSphere.h"
#include "collisionTraverser.h"
#include "collisionVisualizer.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_COLLIDE)
  #error Buildsystem error: BUILDING_PANDA_COLLIDE not defined
#endif

Configure(config_collide);
NotifyCategoryDef(collide, "");

ConfigureFn(config_collide) {
  init_libcollide();
}

ConfigVariableBool respect_prev_transform
("respect-prev-transform", false,
 PRC_DESC("Set this true to have all CollisionTraversers in the world respect "
          "the previous frame's transform (position) for a given object when "
          "determining motion for collision tests.  If this is false, you must "
          "explicitly enable motion detection for a particular traverser.  It "
          "is false by default to force programmers to decide on a "
          "case-by-case basis whether they really need this feature."));

ConfigVariableBool respect_effective_normal
("respect-effective-normal", true,
 PRC_DESC("This should be true to support the effective_normal interface of "
          "polygons.  Set it false to disable this feature, so that all "
          "collision solids (including polygons and planes) use their actual "
          "normal for intersection and physics tests."));

ConfigVariableBool allow_collider_multiple
("allow-collider-multiple", false,
 PRC_DESC("Set this true to enable the use of a DoubleBitMask or QuadBitMask "
          "to manage many "
          "colliders added to a single traverser in one pass.  If this is "
          "false, a one-word BitMask is always used instead, which is faster "
          "per pass, but may require more passes."));

ConfigVariableBool flatten_collision_nodes
("flatten-collision-nodes", false,
 PRC_DESC("Set this true to allow NodePath::flatten_medium() and "
          "flatten_strong() to combine multiple CollisionNodes "
          "into a single CollisionNode--but only if they share the "
          "same name and collide masks.  When false, CollisionNodes "
          "are never combined.  This is false by default, since "
          "collision tests rely heavily on bounding volume tests "
          "to be efficient, and combining CollisionNodes is likely "
          "to merge bounding volumes inappropriately."));

ConfigVariableDouble collision_parabola_bounds_threshold
("collision-parabola-bounds-threshold", 10.0,
 PRC_DESC("This is the threshold size for a CollisionParabola to "
          "make a bounding box (BoundingHexahedron).  If the parabola "
          "is smaller than this, it will make a BoundingSphere instead, "
          "which is much easier to make and will be good enough for "
          "small parabolas."));

ConfigVariableInt collision_parabola_bounds_sample
("collision-parabola-bounds-sample", 10,
 PRC_DESC("This is the number of points along a CollisionParabola to "
          "sample in order to determine an accurate bounding box."));

ConfigVariableInt fluid_cap_amount
("fluid-cap-amount", 100,
 PRC_DESC("ensures that fluid pos doesn't check beyond X feet"));

ConfigVariableBool pushers_horizontal
("pushers-horizontal", false,
 PRC_DESC("Set this true to make all CollisionHandlerPushers have the "
          "set_horizontal() flag by default, false to let the move "
          "in three dimensions by default."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libcollide() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CollisionBox::init_type();
  CollisionCapsule::init_type();
  CollisionEntry::init_type();
  CollisionHandler::init_type();
  CollisionHandlerEvent::init_type();
  CollisionHandlerHighestEvent::init_type();
  CollisionHandlerFloor::init_type();
  CollisionHandlerGravity::init_type();
  CollisionHandlerPhysical::init_type();
  CollisionHandlerPusher::init_type();
  CollisionHandlerFluidPusher::init_type();
  CollisionHandlerQueue::init_type();
  CollisionInvSphere::init_type();
  CollisionLine::init_type();
  CollisionLevelStateBase::init_type();
  CollisionGeom::init_type();
  CollisionNode::init_type();
  CollisionParabola::init_type();
  CollisionPlane::init_type();
  CollisionPolygon::init_type();
  CollisionFloorMesh::init_type();
  CollisionRay::init_type();
  CollisionSegment::init_type();
  CollisionSolid::init_type();
  CollisionSphere::init_type();
  CollisionTraverser::init_type();

#ifdef DO_COLLISION_RECORDING
  CollisionRecorder::init_type();
  CollisionVisualizer::init_type();
#endif

  // Record the old name for CollisionCapsule for backwards compatibility.
  BamWriter::record_obsolete_type_name(CollisionCapsule::get_class_type(),
                                       "CollisionTube", 6, 44);

  CollisionBox::register_with_read_factory();
  CollisionCapsule::register_with_read_factory();
  CollisionInvSphere::register_with_read_factory();
  CollisionLine::register_with_read_factory();
  CollisionNode::register_with_read_factory();
  CollisionParabola::register_with_read_factory();
  CollisionPlane::register_with_read_factory();
  CollisionPolygon::register_with_read_factory();
  CollisionFloorMesh::register_with_read_factory();
  CollisionRay::register_with_read_factory();
  CollisionSegment::register_with_read_factory();
  CollisionSphere::register_with_read_factory();
}
