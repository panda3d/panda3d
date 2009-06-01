#define BUILD_DIRECTORY $[HAVE_PHYSX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET physx
  #define BUILD_TARGET $[HAVE_PHYSX]
  #define USE_PACKAGES physx
  #define LOCAL_LIBS pgraph linmath
  #define COMBINED_SOURCES $[TARGET]_composite.cxx
    #define INTERROGATE_OPTIONS $[INTERROGATE_OPTIONS] -DHAVE_PHYSX

  #define SOURCES \
    NoMinMax.h \
        config_physx.h \
    physxManager.I physxManager.h \
    physxContactHandler.I physxContactHandler.h \
    physxTriggerHandler.I physxTriggerHandler.h \
    physxJointHandler.I physxJointHandler.h \
    physxActorNode.I physxActorNode.h \
    physxActorDesc.I physxActorDesc.h \
    physxBodyDesc.I physxBodyDesc.h \
    physxBounds3.I physxBounds3.h \
    physxBox.I physxBox.h \
    physxJoint.I physxJoint.h \
    physxD6Joint.I physxD6Joint.h \
    physxJointDesc.I physxJointDesc.h \
    physxD6JointDesc.I physxD6JointDesc.h \
    physxJointDriveDesc.I physxJointDriveDesc.h \
    physxJointLimitSoftDesc.I physxJointLimitSoftDesc.h \
    physxJointLimitSoftPairDesc.I physxJointLimitSoftPairDesc.h \
    physxMaterial.I physxMaterial.h \
    physxMaterialDesc.I physxMaterialDesc.h \
    physxPlane.I physxPlane.h \
    physxRay.I physxRay.h \
    physxScene.I physxScene.h \
    physxSceneDesc.I physxSceneDesc.h \
    physxSceneStats2.I physxSceneStats2.h \
    physxSegment.I physxSegment.h \
    physxCapsule.I physxCapsule.h \
    physxShape.I physxShape.h \
    physxBoxShape.I physxBoxShape.h \
    physxCapsuleShape.I physxCapsuleShape.h \
    physxPlaneShape.I physxPlaneShape.h \
    physxSphereShape.I physxSphereShape.h \
    physxShapeDesc.I physxShapeDesc.h \
    physxBoxShapeDesc.I physxBoxShapeDesc.h \
    physxCapsuleShapeDesc.I physxCapsuleShapeDesc.h \
    physxPlaneShapeDesc.I physxPlaneShapeDesc.h \
    physxSphereShapeDesc.I physxSphereShapeDesc.h \
    physxSphere.I physxSphere.h \
    physxUtilLib.I physxUtilLib.h \

  #define INCLUDED_SOURCES \
    physxManager.cxx \
    physxContactHandler.cxx \
    physxTriggerHandler.cxx \
    physxJointHandler.cxx \
    physxActorNode.cxx \
    physxActorDesc.cxx \
    physxBodyDesc.cxx \
    physxBounds3.cxx \
    physxBox.cxx \
    physxJoint.cxx \
    physxD6Joint.cxx \
    physxJointDesc.cxx \
    physxD6JointDesc.cxx \
    physxJointDriveDesc.cxx \
    physxJointLimitSoftDesc.cxx \
    physxJointLimitSoftPairDesc.cxx \
    physxMaterial.cxx \
    physxMaterialDesc.cxx \
    physxPlane.cxx \
    physxRay.cxx \
    physxScene.cxx \
    physxSceneDesc.cxx \
    physxSceneStats2.cxx \
    physxSegment.cxx \
    physxCapsule.cxx \
    physxShape.cxx \
    physxBoxShape.cxx \
    physxCapsuleShape.cxx \
    physxPlaneShape.cxx \
    physxSphereShape.cxx \
    physxShapeDesc.cxx \
    physxBoxShapeDesc.cxx \
    physxCapsuleShapeDesc.cxx \
    physxPlaneShapeDesc.cxx \
    physxSphereShapeDesc.cxx \
    physxSphere.cxx \
    physxUtilLib.cxx \

  #define INSTALL_HEADERS \
    NoMinMax.h \
        config_physx.h \
    physxManager.I physxManager.h \
    physxContactHandler.I physxContactHandler.h \
    physxTriggerHandler.I physxTriggerHandler.h \
    physxJointHandler.I physxJointHandler.h \
    physxActorNode.I physxActorNode.h \
    physxActorDesc.I physxActorDesc.h \
    physxBodyDesc.I physxBodyDesc.h \
    physxBounds3.I physxBounds3.h \
    physxBox.I physxBox.h \
    physxJoint.I physxJoint.h \
    physxD6Joint.I physxD6Joint.h \
    physxJointDesc.I physxJointDesc.h \
    physxD6JointDesc.I physxD6JointDesc.h \
    physxJointDriveDesc.I physxJointDriveDesc.h \
    physxJointLimitSoftDesc.I physxJointLimitSoftDesc.h \
    physxJointLimitSoftPairDesc.I physxJointLimitSoftPairDesc.h \
    physxMaterial.I physxMaterial.h \
    physxMaterialDesc.I physxMaterialDesc.h \
    physxPlane.I physxPlane.h \
    physxRay.I physxRay.h \
    physxScene.I physxScene.h \
    physxSceneDesc.I physxSceneDesc.h \
    physxSceneStats2.I physxSceneStats2.h \
    physxSegment.I physxSegment.h \
    physxCapsule.I physxCapsule.h \
    physxShape.I physxShape.h \
    physxBoxShape.I physxBoxShape.h \
    physxCapsuleShape.I physxCapsuleShape.h \
    physxPlaneShape.I physxPlaneShape.h \
    physxSphereShape.I physxSphereShape.h \
    physxShapeDesc.I physxShapeDesc.h \
    physxBoxShapeDesc.I physxBoxShapeDesc.h \
    physxCapsuleShapeDesc.I physxCapsuleShapeDesc.h \
    physxPlaneShapeDesc.I physxPlaneShapeDesc.h \
    physxSphereShapeDesc.I physxSphereShapeDesc.h \
    physxSphere.I physxSphere.h \
    physxUtilLib.I physxUtilLib.h \

  #define IGATESCAN all

#end lib_target

