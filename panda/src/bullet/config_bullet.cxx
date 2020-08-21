/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_bullet.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "config_bullet.h"

#include "bulletBaseCharacterControllerNode.h"
#include "bulletBodyNode.h"
#include "bulletBoxShape.h"
#include "bulletCapsuleShape.h"
#include "bulletCharacterControllerNode.h"
#include "bulletConeShape.h"
#include "bulletConeTwistConstraint.h"
#include "bulletContactCallbackData.h"
#include "bulletConstraint.h"
#include "bulletConvexHullShape.h"
#include "bulletConvexPointCloudShape.h"
#include "bulletCylinderShape.h"
#include "bulletMinkowskiSumShape.h"
#include "bulletDebugNode.h"
#include "bulletFilterCallbackData.h"
#include "bulletGenericConstraint.h"
#include "bulletGhostNode.h"
#include "bulletHeightfieldShape.h"
#include "bulletHingeConstraint.h"
#include "bulletMultiSphereShape.h"
#include "bulletPlaneShape.h"
#include "bulletRigidBodyNode.h"
#include "bulletShape.h"
#include "bulletSliderConstraint.h"
#include "bulletSphereShape.h"
#include "bulletSphericalConstraint.h"
#include "bulletSoftBodyNode.h"
#include "bulletSoftBodyShape.h"
#include "bulletTickCallbackData.h"
#include "bulletTriangleMesh.h"
#include "bulletTriangleMeshShape.h"
#include "bulletVehicle.h"
#include "bulletWorld.h"

#include "bulletContactCallbacks.h"

extern ContactAddedCallback gContactAddedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactDestroyedCallback gContactDestroyedCallback;

#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDABULLET)
  #error Buildsystem error: BUILDING_PANDABULLET not defined
#endif

Configure(config_bullet);
NotifyCategoryDef(bullet, "");

ConfigureFn(config_bullet) {
  init_libbullet();
}

ConfigVariableInt bullet_max_objects
("bullet-max-objects", 1024,
PRC_DESC("Specifies the maximum number of individual objects within a "
         "bullet physics world. Default value is 1024."));

ConfigVariableInt bullet_gc_lifetime
("bullet-gc-lifetime", 256,
PRC_DESC("Specifies the lifetime of data clean up be the soft body world "
         "info garbage collector. Default value is 256."));

ConfigVariableEnum<BulletWorld::BroadphaseAlgorithm> bullet_broadphase_algorithm
("bullet-broadphase-algorithm", BulletWorld::BA_dynamic_aabb_tree,
PRC_DESC("Specifies the broadphase algorithm to be used by the physics "
         "engine. Default value is 'aabb' (dynamic aabb tree)."));

ConfigVariableEnum<BulletWorld::FilterAlgorithm> bullet_filter_algorithm
("bullet-filter-algorithm", BulletWorld::FA_mask,
PRC_DESC("Specifies the algorithm to be used by the physics engine for "
         "collision filtering. Default value is 'mask'."));

ConfigVariableDouble bullet_sap_extents
("bullet-sap-extents", 1000.0,
PRC_DESC("Specifies the world extent in all directions. The config variable "
         "is only used if bullet-broadphase-algorithm is set to 'sap' "
         "(sweep and prune). Default value is 1000.0."));

ConfigVariableBool bullet_enable_contact_events
("bullet-enable-contact-events", false,
PRC_DESC("Specifies if events should be send when new contacts are "
         "created or existing contacts get remove. Warning: enabling "
         "contact events might create more load on the event queue "
         "then you might want! Default value is FALSE."));

ConfigVariableInt bullet_solver_iterations
("bullet-solver-iterations", 10,
PRC_DESC("Specifies the number of iterations for the Bullet contact "
         "solver. This is the native Bullet property "
         "btContactSolverInfo::m_numIterations. Default value is 10."));

