/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_physx.cxx
 * @author enn0x
 * @date 2009-09-01
 */

#include "config_physx.h"
#include "pandaSystem.h"

#include "physxActor.h"
#include "physxBoxController.h"
#include "physxBoxForceFieldShape.h"
#include "physxBoxShape.h"
#include "physxCapsuleController.h"
#include "physxCapsuleForceFieldShape.h"
#include "physxCapsuleShape.h"
#include "physxCcdSkeleton.h"
#include "physxCloth.h"
#include "physxClothMesh.h"
#include "physxClothNode.h"
#include "physxContactPair.h"
#include "physxContactPoint.h"
#include "physxController.h"
#include "physxControllerReport.h"
#include "physxControllerShapeHit.h"
#include "physxControllersHit.h"
#include "physxConvexMesh.h"
#include "physxConvexForceFieldShape.h"
#include "physxConvexShape.h"
#include "physxCylindricalJoint.h"
#include "physxD6Joint.h"
#include "physxDebugGeomNode.h"
#include "physxDistanceJoint.h"
#include "physxFixedJoint.h"
#include "physxForceField.h"
#include "physxForceFieldShape.h"
#include "physxForceFieldShapeGroup.h"
#include "physxHeightField.h"
#include "physxHeightFieldShape.h"
#include "physxJoint.h"
#include "physxMaterial.h"
#include "physxObject.h"
#include "physxPlaneShape.h"
#include "physxPointInPlaneJoint.h"
#include "physxPointOnLineJoint.h"
#include "physxPrismaticJoint.h"
#include "physxPulleyJoint.h"
#include "physxRevoluteJoint.h"
#include "physxScene.h"
#include "physxShape.h"
#include "physxSoftBody.h"
#include "physxSoftBodyMesh.h"
#include "physxSoftBodyNode.h"
#include "physxSphereForceFieldShape.h"
#include "physxSphereShape.h"
#include "physxSphericalJoint.h"
#include "physxTriangleMesh.h"
#include "physxTriangleMeshShape.h"
#include "physxVehicle.h"
#include "physxWheel.h"
#include "physxWheelShape.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAPHYSX)
  #error Buildsystem error: BUILDING_PANDAPHYSX not defined
#endif

ConfigureDef(config_physx);
NotifyCategoryDef(physx, "");

ConfigureFn(config_physx) {
  init_libphysx();
}

ConfigVariableBool physx_want_vrd
("physx-want-vrd", false,
PRC_DESC("Specified wether the manager should try to connect to the NVIDIA "
         "PhysX visual debugger or not. Connection is established when "
         "the first instance of PhysxManager is created."));

ConfigVariableString physx_vrd_host
("physx-vrd-host", "localhost",
PRC_DESC("Specified the host where the NVIDIA PhysX visual debugger is running "
         "on. Only used if the config-varibale 'physx-want-visual-debugger' "
         "is set to 'true'."));

ConfigVariableInt physx_vrd_port
("physx-visual-debugger-port", 5425,
PRC_DESC("Specified the port where the NVIDIA PhysX visual debugger is running "
         "on. Only used if the config-varibale 'physx-want-visual-debugger' "
         "is set to 'true'."));

ConfigVariableEnum<PhysxEnums::PhysxUpAxis> physx_up_axis
("physx-up-axis", PhysxEnums::Z_up,
PRC_DESC("Set the up direction for controllers and heightfields."));

ConfigVariableInt physx_internal_threads
("physx-internal-threads", 0,
PRC_DESC("Specified the number of internal threads to be created by the "
         "PhysX engine. The threads will be moved to different cores, if "
         "possible. Default value is '0'. PhysX then runs in an external "
         "thread, but no additional internal threads will be created."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libphysx() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  PhysxActor::init_type();
  PhysxBoxController::init_type();
  PhysxBoxForceFieldShape::init_type();
  PhysxBoxShape::init_type();
  PhysxCapsuleController::init_type();
  PhysxCapsuleForceFieldShape::init_type();
  PhysxCapsuleShape::init_type();
  PhysxCcdSkeleton::init_type();
  PhysxCloth::init_type();
  PhysxClothMesh::init_type();
  PhysxClothNode::init_type();
  PhysxContactPair::init_type();
  PhysxContactPoint::init_type();
  PhysxController::init_type();
  PhysxControllerShapeHit::init_type();
  PhysxControllersHit::init_type();
  PhysxConvexMesh::init_type();
  PhysxConvexForceFieldShape::init_type();
  PhysxConvexShape::init_type();
  PhysxCylindricalJoint::init_type();
  PhysxD6Joint::init_type();
  PhysxDebugGeomNode::init_type();
  PhysxDistanceJoint::init_type();
  PhysxFixedJoint::init_type();
  PhysxForceField::init_type();
  PhysxForceFieldShape::init_type();
  PhysxForceFieldShapeGroup::init_type();
  PhysxHeightField::init_type();
  PhysxHeightFieldShape::init_type();
  PhysxJoint::init_type();
  PhysxMaterial::init_type();
  PhysxObject::init_type();
  PhysxPlaneShape::init_type();
  PhysxPointInPlaneJoint::init_type();
  PhysxPointOnLineJoint::init_type();
  PhysxPrismaticJoint::init_type();
  PhysxPulleyJoint::init_type();
  PhysxRevoluteJoint::init_type();
  PhysxScene::init_type();
  PhysxShape::init_type();
  PhysxSoftBody::init_type();
  PhysxSoftBodyMesh::init_type();
  PhysxSoftBodyNode::init_type();
  PhysxSphereForceFieldShape::init_type();
  PhysxSphereShape::init_type();
  PhysxSphericalJoint::init_type();
  PhysxTriangleMesh::init_type();
  PhysxTriangleMeshShape::init_type();
  PhysxVehicle::init_type();
  PhysxWheel::init_type();
  PhysxWheelShape::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("PhysX");
}
