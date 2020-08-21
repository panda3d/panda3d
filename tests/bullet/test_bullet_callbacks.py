import pytest
from .conftest import simulate_until

# Skip these tests if we can't import bullet.
bullet = pytest.importorskip("panda3d.bullet")
from panda3d import core

bullet_filter_algorithm = core.ConfigVariableString('bullet-filter-algorithm')


def test_tick(world):
    fired = []

    def callback(cd):
        fired.append(isinstance(cd, bullet.BulletTickCallbackData))

    world.set_tick_callback(callback, False)

    assert fired == []

    world.do_physics(0.1)

    assert fired == [True]

    world.clear_tick_callback()

    world.do_physics(0.1)

    assert fired == [True]

@pytest.mark.skipif(bullet_filter_algorithm != 'callback', reason='bullet-filter-algorithm not set to callback')
def test_filter(world, scene):
    # This is very similar to the basic physics test, but we're using
    # a filter callback to prevent collisions between the lower box and ball.
    # This should have the effect of the ball rolling to X>+10 without the
    # upper box falling at all.

    def callback(cd):
        assert isinstance(cd, bullet.BulletFilterCallbackData)
        if {cd.node_0.name, cd.node_1.name} == {'ball', 'lower_box'}:
            # ball<->lower_box collisions are excluded
            cd.collide = False
        else:
            # Everything else can collide
            cd.collide = True

    world.set_filter_callback(callback)

    ball = scene.find('**/ball')
    assert simulate_until(world, lambda: ball.get_x() > 10)

    # The upper box shouldn't fall
    upper_box = scene.find('**/upper_box')
    assert not simulate_until(world, lambda: upper_box.get_z() < 5)

def test_contact(world, scene):
    # This just runs the basic physics test, but detects the toppling of the
    # upper box by a contact between upper_box<->ramp

    contacts = []

    def callback(cd):
        assert isinstance(cd, bullet.BulletContactCallbackData)
        if {cd.node0.name, cd.node1.name} == {'upper_box', 'ramp'}:
            if not contacts:
                contacts.append(True)

    world.set_contact_added_callback(callback)

    ball = scene.find('**/ball')
    ramp = scene.find('**/ramp')

    ball.node().notify_collisions(True)
    ramp.node().notify_collisions(True)

    assert simulate_until(world, lambda: ball.get_x() > 0)

    # Now we wait for the upper box to topple
    assert simulate_until(world, lambda: bool(contacts))
