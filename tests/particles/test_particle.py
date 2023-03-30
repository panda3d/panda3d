from panda3d.core import Vec3
from panda3d.physics import LinearVectorForce
from direct.particles import ParticleEffect
from direct.particles import Particles
from direct.particles import ForceGroup


def test_particle(base):
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
    pe.reparentTo(base.render)
    #pe.setPos(0.0, 5.0, 4.0)
    pe.addForceGroup(fg)
    pe.addParticles(p)

    # Particle Panel
    # pp = ParticlePanel.ParticlePanel(pe)
