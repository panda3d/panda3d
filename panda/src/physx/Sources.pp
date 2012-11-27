#define BUILD_DIRECTORY $[HAVE_PHYSX]
#define BUILDING_DLL BUILDING_PANDAPHYSX

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3physx
  #define LOCAL_LIBS p3pgraph p3linmath p3grutil
  #define USE_PACKAGES physx
  #define COMBINED_SOURCES $[TARGET]_composite.cxx

  #define SOURCES \
    config_physx.h physx_includes.h \
    physxActor.I physxActor.h \
    physxActorDesc.I physxActorDesc.h \
    physxBodyDesc.I physxBodyDesc.h \
    physxBounds3.I physxBounds3.h \
    physxBox.I physxBox.h \
    physxBoxController.I physxBoxController.h \
    physxBoxControllerDesc.I physxBoxControllerDesc.h \
    physxBoxForceFieldShape.I physxBoxForceFieldShape.h \
    physxBoxForceFieldShapeDesc.I physxBoxForceFieldShapeDesc.h \
    physxBoxShape.I physxBoxShape.h \
    physxBoxShapeDesc.I physxBoxShapeDesc.h \
    physxCapsule.I physxCapsule.h \
    physxCapsuleController.I physxCapsuleController.h \
    physxCapsuleControllerDesc.I physxCapsuleControllerDesc.h \
    physxCapsuleForceFieldShape.I physxCapsuleForceFieldShape.h \
    physxCapsuleForceFieldShapeDesc.I physxCapsuleForceFieldShapeDesc.h \
    physxCapsuleShape.I physxCapsuleShape.h \
    physxCapsuleShapeDesc.I physxCapsuleShapeDesc.h \
    physxCcdSkeleton.I physxCcdSkeleton.h \
    physxCcdSkeletonDesc.I physxCcdSkeletonDesc.h \
    physxCloth.I physxCloth.h \
    physxClothDesc.I physxClothDesc.h \
    physxClothMesh.I physxClothMesh.h \
    physxClothMeshDesc.I physxClothMeshDesc.h \
    physxClothNode.I physxClothNode.h \
    physxConstraintDominance.I physxConstraintDominance.h \
    physxContactPair.I physxContactPair.h \
    physxContactPoint.I physxContactPoint.h \
    physxContactReport.I physxContactReport.h \
    physxController.I physxController.h \
    physxControllerDesc.I physxControllerDesc.h \
    physxControllerReport.I physxControllerReport.h \
    physxControllerShapeHit.I physxControllerShapeHit.h \
    physxControllersHit.I physxControllersHit.h \
    physxConvexForceFieldShape.I physxConvexForceFieldShape.h \
    physxConvexForceFieldShapeDesc.I physxConvexForceFieldShapeDesc.h \
    physxConvexMesh.I physxConvexMesh.h \
    physxConvexMeshDesc.I physxConvexMeshDesc.h \
    physxConvexShape.I physxConvexShape.h \
    physxConvexShapeDesc.I physxConvexShapeDesc.h \
    physxCylindricalJoint.I physxCylindricalJoint.h \
    physxCylindricalJointDesc.I physxCylindricalJointDesc.h \
    physxD6Joint.I physxD6Joint.h \
    physxD6JointDesc.I physxD6JointDesc.h \
    physxDebugGeomNode.I physxDebugGeomNode.h \
    physxDistanceJoint.I physxDistanceJoint.h \
    physxDistanceJointDesc.I physxDistanceJointDesc.h \
    physxEnums.h \
    physxFileStream.h \
    physxFixedJoint.I physxFixedJoint.h \
    physxFixedJointDesc.I physxFixedJointDesc.h \
    physxForceField.I physxForceField.h \
    physxForceFieldDesc.I physxForceFieldDesc.h \
    physxForceFieldShape.I physxForceFieldShape.h \
    physxForceFieldShapeDesc.I physxForceFieldShapeDesc.h \
    physxForceFieldShapeGroup.I physxForceFieldShapeGroup.h \
    physxForceFieldShapeGroupDesc.I physxForceFieldShapeGroupDesc.h \
    physxGroupsMask.I physxGroupsMask.h \
    physxHeightField.I physxHeightField.h \
    physxHeightFieldDesc.I physxHeightFieldDesc.h \
    physxHeightFieldShape.I physxHeightFieldShape.h \
    physxHeightFieldShapeDesc.I physxHeightFieldShapeDesc.h \
    physxJoint.I physxJoint.h \
    physxJointDesc.I physxJointDesc.h \
    physxJointDriveDesc.I physxJointDriveDesc.h \
    physxJointLimitDesc.I physxJointLimitDesc.h \
    physxJointLimitSoftDesc.I physxJointLimitSoftDesc.h \
    physxKitchen.I physxKitchen.h \
    physxLinearInterpolationValues.I physxLinearInterpolationValues.h \
    physxManager.I physxManager.h \
    physxMask.I physxMask.h \
    physxMaterial.I physxMaterial.h \
    physxMaterialDesc.I physxMaterialDesc.h \
    physxMemoryReadBuffer.h \
    physxMemoryWriteBuffer.h \
    physxMeshPool.I physxMeshPool.h \
    physxMeshHash.I physxMeshHash.h \
    physxMotorDesc.I physxMotorDesc.h \
    physxObject.I physxObject.h \
    physxObjectCollection.I physxObjectCollection.h \
    physxOverlapReport.I physxOverlapReport.h \
    physxPlane.I physxPlane.h \
    physxPlaneShape.I physxPlaneShape.h \
    physxPlaneShapeDesc.I physxPlaneShapeDesc.h \
    physxPointInPlaneJoint.I physxPointInPlaneJoint.h \
    physxPointInPlaneJointDesc.I physxPointInPlaneJointDesc.h \
    physxPointOnLineJoint.I physxPointOnLineJoint.h \
    physxPointOnLineJointDesc.I physxPointOnLineJointDesc.h \
    physxPrismaticJoint.I physxPrismaticJoint.h \
    physxPrismaticJointDesc.I physxPrismaticJointDesc.h \
    physxPulleyJoint.I physxPulleyJoint.h \
    physxPulleyJointDesc.I physxPulleyJointDesc.h \
    physxRay.I physxRay.h \
    physxRaycastHit.I physxRaycastHit.h \
    physxRaycastReport.I physxRaycastReport.h \
    physxRevoluteJoint.I physxRevoluteJoint.h \
    physxRevoluteJointDesc.I physxRevoluteJointDesc.h \
    physxScene.I physxScene.h \
    physxSceneDesc.I physxSceneDesc.h \
    physxSceneStats2.I physxSceneStats2.h \
    physxSegment.I physxSegment.h \
    physxShape.I physxShape.h \
    physxShapeDesc.I physxShapeDesc.h \
    physxSoftBody.I physxSoftBody.h \
    physxSoftBodyDesc.I physxSoftBodyDesc.h \
    physxSoftBodyMesh.I physxSoftBodyMesh.h \
    physxSoftBodyMeshDesc.I physxSoftBodyMeshDesc.h \
    physxSoftBodyNode.I physxSoftBodyNode.h \
    physxSphere.I physxSphere.h \
    physxSphereForceFieldShape.I physxSphereForceFieldShape.h \
    physxSphereForceFieldShapeDesc.I physxSphereForceFieldShapeDesc.h \
    physxSphereShape.I physxSphereShape.h \
    physxSphereShapeDesc.I physxSphereShapeDesc.h \
    physxSphericalJoint.I physxSphericalJoint.h \
    physxSphericalJointDesc.I physxSphericalJointDesc.h \
    physxSpringDesc.I physxSpringDesc.h \
    physxTriangleMesh.I physxTriangleMesh.h \
    physxTriangleMeshDesc.I physxTriangleMeshDesc.h \
    physxTriangleMeshShape.I physxTriangleMeshShape.h \
    physxTriangleMeshShapeDesc.I physxTriangleMeshShapeDesc.h \
    physxTriggerReport.I physxTriggerReport.h \
    physxUtilLib.I physxUtilLib.h \
    physxVehicle.I physxVehicle.h \
    physxVehicleDesc.I physxVehicleDesc.h \
    physxWheel.I physxWheel.h \
    physxWheelDesc.I physxWheelDesc.h \
    physxWheelShape.I physxWheelShape.h \
    physxWheelShapeDesc.I physxWheelShapeDesc.h \

  #define INCLUDED_SOURCES \
    config_physx.cxx \
    physxActor.cxx \
    physxActorDesc.cxx \
    physxBodyDesc.cxx \
    physxBounds3.cxx \
    physxBox.cxx \
    physxBoxController.cxx \
    physxBoxControllerDesc.cxx \
    physxBoxForceFieldShape.cxx \
    physxBoxForceFieldShapeDesc.cxx \
    physxBoxShape.cxx \
    physxBoxShapeDesc.cxx \
    physxCapsule.cxx \
    physxCapsuleController.cxx \
    physxCapsuleControllerDesc.cxx \
    physxCapsuleForceFieldShape.cxx \
    physxCapsuleForceFieldShapeDesc.cxx \
    physxCapsuleShape.cxx \
    physxCapsuleShapeDesc.cxx \
    physxCcdSkeleton.cxx \
    physxCcdSkeletonDesc.cxx \
    physxCloth.cxx \
    physxClothDesc.cxx \
    physxClothMesh.cxx \
    physxClothMeshDesc.cxx \
    physxClothNode.cxx \
    physxConstraintDominance.cxx \
    physxContactPair.cxx \
    physxContactPoint.cxx \
    physxContactReport.cxx \
    physxController.cxx \
    physxControllerDesc.cxx \
    physxControllerReport.cxx \
    physxControllerShapeHit.cxx \
    physxControllersHit.cxx \
    physxConvexForceFieldShape.cxx \
    physxConvexForceFieldShapeDesc.cxx \
    physxConvexMesh.cxx \
    physxConvexMeshDesc.cxx \
    physxConvexShape.cxx \
    physxConvexShapeDesc.cxx \
    physxCylindricalJoint.cxx \
    physxCylindricalJointDesc.cxx \
    physxD6Joint.cxx \
    physxD6JointDesc.cxx \
    physxDebugGeomNode.cxx \
    physxDistanceJoint.cxx \
    physxDistanceJointDesc.cxx \
    physxEnums.cxx \
    physxFileStream.cxx \
    physxFixedJoint.cxx \
    physxFixedJointDesc.cxx \
    physxForceField.cxx \
    physxForceFieldDesc.cxx \
    physxForceFieldShape.cxx \
    physxForceFieldShapeDesc.cxx \
    physxForceFieldShapeGroup.cxx \
    physxForceFieldShapeGroupDesc.cxx \
    physxGroupsMask.cxx \
    physxHeightField.cxx \
    physxHeightFieldDesc.cxx \
    physxHeightFieldShape.cxx \
    physxHeightFieldShapeDesc.cxx \
    physxJoint.cxx \
    physxJointDesc.cxx \
    physxJointDriveDesc.cxx \
    physxJointLimitDesc.cxx \
    physxJointLimitSoftDesc.cxx \
    physxKitchen.cxx \
    physxLinearInterpolationValues.cxx \
    physxManager.cxx \
    physxMask.cxx \
    physxMaterial.cxx \
    physxMaterialDesc.cxx \
    physxMemoryReadBuffer.cxx \
    physxMemoryWriteBuffer.cxx \
    physxMeshPool.cxx \
    physxMeshHash.cxx \
    physxMotorDesc.cxx \
    physxObject.cxx \
    physxObjectCollection.cxx \
    physxOverlapReport.cxx \
    physxPlane.cxx \
    physxPlaneShape.cxx \
    physxPlaneShapeDesc.cxx \
    physxPointInPlaneJoint.cxx \
    physxPointInPlaneJointDesc.cxx \
    physxPointOnLineJoint.cxx \
    physxPointOnLineJointDesc.cxx \
    physxPrismaticJoint.cxx \
    physxPrismaticJointDesc.cxx \
    physxPulleyJoint.cxx \
    physxPulleyJointDesc.cxx \
    physxRay.cxx \
    physxRaycastHit.cxx \
    physxRaycastReport.cxx \
    physxRevoluteJoint.cxx \
    physxRevoluteJointDesc.cxx \
    physxScene.cxx \
    physxSceneDesc.cxx \
    physxSceneStats2.cxx \
    physxSegment.cxx \
    physxShape.cxx \
    physxShapeDesc.cxx \
    physxSoftBody.cxx \
    physxSoftBodyDesc.cxx \
    physxSoftBodyMesh.cxx \
    physxSoftBodyMeshDesc.cxx \
    physxSoftBodyNode.cxx \
    physxSphere.cxx \
    physxSphereForceFieldShape.cxx \
    physxSphereForceFieldShapeDesc.cxx \
    physxSphereShape.cxx \
    physxSphereShapeDesc.cxx \
    physxSphericalJoint.cxx \
    physxSphericalJointDesc.cxx \
    physxSpringDesc.cxx \
    physxTriangleMesh.cxx \
    physxTriangleMeshDesc.cxx \
    physxTriangleMeshShape.cxx \
    physxTriangleMeshShapeDesc.cxx \
    physxTriggerReport.cxx \
    physxUtilLib.cxx \
    physxVehicle.cxx \
    physxVehicleDesc.cxx \
    physxWheel.cxx \
    physxWheelDesc.cxx \
    physxWheelShape.cxx \
    physxWheelShapeDesc.cxx \

  #define INSTALL_HEADERS \
    config_physx.h physx_includes.h \
    physxActor.I physxActor.h \
    physxActorDesc.I physxActorDesc.h \
    physxBodyDesc.I physxBodyDesc.h \
    physxBounds3.I physxBounds3.h \
    physxBox.I physxBox.h \
    physxBoxController.I physxBoxController.h \
    physxBoxControllerDesc.I physxBoxControllerDesc.h \
    physxBoxForceFieldShape.I physxBoxForceFieldShape.h \
    physxBoxForceFieldShapeDesc.I physxBoxForceFieldShapeDesc.h \
    physxBoxShape.I physxBoxShape.h \
    physxBoxShapeDesc.I physxBoxShapeDesc.h \
    physxCapsule.I physxCapsule.h \
    physxCapsuleController.I physxCapsuleController.h \
    physxCapsuleControllerDesc.I physxCapsuleControllerDesc.h \
    physxCapsuleForceFieldShape.I physxCapsuleForceFieldShape.h \
    physxCapsuleForceFieldShapeDesc.I physxCapsuleForceFieldShapeDesc.h \
    physxCapsuleShape.I physxCapsuleShape.h \
    physxCapsuleShapeDesc.I physxCapsuleShapeDesc.h \
    physxCcdSkeleton.I physxCcdSkeleton.h \
    physxCcdSkeletonDesc.I physxCcdSkeletonDesc.h \
    physxCloth.I physxCloth.h \
    physxClothDesc.I physxClothDesc.h \
    physxClothMesh.I physxClothMesh.h \
    physxClothMeshDesc.I physxClothMeshDesc.h \
    physxClothNode.I physxClothNode.h \
    physxConstraintDominance.I physxConstraintDominance.h \
    physxContactPair.I physxContactPair.h \
    physxContactPoint.I physxContactPoint.h \
    physxContactReport.I physxContactReport.h \
    physxController.I physxController.h \
    physxControllerDesc.I physxControllerDesc.h \
    physxControllerReport.I physxControllerReport.h \
    physxControllerShapeHit.I physxControllerShapeHit.h \
    physxControllersHit.I physxControllersHit.h \
    physxConvexForceFieldShape.I physxConvexForceFieldShape.h \
    physxConvexForceFieldShapeDesc.I physxConvexForceFieldShapeDesc.h \
    physxConvexMesh.I physxConvexMesh.h \
    physxConvexMeshDesc.I physxConvexMeshDesc.h \
    physxConvexShape.I physxConvexShape.h \
    physxConvexShapeDesc.I physxConvexShapeDesc.h \
    physxCylindricalJoint.I physxCylindricalJoint.h \
    physxCylindricalJointDesc.I physxCylindricalJointDesc.h \
    physxD6Joint.I physxD6Joint.h \
    physxD6JointDesc.I physxD6JointDesc.h \
    physxDebugGeomNode.I physxDebugGeomNode.h \
    physxDistanceJoint.I physxDistanceJoint.h \
    physxDistanceJointDesc.I physxDistanceJointDesc.h \
    physxEnums.h \
    physxFileStream.h \
    physxFixedJoint.I physxFixedJoint.h \
    physxFixedJointDesc.I physxFixedJointDesc.h \
    physxForceField.I physxForceField.h \
    physxForceFieldDesc.I physxForceFieldDesc.h \
    physxForceFieldShape.I physxForceFieldShape.h \
    physxForceFieldShapeDesc.I physxForceFieldShapeDesc.h \
    physxForceFieldShapeGroup.I physxForceFieldShapeGroup.h \
    physxForceFieldShapeGroupDesc.I physxForceFieldShapeGroupDesc.h \
    physxGroupsMask.I physxGroupsMask.h \
    physxHeightField.I physxHeightField.h \
    physxHeightFieldDesc.I physxHeightFieldDesc.h \
    physxHeightFieldShape.I physxHeightFieldShape.h \
    physxHeightFieldShapeDesc.I physxHeightFieldShapeDesc.h \
    physxJoint.I physxJoint.h \
    physxJointDesc.I physxJointDesc.h \
    physxJointDriveDesc.I physxJointDriveDesc.h \
    physxJointLimitDesc.I physxJointLimitDesc.h \
    physxJointLimitSoftDesc.I physxJointLimitSoftDesc.h \
    physxKitchen.I physxKitchen.h \
    physxLinearInterpolationValues.I physxLinearInterpolationValues.h \
    physxManager.I physxManager.h \
    physxMask.I physxMask.h \
    physxMaterial.I physxMaterial.h \
    physxMaterialDesc.I physxMaterialDesc.h \
    physxMemoryReadBuffer.h \
    physxMemoryWriteBuffer.h \
    physxMeshPool.I physxMeshPool.h \
    physxMeshHash.I physxMeshHash.h \
    physxMotorDesc.I physxMotorDesc.h \
    physxObject.I physxObject.h \
    physxObjectCollection.I physxObjectCollection.h \
    physxOverlapReport.I physxOverlapReport.h \
    physxPlane.I physxPlane.h \
    physxPlaneShape.I physxPlaneShape.h \
    physxPlaneShapeDesc.I physxPlaneShapeDesc.h \
    physxPointInPlaneJoint.I physxPointInPlaneJoint.h \
    physxPointInPlaneJointDesc.I physxPointInPlaneJointDesc.h \
    physxPointOnLineJoint.I physxPointOnLineJoint.h \
    physxPointOnLineJointDesc.I physxPointOnLineJointDesc.h \
    physxPrismaticJoint.I physxPrismaticJoint.h \
    physxPrismaticJointDesc.I physxPrismaticJointDesc.h \
    physxPulleyJoint.I physxPulleyJoint.h \
    physxPulleyJointDesc.I physxPulleyJointDesc.h \
    physxRay.I physxRay.h \
    physxRaycastHit.I physxRaycastHit.h \
    physxRaycastReport.I physxRaycastReport.h \
    physxRevoluteJoint.I physxRevoluteJoint.h \
    physxRevoluteJointDesc.I physxRevoluteJointDesc.h \
    physxScene.I physxScene.h \
    physxSceneDesc.I physxSceneDesc.h \
    physxSceneStats2.I physxSceneStats2.h \
    physxSegment.I physxSegment.h \
    physxShape.I physxShape.h \
    physxShapeDesc.I physxShapeDesc.h \
    physxSoftBody.I physxSoftBody.h \
    physxSoftBodyDesc.I physxSoftBodyDesc.h \
    physxSoftBodyMesh.I physxSoftBodyMesh.h \
    physxSoftBodyMeshDesc.I physxSoftBodyMeshDesc.h \
    physxSoftBodyNode.I physxSoftBodyNode.h \
    physxSphere.I physxSphere.h \
    physxSphereForceFieldShape.I physxSphereForceFieldShape.h \
    physxSphereForceFieldShapeDesc.I physxSphereForceFieldShapeDesc.h \
    physxSphereShape.I physxSphereShape.h \
    physxSphereShapeDesc.I physxSphereShapeDesc.h \
    physxSphericalJoint.I physxSphericalJoint.h \
    physxSphericalJointDesc.I physxSphericalJointDesc.h \
    physxSpringDesc.I physxSpringDesc.h \
    physxTriangleMesh.I physxTriangleMesh.h \
    physxTriangleMeshDesc.I physxTriangleMeshDesc.h \
    physxTriangleMeshShape.I physxTriangleMeshShape.h \
    physxTriangleMeshShapeDesc.I physxTriangleMeshShapeDesc.h \
    physxTriggerReport.I physxTriggerReport.h \
    physxUtilLib.I physxUtilLib.h \
    physxVehicle.I physxVehicle.h \
    physxVehicleDesc.I physxVehicleDesc.h \
    physxWheel.I physxWheel.h \
    physxWheelDesc.I physxWheelDesc.h \
    physxWheelShape.I physxWheelShape.h \
    physxWheelShapeDesc.I physxWheelShapeDesc.h \

  #define IGATESCAN all

#end lib_target
