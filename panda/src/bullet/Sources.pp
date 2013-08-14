#define BUILD_DIRECTORY $[HAVE_BULLET]
#define BUILDING_DLL BUILDING_PANDABULLET

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3bullet
  #define LOCAL_LIBS p3pgraph p3linmath p3grutil
  #define USE_PACKAGES bullet
  #define COMBINED_SOURCES $[TARGET]_composite.cxx

  #define SOURCES \
    bulletAllHitsRayResult.h bulletAllHitsRayResult.I \
    bulletBaseCharacterControllerNode.h bulletBaseCharacterControllerNode.I \
    bulletBodyNode.h bulletBodyNode.I \
    bulletBoxShape.h bulletBoxShape.I \
    bulletCapsuleShape.h bulletCapsuleShape.I \
    bulletCharacterControllerNode.h bulletCharacterControllerNode.I \
    bulletClosestHitRayResult.h bulletClosestHitRayResult.I \
    bulletClosestHitSweepResult.h bulletClosestHitSweepResult.I \
    bulletConeShape.h bulletConeShape.I \
    bulletConeTwistConstraint.h bulletConeTwistConstraint.I \
    bulletConstraint.h bulletConstraint.I \
    bulletContactCallbackData.h bulletContactCallbackData.I \
    bulletContactCallbacks.h \
    bulletContactResult.h bulletContactResult.I \
    bulletConvexHullShape.h bulletConvexHullShape.I \
    bulletConvexPointCloudShape.h bulletConvexPointCloudShape.I \
    bulletCylinderShape.h bulletCylinderShape.I \
    bulletDebugNode.h bulletDebugNode.I \
    bulletFilterCallbackData.h bulletFilterCallbackData.I \
    bulletGenericConstraint.h bulletGenericConstraint.I \
    bulletGhostNode.h bulletGhostNode.I \
    bulletHeightfieldShape.h bulletHeightfieldShape.I \
    bulletHelper.h bulletHelper.I \
    bulletHingeConstraint.h bulletHingeConstraint.I \
    bulletManifoldPoint.h bulletManifoldPoint.I \
    bulletMinkowskiSumShape.h bulletMinkowskiSumShape.I \
    bulletMultiSphereShape.h bulletMultiSphereShape.I \
    bulletPersistentManifold.h bulletPersistentManifold.I \
    bulletPlaneShape.h bulletPlaneShape.I \
    bulletRigidBodyNode.h bulletRigidBodyNode.I \
    bulletShape.h bulletShape.I \
    bulletSliderConstraint.h bulletSliderConstraint.I \
    bulletSoftBodyConfig.h bulletSoftBodyConfig.I \
    bulletSoftBodyControl.h bulletSoftBodyControl.I \
    bulletSoftBodyMaterial.h bulletSoftBodyMaterial.I \
    bulletSoftBodyNode.h bulletSoftBodyNode.I \
    bulletSoftBodyShape.h bulletSoftBodyShape.I \
    bulletSoftBodyWorldInfo.h bulletSoftBodyWorldInfo.I \
    bulletSphereShape.h bulletSphereShape.I \
    bulletSphericalConstraint.h bulletSphericalConstraint.I \
    bulletTickCallbackData.h bulletTickCallbackData.I \
    bulletTriangleMesh.h bulletTriangleMesh.I \
    bulletTriangleMeshShape.h bulletTriangleMeshShape.I \
    bulletVehicle.h bulletVehicle.I \
    bulletWheel.h bulletWheel.I \
    bulletWorld.h bulletWorld.I \
    bullet_includes.h \
    bullet_utils.h bullet_utils.I \
    config_bullet.h

  #define INCLUDED_SOURCES \
    bulletAllHitsRayResult.cxx \
    bulletBaseCharacterControllerNode.cxx \
    bulletBodyNode.cxx \
    bulletBoxShape.cxx \
    bulletCapsuleShape.cxx \
    bulletCharacterControllerNode.cxx \
    bulletClosestHitRayResult.cxx \
    bulletClosestHitSweepResult.cxx \
    bulletConeShape.cxx \
    bulletConeTwistConstraint.cxx \
    bulletConstraint.cxx \
    bulletContactCallbackData.cxx \
    bulletContactResult.cxx \
    bulletConvexHullShape.cxx \
    bulletConvexPointCloudShape.cxx \
    bulletCylinderShape.cxx \
    bulletDebugNode.cxx \
    bulletFilterCallbackData.cxx \
    bulletGenericConstraint.cxx \
    bulletGhostNode.cxx \
    bulletHeightfieldShape.cxx \
    bulletHelper.cxx \
    bulletHingeConstraint.cxx \
    bulletManifoldPoint.cxx \
    bulletMinkowskiSumShape.cxx \
    bulletMultiSphereShape.cxx \
    bulletPersistentManifold.cxx \
    bulletPlaneShape.cxx \
    bulletRigidBodyNode.cxx \
    bulletShape.cxx \
    bulletSliderConstraint.cxx \
    bulletSoftBodyConfig.cxx \
    bulletSoftBodyControl.cxx \
    bulletSoftBodyMaterial.cxx \
    bulletSoftBodyNode.cxx \
    bulletSoftBodyShape.cxx \
    bulletSoftBodyWorldInfo.cxx \
    bulletSphereShape.cxx \
    bulletSphericalConstraint.cxx \
    bulletTickCallbackData.cxx \
    bulletTriangleMesh.cxx \
    bulletTriangleMeshShape.cxx \
    bulletVehicle.cxx \
    bulletWheel.cxx \
    bulletWorld.cxx \
    bullet_utils.cxx \
    config_bullet.cxx

  #define INSTALL_HEADERS \
    bulletAllHitsRayResult.h bulletAllHitsRayResult.I \
    bulletBaseCharacterControllerNode.h bulletBaseCharacterControllerNode.I \
    bulletBodyNode.h bulletBodyNode.I \
    bulletBoxShape.h bulletBoxShape.I \
    bulletCapsuleShape.h bulletCapsuleShape.I \
    bulletCharacterControllerNode.h bulletCharacterControllerNode.I \
    bulletClosestHitRayResult.h bulletClosestHitRayResult.I \
    bulletClosestHitSweepResult.h bulletClosestHitSweepResult.I \
    bulletConeShape.h bulletConeShape.I \
    bulletConeTwistConstraint.h bulletConeTwistConstraint.I \
    bulletConstraint.h bulletConstraint.I \
    bulletContactCallbackData.h bulletContactCallbackData.I \
    bulletContactCallbacks.h \
    bulletContactResult.h bulletContactResult.I \
    bulletConvexHullShape.h bulletConvexHullShape.I \
    bulletConvexPointCloudShape.h bulletConvexPointCloudShape.I \
    bulletCylinderShape.h bulletCylinderShape.I \
    bulletDebugNode.h bulletDebugNode.I \
    bulletFilterCallbackData.h bulletFilterCallbackData.I \
    bulletGenericConstraint.h bulletGenericConstraint.I \
    bulletGhostNode.h bulletGhostNode.I \
    bulletHeightfieldShape.h bulletHeightfieldShape.I \
    bulletHelper.h bulletHelper.I \
    bulletHingeConstraint.h bulletHingeConstraint.I \
    bulletManifoldPoint.h bulletManifoldPoint.I \
    bulletMinkowskiSumShape.h bulletMinkowskiSumShape.I \
    bulletMultiSphereShape.h bulletMultiSphereShape.I \
    bulletPersistentManifold.h bulletPersistentManifold.I \
    bulletPlaneShape.h bulletPlaneShape.I \
    bulletRigidBodyNode.h bulletRigidBodyNode.I \
    bulletShape.h bulletShape.I \
    bulletSliderConstraint.h bulletSliderConstraint.I \
    bulletSoftBodyConfig.h bulletSoftBodyConfig.I \
    bulletSoftBodyControl.h bulletSoftBodyControl.I \
    bulletSoftBodyMaterial.h bulletSoftBodyMaterial.I \
    bulletSoftBodyNode.h bulletSoftBodyNode.I \
    bulletSoftBodyShape.h bulletSoftBodyShape.I \
    bulletSoftBodyWorldInfo.h bulletSoftBodyWorldInfo.I \
    bulletSphereShape.h bulletSphereShape.I \
    bulletSphericalConstraint.h bulletSphericalConstraint.I \
    bulletTickCallbackData.h bulletTickCallbackData.I \
    bulletTriangleMesh.h bulletTriangleMesh.I \
    bulletTriangleMeshShape.h bulletTriangleMeshShape.I \
    bulletVehicle.h bulletVehicle.I \
    bulletWheel.h bulletWheel.I \
    bulletWorld.h bulletWorld.I \
    bullet_includes.h \
    bullet_utils.h bullet_utils.I \
    config_bullet.h

  #define IGATESCAN all

#end lib_target
