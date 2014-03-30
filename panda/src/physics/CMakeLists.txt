set(P3PHYSICS_HEADERS
    actorNode.I actorNode.h angularEulerIntegrator.h angularForce.h
    angularIntegrator.h angularVectorForce.I
    angularVectorForce.h baseForce.I baseForce.h
    baseIntegrator.I baseIntegrator.h config_physics.h
    forceNode.I forceNode.h
    linearControlForce.I linearControlForce.h
    linearCylinderVortexForce.I linearCylinderVortexForce.h
    linearDistanceForce.I
    linearDistanceForce.h linearEulerIntegrator.h linearForce.I
    linearForce.h linearFrictionForce.I linearFrictionForce.h
    linearIntegrator.h linearJitterForce.h linearNoiseForce.I
    linearNoiseForce.h linearRandomForce.I linearRandomForce.h
    linearSinkForce.h linearSourceForce.h
    linearUserDefinedForce.I linearUserDefinedForce.h
    linearVectorForce.I linearVectorForce.h physical.I
    physical.h physicalNode.I physicalNode.h
    physicsCollisionHandler.I physicsCollisionHandler.h
    physicsManager.I physicsManager.h
    physicsObject.I physicsObject.h
    physicsObjectCollection.I physicsObjectCollection.h)

set(P3PHYSICS_SOURCES
    actorNode.cxx angularEulerIntegrator.cxx angularForce.cxx
    angularIntegrator.cxx angularVectorForce.cxx baseForce.cxx
    baseIntegrator.cxx config_physics.cxx forceNode.cxx
    linearControlForce.cxx
    linearCylinderVortexForce.cxx linearDistanceForce.cxx
    linearEulerIntegrator.cxx linearForce.cxx
    linearFrictionForce.cxx linearIntegrator.cxx
    linearJitterForce.cxx linearNoiseForce.cxx
    linearRandomForce.cxx linearSinkForce.cxx
    linearSourceForce.cxx linearUserDefinedForce.cxx
    linearVectorForce.cxx physical.cxx physicalNode.cxx
    physicsCollisionHandler.cxx physicsManager.cxx physicsObject.cxx
    physicsObjectCollection.cxx)

composite_sources(p3physics P3PHYSICS_SOURCES)
add_library(p3physics ${P3PHYSICS_HEADERS} ${P3PHYSICS_SOURCES})
target_link_libraries(p3physics p3pgraph p3linmath p3collide)
target_interrogate(p3physics ALL)

install(TARGETS p3physics DESTINATION lib)
