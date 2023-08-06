import pytest
pytest.importorskip("panda3d.physics")

from panda3d.core import NodePath, PandaNode
from direct.particles.ParticleEffect import ParticleEffect
from direct.particles.Particles import Particles


def test_particle_birth_rate():
    # Tests a system with a standard birth rate of 0.5, that it is
    # indeed birthing at that rate.  It serves as a control for the
    # next test as well.
    system = Particles("testSystem", 2)

    system.set_render_parent(NodePath(PandaNode("test")))
    system.set_spawn_render_node_path(NodePath(PandaNode("test")))

    assert system.get_birth_rate() == 0.5
    assert system.get_tics_since_birth() == 0
    assert system.get_living_particles() == 0

    system.update(0.6)
    assert system.get_living_particles() == 1

    system.update(0.5)
    assert system.get_living_particles() == 2

    # Should still be 2, since the pool size was 2.
    system.update(0.5)
    assert system.get_living_particles() == 2


def test_particle_soft_start():
    # Create a particle effect and a particle system.
    # The effect serves to test the Python-level "soft_start"  method,
    # while the system serves to test the C++-level "soft_start" method
    # (via the associated Python "soft_start" method)
    effect = ParticleEffect()
    system = Particles("testSystem", 10)

    # Setup some dummy nodes, since it seems to want them
    system.set_render_parent(NodePath(PandaNode("test")))
    system.set_spawn_render_node_path(NodePath(PandaNode("test")))

    # Add the system to the effect
    effect.add_particles(system)

    # Re-assign the system, just to make sure that we have the
    # right object.
    system = effect.get_particles_list()[0]

    # First, standard "soft_start"--i.e. without either changing
    # the birth-rate or applying a delay. This should work as it
    # used to.
    effect.soft_start()

    assert system.get_birth_rate() == 0.5

    # Now, check that the pre-existing single-parameter soft-start,
    # which alters the birth-rate, still does so.
    system.soft_start(1)

    assert system.get_birth_rate() == 1

    # Next, birth-delaying.

    # Run a standard soft-start, then check that the birth-timer
    # is zero, as used to be the case on running this command.
    effect.soft_start()

    assert system.get_tics_since_birth() == 0

    # Run an delayed soft-start via the system, then check that the
    # birth-timer has the assigned value, and that the birth-rate is
    # unchanged.

    # (We pass in a birth-rate ("br") of -1 because the related code
    # checks for a birth-rate greater than 0, I believe. This allows
    # us to change the delay without affecting the birth-rate.)
    system.soft_start(br=-1, first_birth_delay=-2)

    assert system.get_birth_rate() == 1
    assert system.get_tics_since_birth() == 2

    # Now, run a delayed soft-start via the effect, and
    # again check that the birth-timer has changed as intended,
    # and the birth-rate hasn't changed at all.
    effect.soft_start(firstBirthDelay=0.25)

    assert system.get_birth_rate() == 1
    assert system.get_tics_since_birth() == -0.25

    # Update the system, advancing it far enough that it should
    # have birthed a particle if not for the delay, but not
    # so far that it should have birthed a particle >with<
    # the delay. Check thus that no particles have been birthed.
    system.update(1)

    assert system.get_living_particles() == 0

    # Update the system again, this time far enough that with the
    # delay it should have birthed just one particle, and
    # then check that this is the case.
    system.update(1)

    assert system.get_living_particles() == 1


def test_particle_burst_emission():
    effect = ParticleEffect()
    system = Particles("testSystem", 10)
    effect.add_particles(system)

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
    effect.birthLitter()

    # And assert that a particle has, in fact, been emitted.
    assert system.getLivingParticles() == 1

    # Check the snake-case version, too.
    effect.birth_litter()

    assert system.getLivingParticles() == 2
