if(NOT HAVE_BULLET)
  return()
endif()

set(P3BULLET_HEADERS
  config_bullet.h
  bullet_includes.h
  bullet_utils.I bullet_utils.h
  bulletAllHitsRayResult.I bulletAllHitsRayResult.h
  bulletBaseCharacterControllerNode.I bulletBaseCharacterControllerNode.h
  bulletBodyNode.I bulletBodyNode.h
  bulletBoxShape.I bulletBoxShape.h
  bulletCapsuleShape.I bulletCapsuleShape.h
  bulletCharacterControllerNode.I bulletCharacterControllerNode.h
  bulletClosestHitRayResult.I bulletClosestHitRayResult.h
  bulletClosestHitSweepResult.I bulletClosestHitSweepResult.h
  bulletConeShape.I bulletConeShape.h
  bulletConeTwistConstraint.I bulletConeTwistConstraint.h
  bulletConstraint.I bulletConstraint.h
  bulletContactCallbackData.I bulletContactCallbackData.h
  bulletContactCallbacks.h
  bulletContactResult.I bulletContactResult.h
  bulletConvexHullShape.I bulletConvexHullShape.h
  bulletConvexPointCloudShape.I bulletConvexPointCloudShape.h
  bulletCylinderShape.I bulletCylinderShape.h
  bulletDebugNode.I bulletDebugNode.h
  bulletFilterCallbackData.I bulletFilterCallbackData.h
  bulletGenericConstraint.I bulletGenericConstraint.h
  bulletGhostNode.I bulletGhostNode.h
  bulletHeightfieldShape.I bulletHeightfieldShape.h
  bulletHelper.I bulletHelper.h
  bulletHingeConstraint.I bulletHingeConstraint.h
  bulletManifoldPoint.I bulletManifoldPoint.h
  bulletMinkowskiSumShape.I bulletMinkowskiSumShape.h
  bulletMultiSphereShape.I bulletMultiSphereShape.h
  bulletPersistentManifold.I bulletPersistentManifold.h
  bulletPlaneShape.I bulletPlaneShape.h
  bulletRigidBodyNode.I bulletRigidBodyNode.h
  bulletRotationalLimitMotor.I bulletRotationalLimitMotor.h
  bulletShape.I bulletShape.h
  bulletSliderConstraint.I bulletSliderConstraint.h
  bulletSoftBodyConfig.I bulletSoftBodyConfig.h
  bulletSoftBodyControl.I bulletSoftBodyControl.h
  bulletSoftBodyMaterial.I bulletSoftBodyMaterial.h
  bulletSoftBodyNode.I bulletSoftBodyNode.h
  bulletSoftBodyShape.I bulletSoftBodyShape.h
  bulletSoftBodyWorldInfo.I bulletSoftBodyWorldInfo.h
  bulletSphereShape.I bulletSphereShape.h
  bulletSphericalConstraint.I bulletSphericalConstraint.h
  bulletTickCallbackData.I bulletTickCallbackData.h
  bulletTranslationalLimitMotor.I bulletTranslationalLimitMotor.h
  bulletTriangleMesh.I bulletTriangleMesh.h
  bulletTriangleMeshShape.I bulletTriangleMeshShape.h
  bulletVehicle.I bulletVehicle.h
  bulletWheel.I bulletWheel.h
  bulletWorld.I bulletWorld.h
)

set(P3BULLET_SOURCES
  config_bullet.cxx
  bullet_utils.cxx
  bulletAllHitsRayResult.cxx
  bulletBaseCharacterControllerNode.cxx
  bulletBodyNode.cxx
  bulletBoxShape.cxx
  bulletCapsuleShape.cxx
  bulletCharacterControllerNode.cxx
  bulletClosestHitRayResult.cxx
  bulletClosestHitSweepResult.cxx
  bulletConeShape.cxx
  bulletConeTwistConstraint.cxx
  bulletConstraint.cxx
  bulletContactCallbackData.cxx
  bulletContactResult.cxx
  bulletConvexHullShape.cxx
  bulletConvexPointCloudShape.cxx
  bulletCylinderShape.cxx
  bulletDebugNode.cxx
  bulletFilterCallbackData.cxx
  bulletGenericConstraint.cxx
  bulletGhostNode.cxx
  bulletHeightfieldShape.cxx
  bulletHelper.cxx
  bulletHingeConstraint.cxx
  bulletManifoldPoint.cxx
  bulletMinkowskiSumShape.cxx
  bulletMultiSphereShape.cxx
  bulletPersistentManifold.cxx
  bulletPlaneShape.cxx
  bulletRigidBodyNode.cxx
  bulletRotationalLimitMotor.cxx
  bulletShape.cxx
  bulletSliderConstraint.cxx
  bulletSoftBodyConfig.cxx
  bulletSoftBodyControl.cxx
  bulletSoftBodyMaterial.cxx
  bulletSoftBodyNode.cxx
  bulletSoftBodyShape.cxx
  bulletSoftBodyWorldInfo.cxx
  bulletSphereShape.cxx
  bulletSphericalConstraint.cxx
  bulletTickCallbackData.cxx
  bulletTranslationalLimitMotor.cxx
  bulletTriangleMesh.cxx
  bulletTriangleMeshShape.cxx
  bulletVehicle.cxx
  bulletWheel.cxx
  bulletWorld.cxx
)

composite_sources(p3bullet P3BULLET_SOURCES)
add_library(p3bullet ${P3BULLET_SOURCES} ${P3BULLET_HEADERS})
set_target_properties(p3bullet PROPERTIES DEFINE_SYMBOL BUILDING_PANDABULLET)
target_link_libraries(p3bullet panda PKG::BULLET)
target_interrogate(p3bullet ALL)

if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Clang emulating MSVC; it has builtin defines for __m128 + __m128 and co.
  target_compile_definitions(p3bullet PUBLIC BT_NO_SIMD_OPERATOR_OVERLOADS)
endif()

install(TARGETS p3bullet
  EXPORT Bullet COMPONENT Bullet
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT BulletDevel)
install(FILES ${P3BULLET_HEADERS} COMPONENT BulletDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(Bullet COMPONENT BulletDevel)
