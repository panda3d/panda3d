#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET physics
  #define LOCAL_LIBS \
    linmath sgraph sgattrib sgraphutil
 
  #define SOURCES \
    actorNode.I actorNode.cxx actorNode.h angularEulerIntegrator.cxx \
    angularEulerIntegrator.h angularForce.cxx angularForce.h \
    angularIntegrator.cxx angularIntegrator.h angularVectorForce.I \
    angularVectorForce.cxx angularVectorForce.h baseForce.I \
    baseForce.cxx baseForce.h baseIntegrator.I baseIntegrator.cxx \
    baseIntegrator.h config_physics.cxx config_physics.h forceNode.I \
    forceNode.cxx forceNode.h linearCylinderVortexForce.I \
    linearCylinderVortexForce.cxx linearCylinderVortexForce.h \
    linearDistanceForce.I linearDistanceForce.cxx linearDistanceForce.h \
    linearEulerIntegrator.cxx linearEulerIntegrator.h linearForce.I \
    linearForce.cxx linearForce.h linearFrictionForce.I \
    linearFrictionForce.cxx linearFrictionForce.h linearIntegrator.cxx \
    linearIntegrator.h linearJitterForce.cxx linearJitterForce.h \
    linearNoiseForce.I linearNoiseForce.cxx linearNoiseForce.h \
    linearRandomForce.I linearRandomForce.cxx linearRandomForce.h \
    linearSinkForce.cxx linearSinkForce.h linearSourceForce.cxx \
    linearSourceForce.h linearUserDefinedForce.I \
    linearUserDefinedForce.cxx linearUserDefinedForce.h \
    linearVectorForce.I linearVectorForce.cxx linearVectorForce.h \
    physical.I physical.cxx physical.h physicalNode.I physicalNode.cxx \
    physicalNode.h physicsManager.I physicsManager.cxx physicsManager.h \
    physicsObject.I physicsObject.cxx physicsObject.h

  #define INSTALL_HEADERS \
    actorNode.I actorNode.h angularEulerIntegrator.h angularForce.h \
    angularIntegrator.h angularVectorForce.I angularVectorForce.h \
    baseForce.I baseForce.h baseIntegrator.I baseIntegrator.h \
    config_physics.h forceNode.I forceNode.h forces.h \
    linearCylinderVortexForce.I linearCylinderVortexForce.h \
    linearDistanceForce.I linearDistanceForce.h linearEulerIntegrator.h \
    linearForce.I linearForce.h linearFrictionForce.I \
    linearFrictionForce.h linearIntegrator.h linearJitterForce.h \
    linearNoiseForce.I linearNoiseForce.h linearRandomForce.I \
    linearRandomForce.h linearSinkForce.h linearSourceForce.h \
    linearUserDefinedForce.I linearUserDefinedForce.h \
    linearVectorForce.I linearVectorForce.h physical.I physical.h \
    physicalNode.I physicalNode.h physicsManager.I physicsManager.h \
    physicsObject.I physicsObject.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_physics
  #define LOCAL_LIBS \
    linmath physics

  #define SOURCES \
    test_physics.cxx

#end test_bin_target

