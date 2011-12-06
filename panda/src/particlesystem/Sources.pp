#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3particlesystem
  #define LOCAL_LIBS \
    p3pgraph p3physics
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     baseParticle.I baseParticle.h baseParticleEmitter.I  \
     baseParticleEmitter.h baseParticleFactory.I  \
     baseParticleFactory.h baseParticleRenderer.I  \
     baseParticleRenderer.h arcEmitter.I arcEmitter.h \
     boxEmitter.I boxEmitter.h  \
     config_particlesystem.h discEmitter.I discEmitter.h  \
     geomParticleRenderer.I geomParticleRenderer.h lineEmitter.I  \
     lineEmitter.h lineParticleRenderer.I lineParticleRenderer.h  \
     particleSystem.I particleSystem.h particleSystemManager.I  \
     particleSystemManager.h pointEmitter.I pointEmitter.h  \
     pointParticle.h pointParticleFactory.h  \
     pointParticleRenderer.I pointParticleRenderer.h  \
     rectangleEmitter.I rectangleEmitter.h ringEmitter.I  \
     ringEmitter.h sparkleParticleRenderer.I  \
     sparkleParticleRenderer.h sphereSurfaceEmitter.I  \
     sphereSurfaceEmitter.h sphereVolumeEmitter.I  \
     sphereVolumeEmitter.h spriteParticleRenderer.I  \
     spriteParticleRenderer.h tangentRingEmitter.I  \
     tangentRingEmitter.h zSpinParticle.I zSpinParticle.h  \
     zSpinParticleFactory.I zSpinParticleFactory.h  \
     particleCommonFuncs.h colorInterpolationManager.I \
     colorInterpolationManager.h 

// oriented particles currently unimplemented
//     orientedParticle.I orientedParticle.h  \
//     orientedParticleFactory.I orientedParticleFactory.h  \
    
 #define INCLUDED_SOURCES \
     baseParticle.cxx baseParticleEmitter.cxx baseParticleFactory.cxx \
     baseParticleRenderer.cxx boxEmitter.cxx arcEmitter.cxx \
     config_particlesystem.cxx discEmitter.cxx \
     geomParticleRenderer.cxx lineEmitter.cxx \
     lineParticleRenderer.cxx particleSystem.cxx \
     particleSystemManager.cxx pointEmitter.cxx pointParticle.cxx \
     pointParticleFactory.cxx pointParticleRenderer.cxx \
     rectangleEmitter.cxx ringEmitter.cxx \
     sparkleParticleRenderer.cxx sphereSurfaceEmitter.cxx \
     sphereVolumeEmitter.cxx spriteParticleRenderer.cxx \
     tangentRingEmitter.cxx zSpinParticle.cxx \
     zSpinParticleFactory.cxx colorInterpolationManager.cxx

// orientedParticle.cxx orientedParticleFactory.cxx 

  #define INSTALL_HEADERS \
    baseParticle.I baseParticle.h baseParticleEmitter.I \
    baseParticleEmitter.h baseParticleFactory.I baseParticleFactory.h \
    baseParticleRenderer.I baseParticleRenderer.h arcEmitter.I arcEmitter.h \
    boxEmitter.I boxEmitter.h config_particlesystem.h \
    discEmitter.I discEmitter.h \
    emitters.h geomParticleRenderer.I geomParticleRenderer.h \
    lineEmitter.I lineEmitter.h lineParticleRenderer.I \
    lineParticleRenderer.h particleSystem.I particleSystem.h \
    particleSystemManager.I \
    particleSystemManager.h particlefactories.h particles.h \
    pointEmitter.I pointEmitter.h pointParticle.h \
    pointParticleFactory.h pointParticleRenderer.I \
    pointParticleRenderer.h rectangleEmitter.I rectangleEmitter.h \
    ringEmitter.I ringEmitter.h sparkleParticleRenderer.I \
    sparkleParticleRenderer.h sphereSurfaceEmitter.I \
    sphereSurfaceEmitter.h sphereVolumeEmitter.I sphereVolumeEmitter.h \
    spriteParticleRenderer.I spriteParticleRenderer.h \
    tangentRingEmitter.I tangentRingEmitter.h zSpinParticle.I \
    zSpinParticle.h zSpinParticleFactory.I zSpinParticleFactory.h \
    particleCommonFuncs.h colorInterpolationManager.I \
    colorInterpolationManager.h

// orientedParticle.I orientedParticle.h \
// orientedParticleFactory.I orientedParticleFactory.h \

  #define IGATESCAN all

#end lib_target

