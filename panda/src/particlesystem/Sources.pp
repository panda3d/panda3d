#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET particlesystem
  #define LOCAL_LIBS \
    physics sgraph sgattrib graph sgraphutil

  #define SOURCES \
    baseParticle.I baseParticle.cxx baseParticle.h \
    baseParticleEmitter.I baseParticleEmitter.cxx baseParticleEmitter.h \
    baseParticleFactory.I baseParticleFactory.cxx baseParticleFactory.h \
    baseParticleRenderer.I baseParticleRenderer.cxx \
    baseParticleRenderer.h boxEmitter.I boxEmitter.cxx boxEmitter.h \
    config_particlesystem.cxx config_particlesystem.h discEmitter.I \
    discEmitter.cxx discEmitter.h geomParticleRenderer.I \
    geomParticleRenderer.cxx geomParticleRenderer.h lineEmitter.I \
    lineEmitter.cxx lineEmitter.h orientedParticle.I \
    orientedParticle.cxx orientedParticle.h orientedParticleFactory.I \
    orientedParticleFactory.cxx orientedParticleFactory.h \
    particleSystem.I particleSystem.cxx particleSystem.h \
    particleSystemManager.I particleSystemManager.cxx \
    particleSystemManager.h pointEmitter.I pointEmitter.cxx \
    pointEmitter.h pointParticle.cxx pointParticle.h \
    pointParticleFactory.cxx pointParticleFactory.h \
    pointParticleRenderer.I pointParticleRenderer.cxx \
    pointParticleRenderer.h rectangleEmitter.I rectangleEmitter.cxx \
    rectangleEmitter.h ringEmitter.I ringEmitter.cxx ringEmitter.h \
    sparkleParticleRenderer.I sparkleParticleRenderer.cxx \
    sparkleParticleRenderer.h sphereSurfaceEmitter.I \
    sphereSurfaceEmitter.cxx sphereSurfaceEmitter.h \
    sphereVolumeEmitter.I sphereVolumeEmitter.cxx sphereVolumeEmitter.h \
    spriteParticleRenderer.I spriteParticleRenderer.cxx \
    spriteParticleRenderer.h tangentRingEmitter.I \
    tangentRingEmitter.cxx tangentRingEmitter.h zSpinParticle.I \
    zSpinParticle.cxx zSpinParticle.h zSpinParticleFactory.I \
    zSpinParticleFactory.cxx zSpinParticleFactory.h

  #define INSTALL_HEADERS \
    baseParticle.I baseParticle.h baseParticleEmitter.I \
    baseParticleEmitter.h baseParticleFactory.I baseParticleFactory.h \
    baseParticleRenderer.I baseParticleRenderer.h boxEmitter.I \
    boxEmitter.h config_particlesystem.h discEmitter.I discEmitter.h \
    emitters.h geomParticleRenderer.I geomParticleRenderer.h \
    lineEmitter.I lineEmitter.h orientedParticle.I orientedParticle.h \
    orientedParticleFactory.I orientedParticleFactory.h \
    particleSystem.I particleSystem.h particleSystemManager.I \
    particleSystemManager.h particlefactories.h particles.h \
    pointEmitter.I pointEmitter.h pointParticle.h \
    pointParticleFactory.h pointParticleRenderer.I \
    pointParticleRenderer.h rectangleEmitter.I rectangleEmitter.h \
    ringEmitter.I ringEmitter.h sparkleParticleRenderer.I \
    sparkleParticleRenderer.h sphereSurfaceEmitter.I \
    sphereSurfaceEmitter.h sphereVolumeEmitter.I sphereVolumeEmitter.h \
    spriteParticleRenderer.I spriteParticleRenderer.h \
    tangentRingEmitter.I tangentRingEmitter.h zSpinParticle.I \
    zSpinParticle.h zSpinParticleFactory.I zSpinParticleFactory.h

  #define IGATESCAN all

#end lib_target

