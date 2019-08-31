if(NOT HAVE_ODE)
  return()
endif()

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
  odeHelperStructs.h
)

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
  odeCollisionEntry.cxx
)

set(P3ODE_IGATEEXT
  odeBody_ext.cxx
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
  odeUtil_ext.h
)

set(P3ODE_IGATE_SOURCES "${P3ODE_HEADERS};${P3ODE_SOURCES}")
list(REMOVE_ITEM P3ODE_IGATE_SOURCES "odeHelperStructs.h")

composite_sources(p3ode P3ODE_SOURCES)
add_library(p3ode ${P3ODE_SOURCES} ${P3ODE_HEADERS})
set_target_properties(p3ode PROPERTIES DEFINE_SYMBOL BUILDING_PANDAODE)
target_link_libraries(p3ode panda
  PKG::ODE)
target_interrogate(p3ode ${P3ODE_IGATE_SOURCES} EXTENSIONS ${P3ODE_IGATEEXT})

install(TARGETS p3ode
  EXPORT ODE COMPONENT ODE
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ODEDevel)
install(FILES ${P3ODE_HEADERS} COMPONENT ODEDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(ODE COMPONENT ODEDevel)
