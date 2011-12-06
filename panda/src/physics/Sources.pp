#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3physics
  #define LOCAL_LIBS \
    p3pgraph p3linmath p3collide
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
 
  #define SOURCES \
     actorNode.I actorNode.h angularEulerIntegrator.h angularForce.h \
     angularIntegrator.h angularVectorForce.I \
     angularVectorForce.h baseForce.I baseForce.h \
     baseIntegrator.I baseIntegrator.h config_physics.h \
     forceNode.I forceNode.h \
     linearControlForce.I linearControlForce.h \
     linearCylinderVortexForce.I linearCylinderVortexForce.h \
     linearDistanceForce.I \
     linearDistanceForce.h linearEulerIntegrator.h linearForce.I \
     linearForce.h linearFrictionForce.I linearFrictionForce.h \
     linearIntegrator.h linearJitterForce.h linearNoiseForce.I \
     linearNoiseForce.h linearRandomForce.I linearRandomForce.h \
     linearSinkForce.h linearSourceForce.h \
     linearUserDefinedForce.I linearUserDefinedForce.h \
     linearVectorForce.I linearVectorForce.h physical.I \
     physical.h physicalNode.I physicalNode.h \
     physicsCollisionHandler.I physicsCollisionHandler.h \
     physicsManager.I physicsManager.h \
     physicsObject.I physicsObject.h \
     physicsObjectCollection.I physicsObjectCollection.h 

  #define INCLUDED_SOURCES \
     actorNode.cxx angularEulerIntegrator.cxx angularForce.cxx \
     angularIntegrator.cxx angularVectorForce.cxx baseForce.cxx \
     baseIntegrator.cxx config_physics.cxx forceNode.cxx \
     linearControlForce.cxx \
     linearCylinderVortexForce.cxx linearDistanceForce.cxx \
     linearEulerIntegrator.cxx linearForce.cxx \
     linearFrictionForce.cxx linearIntegrator.cxx \
     linearJitterForce.cxx linearNoiseForce.cxx \
     linearRandomForce.cxx linearSinkForce.cxx \
     linearSourceForce.cxx linearUserDefinedForce.cxx \
     linearVectorForce.cxx physical.cxx physicalNode.cxx \
     physicsCollisionHandler.cxx physicsManager.cxx physicsObject.cxx \
     physicsObjectCollection.cxx 

  #define INSTALL_HEADERS \
    actorNode.I actorNode.h angularEulerIntegrator.h angularForce.h \
    angularIntegrator.h angularVectorForce.I angularVectorForce.h \
    baseForce.I baseForce.h baseIntegrator.I baseIntegrator.h \
    config_physics.h forceNode.I forceNode.h forces.h \
    linearControlForce.I linearControlForce.h \
    linearCylinderVortexForce.I linearCylinderVortexForce.h \
    linearDistanceForce.I linearDistanceForce.h linearEulerIntegrator.h \
    linearForce.I linearForce.h linearFrictionForce.I \
    linearFrictionForce.h linearIntegrator.h linearJitterForce.h \
    linearNoiseForce.I linearNoiseForce.h linearRandomForce.I \
    linearRandomForce.h linearSinkForce.h linearSourceForce.h \
    linearUserDefinedForce.I linearUserDefinedForce.h \
    linearVectorForce.I linearVectorForce.h physical.I physical.h \
    physicalNode.I physicalNode.h \
    physicsCollisionHandler.I physicsCollisionHandler.h \
    physicsManager.I physicsManager.h \
    physicsObject.I physicsObject.h \
    physicsObjectCollection.h physicsObjectCollection.I

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_physics
  #define LOCAL_LIBS \
    p3linmath p3physics p3collide

  #define SOURCES \
    test_physics.cxx

#end test_bin_target

