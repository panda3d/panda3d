
if __name__ == "__main__":
    from direct.directbase.TestStart import *

    from panda3d.physics import LinearVectorForce
    from panda3d.core import Vec3
    from . import ParticleEffect
    from direct.tkpanels import ParticlePanel
    from . import Particles
    from . import ForceGroup

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

    base.run()
