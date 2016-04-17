/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file NxPhysics.h
 * @author enn0x
 * @date 2009-09-02
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef NXPHYSICS_H
#define NXPHYSICS_H

class NxActor;
class NxActorDesc;
class NxBodyDesc;
class NxBoxForceFieldShape;
class NxBoxForceFieldShapeDesc;
class NxBoxShape;
class NxBoxShapeDesc;
class NxCapsuleForceFieldShape;
class NxCapsuleForceFieldShapeDesc;
class NxCapsuleShape;
class NxCapsuleShapeDesc;
class NxCCDSkeleton;
class NxContactStreamIterator;
class NxConvexMesh;
class NxConvexMeshDesc;
class NxConvexForceFieldShape;
class NxConvexForceFieldShapeDesc;
class NxConvexShape;
class NxConvexShapeDesc;
class NxForceField;
class NxForceFieldDesc;
class NxForceFieldLinearKernelDesc;
class NxForceFieldLinearKernel;
class NxForceFieldShape;
class NxForceFieldShapeDesc;
class NxForceFieldShapeGroup;
class NxForceFieldShapeGroupDesc;
class NxHeightFieldShape;
class NxHeightFieldShapeDesc;
class NxHeightField;
class NxHeightFieldDesc;
class NxHeightFieldSample;
class NxMaterial;
class NxMaterialDesc;
class NxPlaneShape;
class NxPlaneShapeDesc;
class NxPhysicsSDK;
class NxRay;
class NxRaycastHit;
class NxScene;
class NxSceneDesc;
class NxShape;
class NxShapeDesc;
class NxSphereForceFieldShape;
class NxSphereForceFieldShapeDesc;
class NxSphereShape;
class NxSphereShapeDesc;
class NxTriangleMesh;
class NxTriangleMeshDesc;
class NxTriangleMeshShape;
class NxTriangleMeshShapeDesc;
class NxJointDesc;
class NxCylindricalJointDesc;
class NxD6JointDesc;
class NxDistanceJointDesc;
class NxFixedJointDesc;
class NxPointInPlaneJointDesc;
class NxPointOnLineJointDesc;
class NxPrismaticJointDesc;
class NxPulleyJointDesc;
class NxRevoluteJointDesc;
class NxSphericalJointDesc;
class NxJoint;
class NxCylindricalJoint;
class NxD6Joint;
class NxDistanceJoint;
class NxFixedJoint;
class NxPointInPlaneJoint;
class NxPointOnLineJoint;
class NxPrismaticJoint;
class NxPulleyJoint;
class NxRevoluteJoint;
class NxSphericalJoint;
class NxMotorDesc;
class NxSpringDesc;
class NxJointLimitDesc;
class NxJointLimitPairDesc;
class NxJointLimitSoftDesc;
class NxJointLimitSoftPairDesc;
class NxJointDriveDesc;
class NxUserRaycastReport;
class NxBounds3;
class NxWheelShape;
class NxWheelShapeDesc;
class NxContactPair;
class NxUserContactReport;
class NxCloth;
class NxClothDesc;
class NxClothMesh;
class NxClothMeshDesc;
class NxMeshData;
class NxConstraintDominance;
class NxRemoteDebugger;
class NxGroupsMask;
class NxSceneStats2;
class NxSimpleTriangleMesh;

class NxSoftBody;
class NxSoftBodyDesc;
class NxSoftBodyMesh;
class NxSoftBodyMeshDesc;

enum NxAssertResponse {
  NX_AR_CONTINUE,
  NX_AR_IGNORE,
  NX_AR_BREAKPOINT
};

enum NxControllerType {
  NX_CONTROLLER_CAPSULE,
  NX_CONTROLLER_FORCE_DWORD = 0x7fffffff
};

template<class T> class NxUserEntityReport;

class NxU8;
class NxU16;
class NxU32;
class NxI32;
class NxReal;
class NxF32;
class NxVec3;
class NxMat33;
class NxMat34;
class NxQuat;
class NxMaterialIndex;

class NxBox;
class NxSphere;
class NxSegment;
class NxCapsule;
class NxPlane;
class NxUtilLib;

#endif  // NXPHYSICS_H
