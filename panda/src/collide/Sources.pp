#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    sgmanip light tform sgraphutil gobj graph putil \
    pstatclient
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     collisionEntry.I collisionEntry.h collisionHandler.h  \
     collisionHandlerEvent.I collisionHandlerEvent.h  \
     collisionHandlerFloor.I collisionHandlerFloor.h  \
     collisionHandlerPhysical.I collisionHandlerPhysical.h  \
     collisionHandlerPusher.I collisionHandlerPusher.h  \
     collisionHandlerQueue.h collisionLevelState.I  \
     collisionLevelState.N collisionLevelState.h collisionNode.I  \
     collisionNode.h collisionPlane.I collisionPlane.h  \
     collisionPolygon.I collisionPolygon.h collisionRay.I  \
     collisionRay.h collisionSegment.I collisionSegment.h  \
     collisionSolid.I collisionSolid.h collisionSphere.I  \
     collisionSphere.h collisionTraverser.I collisionTraverser.h  \
     config_collide.h
    
 #define INCLUDED_SOURCES \
     collisionEntry.cxx collisionHandler.cxx collisionHandlerEvent.cxx  \
     collisionHandlerFloor.cxx collisionHandlerPhysical.cxx  \
     collisionHandlerPusher.cxx collisionHandlerQueue.cxx  \
     collisionLevelState.cxx collisionNode.cxx collisionPlane.cxx  \
     collisionPolygon.cxx collisionRay.cxx collisionSegment.cxx  \
     collisionSolid.cxx collisionSphere.cxx  \
     collisionTraverser.cxx config_collide.cxx 

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

//    #define PRECOMPILED_HEADER collide_headers.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_collide
  #define LOCAL_LIBS \
    collide

  #define SOURCES \
    test_collide.cxx

#end test_bin_target

