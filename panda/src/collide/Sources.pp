#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    sgmanip light tform sgraphutil gobj graph putil \
    pstatclient
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    collisionEntry.I collisionEntry.h \
    qpcollisionEntry.I qpcollisionEntry.h \
    collisionHandler.h  \
    qpcollisionHandler.h  \
    collisionHandlerEvent.I collisionHandlerEvent.h  \
    qpcollisionHandlerEvent.I qpcollisionHandlerEvent.h  \
    collisionHandlerFloor.I collisionHandlerFloor.h  \
    qpcollisionHandlerFloor.I qpcollisionHandlerFloor.h  \
    collisionHandlerPhysical.I collisionHandlerPhysical.h  \
    qpcollisionHandlerPhysical.I qpcollisionHandlerPhysical.h  \
    collisionHandlerPusher.I collisionHandlerPusher.h  \
    qpcollisionHandlerPusher.I qpcollisionHandlerPusher.h  \
    collisionHandlerQueue.h \
    qpcollisionHandlerQueue.h \
    collisionLevelState.I collisionLevelState.h \
    qpcollisionLevelState.I qpcollisionLevelState.h \
    collisionNode.I collisionNode.h \
    qpcollisionNode.I qpcollisionNode.h \
    collisionPlane.I collisionPlane.h  \
    collisionPolygon.I collisionPolygon.h collisionRay.I  \
    collisionRay.h collisionSegment.I collisionSegment.h  \
    collisionSolid.I collisionSolid.h collisionSphere.I  \
    collisionSphere.h \
    collisionTraverser.I collisionTraverser.h  \
    qpcollisionTraverser.I qpcollisionTraverser.h  \
    config_collide.h
    
 #define INCLUDED_SOURCES \
    collisionEntry.cxx \
    qpcollisionEntry.cxx \
    collisionHandler.cxx \
    qpcollisionHandler.cxx \
    collisionHandlerEvent.cxx  \
    qpcollisionHandlerEvent.cxx  \
    collisionHandlerFloor.cxx \
    qpcollisionHandlerFloor.cxx \
    collisionHandlerPhysical.cxx  \
    qpcollisionHandlerPhysical.cxx  \
    collisionHandlerPusher.cxx \
    qpcollisionHandlerPusher.cxx \
    collisionHandlerQueue.cxx  \
    qpcollisionHandlerQueue.cxx  \
    collisionLevelState.cxx \
    qpcollisionLevelState.cxx \
    collisionNode.cxx \
    qpcollisionNode.cxx \
    collisionPlane.cxx  \
    collisionPolygon.cxx collisionRay.cxx collisionSegment.cxx  \
    collisionSolid.cxx collisionSphere.cxx  \
    collisionTraverser.cxx \
    qpcollisionTraverser.cxx \
    config_collide.cxx 

  #define INSTALL_HEADERS \
    collisionEntry.I collisionEntry.h \
    qpcollisionEntry.I qpcollisionEntry.h \
    collisionHandler.h \
    qpcollisionHandler.h \
    collisionHandlerEvent.I collisionHandlerEvent.h \
    qpcollisionHandlerEvent.I qpcollisionHandlerEvent.h \
    collisionHandlerFloor.I collisionHandlerFloor.h \
    qpcollisionHandlerFloor.I qpcollisionHandlerFloor.h \
    collisionHandlerPhysical.I collisionHandlerPhysical.h \
    qpcollisionHandlerPhysical.I qpcollisionHandlerPhysical.h \
    collisionHandlerPusher.I collisionHandlerPusher.h \
    qpcollisionHandlerPusher.I qpcollisionHandlerPusher.h \
    collisionHandlerQueue.h \
    qpcollisionHandlerQueue.h \
    collisionLevelState.I collisionLevelState.h \
    qpcollisionLevelState.I qpcollisionLevelState.h \
    collisionNode.I collisionNode.h \
    qpcollisionNode.I qpcollisionNode.h \
    collisionPlane.I collisionPlane.h \
    collisionPolygon.I collisionPolygon.h collisionRay.I collisionRay.h \
    collisionSegment.I collisionSegment.h \
    collisionSolid.I collisionSolid.h collisionSphere.I \
    collisionSphere.h \
    collisionTraverser.I collisionTraverser.h \
    qpcollisionTraverser.I qpcollisionTraverser.h

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

