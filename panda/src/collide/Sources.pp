#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    tform gobj pgraph putil \
    pstatclient
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    collisionEntry.I collisionEntry.h \
    collisionGeom.I collisionGeom.h \
    collisionHandler.I collisionHandler.h  \
    collisionHandlerEvent.I collisionHandlerEvent.h  \
    collisionHandlerFloor.I collisionHandlerFloor.h  \
    collisionHandlerGravity.I collisionHandlerGravity.h  \
    collisionHandlerPhysical.I collisionHandlerPhysical.h  \
    collisionHandlerPusher.I collisionHandlerPusher.h  \
    collisionHandlerFluidPusher.I collisionHandlerFluidPusher.h  \
    collisionHandlerQueue.h \
    collisionDSSolid.I collisionDSSolid.h \
    collisionInvSphere.I collisionInvSphere.h \
    collisionLine.I collisionLine.h \
    collisionLevelStateBase.I collisionLevelStateBase.h \
    collisionLevelState.I collisionLevelState.h \
    collisionNode.I collisionNode.h \
    collisionParabola.I collisionParabola.h  \
    collisionPlane.I collisionPlane.h  \
    collisionPolygon.I collisionPolygon.h \
    collisionFloorMesh.I collisionFloorMesh.h \
    collisionRay.I collisionRay.h \
    collisionRecorder.I collisionRecorder.h \
    collisionSegment.I collisionSegment.h  \
    collisionSolid.I collisionSolid.h \
    collisionSphere.I collisionSphere.h \
    collisionBox.I collisionBox.h \
    collisionTraverser.I collisionTraverser.h  \
    collisionTube.I collisionTube.h \
    collisionVisualizer.I collisionVisualizer.h \
    config_collide.h
    
 #define INCLUDED_SOURCES \
    collisionEntry.cxx \
    collisionGeom.cxx \
    collisionHandler.cxx \
    collisionHandlerEvent.cxx  \
    collisionHandlerFloor.cxx \
    collisionHandlerGravity.cxx \
    collisionHandlerPhysical.cxx  \
    collisionHandlerPusher.cxx \
    collisionHandlerFluidPusher.cxx \
    collisionHandlerQueue.cxx  \
    collisionLevelStateBase.cxx \
    collisionLevelState.cxx \
    collisionDSSolid.cxx \
    collisionInvSphere.cxx  \
    collisionLine.cxx \
    collisionNode.cxx \
    collisionParabola.cxx  \
    collisionPlane.cxx  \
    collisionPolygon.cxx \
    collisionFloorMesh.cxx \
    collisionRay.cxx \
    collisionRecorder.cxx \
    collisionSegment.cxx  \
    collisionSolid.cxx \
    collisionSphere.cxx  \
    collisionBox.cxx  \
    collisionTraverser.cxx \
    collisionTube.cxx  \
    collisionVisualizer.cxx \
    config_collide.cxx 

  #define INSTALL_HEADERS \
    collisionEntry.I collisionEntry.h \
    collisionGeom.I collisionGeom.h \
    collisionHandler.I collisionHandler.h \
    collisionHandlerEvent.I collisionHandlerEvent.h \
    collisionHandlerFloor.I collisionHandlerFloor.h \
    collisionHandlerGravity.I collisionHandlerGravity.h \
    collisionHandlerPhysical.I collisionHandlerPhysical.h \
    collisionHandlerPusher.I collisionHandlerPusher.h \
    collisionHandlerFluidPusher.I collisionHandlerFluidPusher.h \
    collisionHandlerQueue.h \
    collisionDSSolid.I collisionDSSolid.h \
    collisionInvSphere.I collisionInvSphere.h \
    collisionLevelStateBase.I collisionLevelStateBase.h \
    collisionLevelState.I collisionLevelState.h \
    collisionLine.I collisionLine.h \
    collisionNode.I collisionNode.h \
    collisionParabola.I collisionParabola.h \
    collisionPlane.I collisionPlane.h \
    collisionPolygon.I collisionPolygon.h \
    collisionFloorMesh.I collisionFloorMesh.h \
    collisionRay.I collisionRay.h \
    collisionRecorder.I collisionRecorder.h \
    collisionSegment.I collisionSegment.h \
    collisionSolid.I collisionSolid.h \
    collisionSphere.I collisionSphere.h \
    collisionBox.I collisionBox.h \
    collisionTraverser.I collisionTraverser.h \
    collisionTube.I collisionTube.h \
    collisionVisualizer.I collisionVisualizer.h \
    config_collide.h


  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_collide
  #define LOCAL_LIBS \
    collide
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_collide.cxx

#end test_bin_target

