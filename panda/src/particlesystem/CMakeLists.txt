set(P3PARTICLESYSTEM_HEADERS
    baseParticle.I baseParticle.h baseParticleEmitter.I
    baseParticleEmitter.h baseParticleFactory.I
    baseParticleFactory.h baseParticleRenderer.I
    baseParticleRenderer.h arcEmitter.I arcEmitter.h
    boxEmitter.I boxEmitter.h
    config_particlesystem.h discEmitter.I discEmitter.h
    geomParticleRenderer.I geomParticleRenderer.h lineEmitter.I
    lineEmitter.h lineParticleRenderer.I lineParticleRenderer.h
    particleSystem.I particleSystem.h particleSystemManager.I
    particleSystemManager.h pointEmitter.I pointEmitter.h
    pointParticle.h pointParticleFactory.h
    pointParticleRenderer.I pointParticleRenderer.h
    rectangleEmitter.I rectangleEmitter.h ringEmitter.I
    ringEmitter.h sparkleParticleRenderer.I
    sparkleParticleRenderer.h sphereSurfaceEmitter.I
    sphereSurfaceEmitter.h sphereVolumeEmitter.I
    sphereVolumeEmitter.h spriteParticleRenderer.I
    spriteParticleRenderer.h tangentRingEmitter.I
    tangentRingEmitter.h zSpinParticle.I zSpinParticle.h
    zSpinParticleFactory.I zSpinParticleFactory.h
    particleCommonFuncs.h colorInterpolationManager.I
    colorInterpolationManager.h)

set(P3PARTICLESYSTEM_SOURCES
    baseParticle.cxx baseParticleEmitter.cxx baseParticleFactory.cxx
    baseParticleRenderer.cxx boxEmitter.cxx arcEmitter.cxx
    config_particlesystem.cxx discEmitter.cxx
    geomParticleRenderer.cxx lineEmitter.cxx
    lineParticleRenderer.cxx particleSystem.cxx
    particleSystemManager.cxx pointEmitter.cxx pointParticle.cxx
    pointParticleFactory.cxx pointParticleRenderer.cxx
    rectangleEmitter.cxx ringEmitter.cxx
    sparkleParticleRenderer.cxx sphereSurfaceEmitter.cxx
    sphereVolumeEmitter.cxx spriteParticleRenderer.cxx
    tangentRingEmitter.cxx zSpinParticle.cxx
    zSpinParticleFactory.cxx colorInterpolationManager.cxx)


composite_sources(p3particlesystem P3PARTICLESYSTEM_SOURCES)
add_library(p3particlesystem ${P3PARTICLESYSTEM_HEADERS} ${P3PARTICLESYSTEM_SOURCES})
set_target_properties(p3particlesystem PROPERTIES DEFINE_SYMBOL BUILDING_PANDAPHYSICS)
target_link_libraries(p3particlesystem p3physics panda)
target_interrogate(p3particlesystem ALL)

install(TARGETS p3particlesystem DESTINATION lib)
install(FILES ${P3PARTICLESYSTEM_HEADERS} DESTINATION include/panda3d)
