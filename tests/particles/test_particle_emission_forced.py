from panda3d.core import NodePath, PandaNode
from direct.particles.ParticleEffect import ParticleEffect
from direct.particles.Particles import Particles

def test_particle_burst_emission():
    # Create a particle effect and a particle system.
    effect = ParticleEffect()
    system = Particles("testSystem", 10)

    # Add the system to the effect
    effect.addParticles(system)

    # Setup some dummy nodes, since it seems to want them
    # We might normally call "start", but that calls "enable", which
    # seems to assume that "base" exists and has physics and particle managers...
    system.setRenderParent(NodePath(PandaNode("test")))
    system.setSpawnRenderNodePath(NodePath(PandaNode("test")))
    # However, we don't want normal emission, so we now soft-stop it immediately,
    # before the system has a chance to update and emit.
    effect.softStop()

    # Now, a sanity-check: assert that we have no particles,
    # Then update the system, and assert again that we
    # have no particles. If so, then we're (hopefully)
    # not emitting normally!

    assert system.getLivingParticles() == 0

    system.update(1)

    assert system.getLivingParticles() == 0

    # Now, the real test: emit a particle-burst!
    effect.forceLitterBirth()

    # And assert that a particle has, in fact, been emitted.
    assert system.getLivingParticles() == 1

    # Check the snake-case version, too.
    effect.force_litter_birth()

    assert system.getLivingParticles() == 2

    print ("Done")
    # Done! :D
