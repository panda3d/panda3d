set(P3COLLIDE_HEADERS
  collisionBox.I collisionBox.h
  collisionCapsule.I collisionCapsule.h
  collisionEntry.I collisionEntry.h
  collisionGeom.I collisionGeom.h
  collisionHandler.I collisionHandler.h
  collisionHandlerEvent.I collisionHandlerEvent.h
  collisionHandlerHighestEvent.h
  collisionHandlerFloor.I collisionHandlerFloor.h
  collisionHandlerGravity.I collisionHandlerGravity.h
  collisionHandlerPhysical.I collisionHandlerPhysical.h
  collisionHandlerPusher.I collisionHandlerPusher.h
  collisionHandlerFluidPusher.I collisionHandlerFluidPusher.h
  collisionHandlerQueue.h
  collisionHeightfield.I collisionHeightfield.h
  collisionInvSphere.I collisionInvSphere.h
  collisionLine.I collisionLine.h
  collisionLevelStateBase.I collisionLevelStateBase.h
  collisionLevelState.I collisionLevelState.h
  collisionNode.I collisionNode.h
  collisionParabola.I collisionParabola.h
  collisionPlane.I collisionPlane.h
  collisionPolygon.I collisionPolygon.h
  collisionFloorMesh.I collisionFloorMesh.h
  collisionRay.I collisionRay.h
  collisionRecorder.I collisionRecorder.h
  collisionSegment.I collisionSegment.h
  collisionSolid.I collisionSolid.h
  collisionSphere.I collisionSphere.h
  collisionTraverser.I collisionTraverser.h
  collisionTube.h
  collisionVisualizer.I collisionVisualizer.h
  config_collide.h
)

set(P3COLLIDE_SOURCES
  collisionBox.cxx
  collisionCapsule.cxx
  collisionEntry.cxx
  collisionGeom.cxx
  collisionHandler.cxx
  collisionHandlerEvent.cxx
  collisionHandlerHighestEvent.cxx
  collisionHandlerFloor.cxx
  collisionHandlerGravity.cxx
  collisionHandlerPhysical.cxx
  collisionHandlerPusher.cxx
  collisionHandlerFluidPusher.cxx
  collisionHandlerQueue.cxx
  collisionHeightfield.cxx
  collisionLevelStateBase.cxx
  collisionLevelState.cxx
  collisionInvSphere.cxx
  collisionLine.cxx
  collisionNode.cxx
  collisionParabola.cxx
  collisionPlane.cxx
  collisionPolygon.cxx
  collisionFloorMesh.cxx
  collisionRay.cxx
  collisionRecorder.cxx
  collisionSegment.cxx
  collisionSolid.cxx
  collisionSphere.cxx
  collisionTraverser.cxx
  collisionVisualizer.cxx
  config_collide.cxx
)

set(P3COLLIDE_IGATEEXT
  collisionHandlerEvent_ext.cxx
  collisionHandlerEvent_ext.h
  collisionHandlerPhysical_ext.cxx
  collisionHandlerPhysical_ext.h
  collisionHandlerQueue_ext.cxx
  collisionHandlerQueue_ext.h
  collisionNode_ext.cxx
  collisionNode_ext.h
  collisionPolygon_ext.cxx
  collisionPolygon_ext.h
  collisionTraverser_ext.cxx
  collisionTraverser_ext.h
)

composite_sources(p3collide P3COLLIDE_SOURCES)
add_component_library(p3collide SYMBOL BUILDING_PANDA_COLLIDE
  ${P3COLLIDE_HEADERS} ${P3COLLIDE_SOURCES})
target_link_libraries(p3collide p3tform)
target_interrogate(p3collide ALL EXTENSIONS ${P3COLLIDE_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3collide
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3COLLIDE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