ConfigVariableBool bullet_additional_damping
("bullet-additional-damping", false,
PRC_DESC("Enables additional damping on eachrigid body, in order to reduce "
         "jitter. Default value is FALSE. Additional damping is an "
         "experimental feature of the Bullet physics engine. Use with "
         "care."));

ConfigVariableDouble bullet_additional_damping_linear_factor
("bullet-additional-damping-linear-factor", 0.005,
PRC_DESC("Only used when bullet-additional-damping is set to TRUE. "
         "Default value is 0.005"));

ConfigVariableDouble bullet_additional_damping_angular_factor
("bullet-additional-damping-angular-factor", 0.01,
PRC_DESC("Only used when bullet-additional-damping is set to TRUE. "
         "Default value is 0.01"));

ConfigVariableDouble bullet_additional_damping_linear_threshold
("bullet-additional-damping-linear-threshold", 0.01,
PRC_DESC("Only used when bullet-additional-damping is set to TRUE. "
         "Default value is 0.01"));

ConfigVariableDouble bullet_additional_damping_angular_threshold
("bullet-additional-damping-angular-threshold", 0.01,
PRC_DESC("Only used when bullet-additional-damping is set to TRUE. "
         "Default value is 0.01."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libbullet() {

  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  // Initialize types
  BulletBaseCharacterControllerNode::init_type();
  BulletBodyNode::init_type();
  BulletBoxShape::init_type();
  BulletCapsuleShape::init_type();
  BulletCharacterControllerNode::init_type();
  BulletConeShape::init_type();
  BulletConeTwistConstraint::init_type();
  BulletContactCallbackData::init_type();
  BulletConstraint::init_type();
  BulletConvexHullShape::init_type();
  BulletConvexPointCloudShape::init_type();
  BulletCylinderShape::init_type();
  BulletMinkowskiSumShape::init_type();
  BulletDebugNode::init_type();
  BulletFilterCallbackData::init_type();
  BulletGenericConstraint::init_type();
  BulletGhostNode::init_type();
  BulletHeightfieldShape::init_type();
  BulletHingeConstraint::init_type();
  BulletMultiSphereShape::init_type();
  BulletPlaneShape::init_type();
  BulletRigidBodyNode::init_type();
  BulletShape::init_type();
  BulletSliderConstraint::init_type();
  BulletSphereShape::init_type();
  BulletSphericalConstraint::init_type();
  BulletSoftBodyNode::init_type();
  BulletSoftBodyShape::init_type();
  BulletTickCallbackData::init_type();
  BulletTriangleMesh::init_type();
  BulletTriangleMeshShape::init_type();
  BulletVehicle::init_type();
  BulletWorld::init_type();

  // Register factory functions for constructing objects from .bam files
  BulletBoxShape::register_with_read_factory();
  BulletConvexHullShape::register_with_read_factory();
  BulletDebugNode::register_with_read_factory();
  BulletPlaneShape::register_with_read_factory();
  BulletRigidBodyNode::register_with_read_factory();
  BulletSphereShape::register_with_read_factory();
  BulletTriangleMesh::register_with_read_factory();
  BulletTriangleMeshShape::register_with_read_factory();
  BulletCylinderShape::register_with_read_factory();
  BulletCapsuleShape::register_with_read_factory();
  BulletConeShape::register_with_read_factory();
  BulletHeightfieldShape::register_with_read_factory();
  BulletConvexPointCloudShape::register_with_read_factory();
  BulletMinkowskiSumShape::register_with_read_factory();
  BulletMultiSphereShape::register_with_read_factory();

  // Custom contact callbacks
  gContactAddedCallback = contact_added_callback;
  gContactProcessedCallback = contact_processed_callback;
  gContactDestroyedCallback = contact_destroyed_callback;

  // Initialize notification category
  bullet_cat.init();
  if (bullet_cat.is_debug()) {
    bullet_cat.debug() << "initialize module" << std::endl;
  }

  // Register the Bullet system
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Bullet");
}
