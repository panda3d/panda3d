#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    tform gobj pgraph putil \
    pstatclient
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    qpcollisionEntry.I qpcollisionEntry.h \
    qpcollisionHandler.h  \
    qpcollisionHandlerEvent.I qpcollisionHandlerEvent.h  \
    qpcollisionHandlerFloor.I qpcollisionHandlerFloor.h  \
    qpcollisionHandlerPhysical.I qpcollisionHandlerPhysical.h  \
    qpcollisionHandlerPusher.I qpcollisionHandlerPusher.h  \
    qpcollisionHandlerQueue.h \
    qpcollisionLevelState.I qpcollisionLevelState.h \
    qpcollisionNode.I qpcollisionNode.h \
    collisionPlane.I collisionPlane.h  \
    collisionPolygon.I collisionPolygon.h collisionRay.I  \
    collisionRay.h collisionSegment.I collisionSegment.h  \
    collisionSolid.I collisionSolid.h collisionSphere.I  \
    collisionSphere.h \
    qpcollisionTraverser.I qpcollisionTraverser.h  \
    config_collide.h
    
 #define INCLUDED_SOURCES \
    qpcollisionEntry.cxx \
    qpcollisionHandler.cxx \
    qpcollisionHandlerEvent.cxx  \
    qpcollisionHandlerFloor.cxx \
    qpcollisionHandlerPhysical.cxx  \
    qpcollisionHandlerPusher.cxx \
    qpcollisionHandlerQueue.cxx  \
    qpcollisionLevelState.cxx \
    qpcollisionNode.cxx \
    collisionPlane.cxx  \
    collisionPolygon.cxx collisionRay.cxx collisionSegment.cxx  \
    collisionSolid.cxx collisionSphere.cxx  \
    qpcollisionTraverser.cxx \
    config_collide.cxx 

  #define INSTALL_HEADERS \
    qpcollisionEntry.I qpcollisionEntry.h \
    qpcollisionHandler.h \
    qpcollisionHandlerEvent.I qpcollisionHandlerEvent.h \
    qpcollisionHandlerFloor.I qpcollisionHandlerFloor.h \
    qpcollisionHandlerPhysical.I qpcollisionHandlerPhysical.h \
    qpcollisionHandlerPusher.I qpcollisionHandlerPusher.h \
    qpcollisionHandlerQueue.h \
    qpcollisionLevelState.I qpcollisionLevelState.h \
    qpcollisionNode.I qpcollisionNode.h \
    collisionPlane.I collisionPlane.h \
    collisionPolygon.I collisionPolygon.h collisionRay.I collisionRay.h \
    collisionSegment.I collisionSegment.h \
    collisionSolid.I collisionSolid.h collisionSphere.I \
    collisionSphere.h \
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

