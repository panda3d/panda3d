set(P3PARTICLESYSTEM_HEADERS
  baseParticle.I baseParticle.h baseParticleEmitter.I
  baseParticleEmitter.h baseParticleFactory.I
  baseParticleFactory.h baseParticleRenderer.I
  baseParticleRenderer.h arcEmitter.I arcEmitter.h
  boxEmitter.I boxEmitter.h
  config_particlesystem.h discEmitter.I discEmitter.h
  emitters.h
  geomParticleRenderer.I geomParticleRenderer.h lineEmitter.I
  lineEmitter.h lineParticleRenderer.I lineParticleRenderer.h
  particlefactories.h
  particles.h
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
  colorInterpolationManager.h
)

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
  zSpinParticleFactory.cxx colorInterpolationManager.cxx
)

set(P3PARTICLESYSTEM_IGATE_SOURCES
  ${P3PARTICLESYSTEM_HEADERS} ${P3PARTICLESYSTEM_SOURCES})
list(REMOVE_ITEM P3PARTICLESYSTEM_IGATE_SOURCES "emitters.h")
list(REMOVE_ITEM P3PARTICLESYSTEM_IGATE_SOURCES "particlefactories.h")
list(REMOVE_ITEM P3PARTICLESYSTEM_IGATE_SOURCES "particles.h")

composite_sources(p3particlesystem P3PARTICLESYSTEM_SOURCES)
add_component_library(p3particlesystem SYMBOL BUILDING_PANDA_PARTICLESYSTEM
  ${P3PARTICLESYSTEM_HEADERS} ${P3PARTICLESYSTEM_SOURCES})
target_link_libraries(p3particlesystem p3physics panda)
target_interrogate(p3particlesystem ${P3PARTICLESYSTEM_IGATE_SOURCES})

if(NOT BUILD_METALIBS)
  install(TARGETS p3particlesystem
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PARTICLESYSTEM_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
