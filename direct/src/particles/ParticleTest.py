
if __name__ == "__main__":
    from direct.directbase.TestStart import *
    
    from pandac.LinearVectorForce import LinearVectorForce
    from pandac.Vec3 import Vec3
    import ParticleEffect
    from direct.tkpanels import ParticlePanel
    import Particles
    import ForceGroup
    
    # Showbase
    base.enableParticles()
    
    # ForceGroup
    fg = ForceGroup.ForceGroup()
    gravity = LinearVectorForce(Vec3(0.0, 0.0, -10.0))
    fg.addForce(gravity)
    
    # Particles
    p = Particles.Particles()

    # Particle effect
    pe = ParticleEffect.ParticleEffect('particle-fx')
    pe.reparentTo(render)
    #pe.setPos(0.0, 5.0, 4.0)
    pe.addForceGroup(fg)
    pe.addParticles(p)
    
    # Particle Panel
    pp = ParticlePanel.ParticlePanel(pe)

    run()
