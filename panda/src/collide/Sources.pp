#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
				   
// need to modify global scope
#set NOT_INTEL_BUILDABLE true				   

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    sgmanip light tform sgraphutil gobj graph putil

  #define SOURCES \
    collisionEntry.I collisionEntry.cxx collisionEntry.h \
    collisionHandler.cxx collisionHandler.h collisionHandlerEvent.I \
    collisionHandlerEvent.cxx collisionHandlerEvent.h \
    collisionHandlerFloor.I collisionHandlerFloor.cxx \
    collisionHandlerFloor.h collisionHandlerPhysical.I \
    collisionHandlerPhysical.cxx collisionHandlerPhysical.h \
    collisionHandlerPusher.I collisionHandlerPusher.cxx \
    collisionHandlerPusher.h collisionHandlerQueue.cxx \
    collisionHandlerQueue.h collisionLevelState.I collisionLevelState.N \
    collisionLevelState.cxx collisionLevelState.h collisionNode.I \
    collisionNode.cxx collisionNode.h collisionPlane.I \
    collisionPlane.cxx collisionPlane.h collisionPolygon.I \
    collisionPolygon.cxx collisionPolygon.h collisionRay.I \
    collisionRay.cxx collisionRay.h collisionSegment.I \
    collisionSegment.cxx collisionSegment.h \
    collisionSolid.I collisionSolid.cxx \
    collisionSolid.h collisionSphere.I collisionSphere.cxx \
    collisionSphere.h collisionTraverser.I collisionTraverser.cxx \
    collisionTraverser.h config_collide.cxx config_collide.h

  #define INSTALL_HEADERS \
    collisionEntry.I collisionEntry.h collisionHandler.h \
    collisionHandlerEvent.I collisionHandlerEvent.h \
    collisionHandlerFloor.I collisionHandlerFloor.h \
    collisionHandlerPhysical.I collisionHandlerPhysical.h \
    collisionHandlerPusher.I collisionHandlerPusher.h \
    collisionHandlerQueue.h collisionLevelState.I collisionLevelState.h \
    collisionNode.I collisionNode.h collisionPlane.I collisionPlane.h \
    collisionPolygon.I collisionPolygon.h collisionRay.I collisionRay.h \
    collisionSegment.I collisionSegment.h \
    collisionSolid.I collisionSolid.h collisionSphere.I \
    collisionSphere.h collisionTraverser.I collisionTraverser.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_collide
  #define LOCAL_LIBS \
    collide

  #define SOURCES \
    test_collide.cxx

#end test_bin_target

