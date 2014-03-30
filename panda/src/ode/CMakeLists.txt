if(HAVE_ODE)
  set(P3ODE_HEADERS
      ode_includes.h config_ode.h
      odeWorld.I odeWorld.h
      odeMass.I odeMass.h
      odeBody.I odeBody.h
      odeJointGroup.I odeJointGroup.h
      odeJoint.I odeJoint.h
      odeUtil.h
      odeSpace.I odeSpace.h
      odeGeom.I odeGeom.h
      odeSurfaceParameters.I odeSurfaceParameters.h
      odeContactGeom.I odeContactGeom.h
      odeContact.I odeContact.h
      odeAMotorJoint.I odeAMotorJoint.h
      odeBallJoint.I odeBallJoint.h
      odeContactJoint.I odeContactJoint.h
      odeFixedJoint.I odeFixedJoint.h
      odeHingeJoint.I odeHingeJoint.h
      odeHinge2Joint.I odeHinge2Joint.h
      odeLMotorJoint.I odeLMotorJoint.h
      odeNullJoint.I odeNullJoint.h
      odePlane2dJoint.I odePlane2dJoint.h
      odeSliderJoint.I odeSliderJoint.h
      odeUniversalJoint.I odeUniversalJoint.h
      odeJointCollection.I odeJointCollection.h
      odeSimpleSpace.I odeSimpleSpace.h
      odeHashSpace.I odeHashSpace.h
      odeQuadTreeSpace.I odeQuadTreeSpace.h
      odeSphereGeom.I odeSphereGeom.h
      odeBoxGeom.I odeBoxGeom.h
      odePlaneGeom.I odePlaneGeom.h
      odeCappedCylinderGeom.I odeCappedCylinderGeom.h
      odeCylinderGeom.I odeCylinderGeom.h
      odeRayGeom.I odeRayGeom.h
      odeTriMeshData.I odeTriMeshData.h
      odeTriMeshGeom.I odeTriMeshGeom.h
      odeCollisionEntry.I odeCollisionEntry.h
      odeHelperStructs.h)

  set(P3ODE_SOURCES
      config_ode.cxx
      odeWorld.cxx odeMass.cxx odeBody.cxx
      odeJointGroup.cxx odeJoint.cxx
      odeUtil.cxx
      odeSpace.cxx
      odeGeom.cxx
      odeSurfaceParameters.cxx
      odeContactGeom.cxx odeContact.cxx
      odeAMotorJoint.cxx odeBallJoint.cxx
      odeContactJoint.cxx odeFixedJoint.cxx
      odeHingeJoint.cxx odeHinge2Joint.cxx
      odeLMotorJoint.cxx odeNullJoint.cxx
      odePlane2dJoint.cxx odeSliderJoint.cxx
      odeUniversalJoint.cxx odeJointCollection.cxx
      odeSimpleSpace.cxx
      odeHashSpace.cxx odeQuadTreeSpace.cxx
      odeSphereGeom.cxx odeBoxGeom.cxx
      odePlaneGeom.cxx odeCappedCylinderGeom.cxx
      odeCylinderGeom.cxx odeRayGeom.cxx
      odeTriMeshData.cxx  odeTriMeshGeom.cxx
      odeCollisionEntry.cxx)

  set(P3ODE_IGATEEXT
      odeBody_ext.h
      odeBody_ext.I
      odeGeom_ext.cxx
      odeGeom_ext.h
      odeGeom_ext.I
      odeJoint_ext.cxx
      odeJoint_ext.h
      odeSpace_ext.cxx
      odeSpace_ext.h
      odeSpace_ext.I
      odeUtil_ext.cxx
      odeUtil_ext.h)

  set(P3ODE_IGATE_SOURCES "${P3ODE_HEADERS};${P3ODE_SOURCES}")
  list(REMOVE_ITEM P3ODE_IGATE_SOURCES "odeConvexGeom.h")
  list(REMOVE_ITEM P3ODE_IGATE_SOURCES "odeHeightFieldGeom.h")
  list(REMOVE_ITEM P3ODE_IGATE_SOURCES "odeHelperStructs.h")

  composite_sources(p3ode P3ODE_SOURCES)
  add_library(p3ode ${P3ODE_SOURCES} ${P3ODE_HEADERS} ${P3ODE_IGATEEXT})
  set_target_properties(p3ode PROPERTIES COMPILE_DEFINITIONS dSINGLE)
  target_link_libraries(p3ode p3pgraph p3physics ${ODE_LIBRARY})
  target_interrogate(p3ode ${P3ODE_IGATE_SOURCES} ${P3ODE_IGATEEXT})

  install(TARGETS p3ode DESTINATION lib)
endif()
