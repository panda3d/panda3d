// Filename: NxPhysics.h
// Created by:  pratt (Apr 26, 2006)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2006, Disney Enterprises, Inc.  All rights reserved
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef NXPHYSICS_H
#define NXPHYSICS_H

#define NX_CALL_CONV


#include "NxFoundation.h"   //include the all of the foundation SDK 


//////////////general:

#include "NxScene.h"
#include "NxSceneDesc.h"

#include "NxActor.h"
#include "NxActorDesc.h"

#include "NxMaterial.h"
#include "NxMaterialDesc.h"

#include "NxContactStreamIterator.h"

#include "NxUserContactReport.h"
#include "NxUserNotify.h"
#include "NxUserRaycastReport.h"
#include "NxUserEntityReport.h"

#include "NxBodyDesc.h"

#include "NxEffector.h"

#include "NxSpringAndDamperEffector.h"
#include "NxSpringAndDamperEffectorDesc.h"

#include "NxScheduler.h"

//#if NX_USE_FLUID_API
//#include "fluids/NxFluid.h"
//#include "fluids/NxFluidDesc.h"
//#include "fluids/NxFluidEmitter.h"
//#include "fluids/NxFluidEmitterDesc.h"
//#endif
//
//#if NX_USE_CLOTH_API
//#include "cloth/NxCloth.h"
//#include "cloth/NxClothDesc.h"
//#endif

#include "NxCCDSkeleton.h"
#include "NxTriangle.h"
#include "NxScheduler.h"
#include "NxSceneStats.h"
#include "NxSceneStats2.h"
/////////////joints:

#include "NxJoint.h"

#include "NxJointLimitDesc.h"
#include "NxJointLimitPairDesc.h"
#include "NxMotorDesc.h"
#include "NxSpringDesc.h"

#include "NxPointInPlaneJoint.h"
#include "NxPointInPlaneJointDesc.h"

#include "NxPointOnLineJoint.h"
#include "NxPointOnLineJointDesc.h"

#include "NxRevoluteJoint.h"
#include "NxRevoluteJointDesc.h"

#include "NxPrismaticJoint.h"
#include "NxPrismaticJointDesc.h"

#include "NxCylindricalJoint.h"
#include "NxCylindricalJointDesc.h"

#include "NxSphericalJoint.h"
#include "NxSphericalJointDesc.h"

#include "NxFixedJoint.h"
#include "NxFixedJointDesc.h"

#include "NxDistanceJoint.h"
#include "NxDistanceJointDesc.h"

#include "NxPulleyJoint.h"
#include "NxPulleyJointDesc.h"

#include "NxD6Joint.h"
#include "NxD6JointDesc.h"

//////////////shapes:

#include "NxShape.h"
#include "NxShapeDesc.h"

#include "NxBoxShape.h"
#include "NxBoxShapeDesc.h"

#include "NxCapsuleShape.h"
#include "NxCapsuleShapeDesc.h"

#include "NxPlaneShape.h"
#include "NxPlaneShapeDesc.h"

#include "NxSphereShape.h"
#include "NxSphereShapeDesc.h"

#include "NxTriangleMesh.h"
#include "NxTriangleMeshDesc.h"

#include "NxTriangleMeshShape.h"
#include "NxTriangleMeshShapeDesc.h"

#include "NxConvexMesh.h"
#include "NxConvexMeshDesc.h"

#include "NxConvexShape.h"
#include "NxConvexShapeDesc.h"

#include "NxHeightField.h"
#include "NxHeightFieldDesc.h"

#include "NxHeightFieldShape.h"
#include "NxHeightFieldShapeDesc.h"
#include "NxHeightFieldSample.h"

#include "NxWheelShape.h"
#include "NxWheelShapeDesc.h"
//////////////utils:

#include "NxInertiaTensor.h"
#include "NxIntersectionBoxBox.h"
#include "NxIntersectionPointTriangle.h"
#include "NxIntersectionRayPlane.h"
#include "NxIntersectionRaySphere.h"
#include "NxIntersectionRayTriangle.h"
#include "NxIntersectionSegmentBox.h"
#include "NxIntersectionSegmentCapsule.h"
#include "NxIntersectionSweptSpheres.h"
#include "NxPMap.h"
#include "NxSmoothNormals.h"
#include "NxConvexHull.h"
#include "NxAllocateable.h"
#include "NxExportedUtils.h"

#include "PhysXLoader.h"


#endif  // NXPHYSICS_H
