from panda3d.core import NodePath, PandaNode
from direct.particles.ParticleEffect import ParticleEffect
from direct.particles.Particles import Particles

def test_particle_soft_start():
    # Create a particle effect and a particle system.
    # The effect serves to test the Python-level "softStart"
    # method, while the systme serves to test the C++-level
    # "soft_start" method (via the associated Python "softStart"
    # method)
    effect = ParticleEffect()
    system = Particles("testSystem", 10)

    # Setup some dummy nodes, since it seems to want them
    system.setRenderParent(NodePath(PandaNode("test")))
    system.setSpawnRenderNodePath(NodePath(PandaNode("test")))

    # Add the system to the effect
    effect.addParticles(system)

    # Re-assign the system, just to make sure that we have the
    # right object.
    system = effect.getParticlesList()[0]

    # First, standard "softStart"--i.e. without changing either
    # birth-rate or applying an offset. This should work as it
    # used to.
    effect.softStart()

    assert (system.getBirthRate() == 0.5)

    # Now, check that the pre-existing single-parameter soft-start,
    # which alters the birth-rate, still does so.
    system.softStart(1)

    assert (system.getBirthRate() == 1)

    # Next, birth-offsetting.

    # Run a standard soft-start, then check that the birth-timer
    # is zero, as used to be the case on running this command.
    effect.softStart()

    assert (system.getTicsSinceBirth() == 0)

    # Run an offset soft-start via the system, then check
    # that the birth-timer has the assigned value,
    # and that the birth-rate is unchanged.

    # (We pass in a birth-rate ("br") of -1 because the related code
    # checks for a birth-rate greater than 0, I believe. This allows
    # us to change the offset-time without affecting the birth-rate.)
    system.softStart(br = -1, first_birth_offset_time = 2)

    assert (system.getBirthRate() == 1)
    assert (system.getTicsSinceBirth() == 2)

    # Now, run an offset soft-start via the effect, and
    # again check that the birth-timer has changed as intended,
    # and the birth-rate hasn't changed at all.
    effect.softStart(firstBirthOffset = -0.25)

    assert (system.getBirthRate() == 1)
    assert (system.getTicsSinceBirth() == -0.25)

    # Update the system, advancing it far enough that it should
    # have birthed a particle if not for the offset, but not
    # so far that it should have birthed a particle >with<
    # the offset. Check thus that no particles have been birthed.
    system.update(1)

    assert (system.getLivingParticles() == 0)

    # Update the system again, this time far enough that with the
    # offset it should have birthed just one particle, and
    # then check that this is the case.
    system.update(1)

    assert (system.getLivingParticles() == 1)

    # And finally, check that an unaltered system births as
    # expected given a single update:
    systemControl = Particles("testSystem; control", 10)

    systemControl.setRenderParent(NodePath(PandaNode("test 2")))
    systemControl.setSpawnRenderNodePath(NodePath(PandaNode("test 2")))

    assert (systemControl.getBirthRate() == 0.5)
    assert (systemControl.getTicsSinceBirth() == 0)

    assert (systemControl.getLivingParticles() == 0)

    systemControl.update(0.6)

    assert (systemControl.getLivingParticles() == 1)

    # Done! :D