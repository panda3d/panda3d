from direct.stdpy.pickle import dumps, loads
from panda3d.core import NodePath, CollisionNode


def test_collision_handler_event_pickle():
    from panda3d.core import CollisionHandlerEvent

    handler = CollisionHandlerEvent()
    handler.add_in_pattern("abcdefg")
    handler.add_in_pattern("test")
    handler.add_again_pattern("again pattern")
    handler.add_again_pattern("another again pattern")
    handler.add_out_pattern("out pattern")

    handler = loads(dumps(handler, -1))

    assert tuple(handler.in_patterns) == ("abcdefg", "test")
    assert tuple(handler.again_patterns) == ("again pattern", "another again pattern")
    assert tuple(handler.out_patterns) == ("out pattern",)


def test_collision_handler_queue_pickle():
    from panda3d.core import CollisionHandlerQueue

    handler = CollisionHandlerQueue()
    handler = loads(dumps(handler, -1))
    assert type(handler) == CollisionHandlerQueue


def test_collision_handler_floor_pickle():
    from panda3d.core import CollisionHandlerFloor

    collider1 = NodePath(CollisionNode("collider1"))
    collider2 = NodePath(CollisionNode("collider2"))
    target1 = NodePath("target1")
    target2 = NodePath("target2")
    center = NodePath("center")

    handler = CollisionHandlerFloor()
    handler.add_out_pattern("out pattern")
    handler.add_collider(collider1, target1)
    handler.add_collider(collider2, target2)
    handler.center = center
    handler.offset = 1.0
    handler.reach = 2.0
    handler.max_velocity = 3.0

    handler = loads(dumps(handler, -1))

    assert tuple(handler.in_patterns) == ()
    assert tuple(handler.again_patterns) == ()
    assert tuple(handler.out_patterns) == ("out pattern",)
    assert handler.center.name == "center"
    assert handler.offset == 1.0
    assert handler.reach == 2.0
    assert handler.max_velocity == 3.0


def test_collision_handler_gravity_pickle():
    from panda3d.core import CollisionHandlerGravity

    collider1 = NodePath(CollisionNode("collider1"))
    collider2 = NodePath(CollisionNode("collider2"))
    target1 = NodePath("target1")
    target2 = NodePath("target2")

    handler = CollisionHandlerGravity()
    handler.add_out_pattern("out pattern")
    handler.add_collider(collider1, target1)
    handler.add_collider(collider2, target2)
    handler.offset = 1.0
    handler.reach = 2.0
    handler.max_velocity = 3.0
    handler.gravity = -4.0

    handler = loads(dumps(handler, -1))

    assert tuple(handler.in_patterns) == ()
    assert tuple(handler.again_patterns) == ()
    assert tuple(handler.out_patterns) == ("out pattern",)
    assert handler.center == None
    assert handler.offset == 1.0
    assert handler.reach == 2.0
    assert handler.max_velocity == 3.0
    assert handler.gravity == -4.0


def test_collision_handler_pusher_pickle():
    from panda3d.core import CollisionHandlerPusher

    collider1 = NodePath(CollisionNode("collider1"))
    collider2 = NodePath(CollisionNode("collider2"))
    target1 = NodePath("target1")
    target2 = NodePath("target2")

    handler = CollisionHandlerPusher()
    handler.add_again_pattern("again pattern")
    handler.add_collider(collider1, target1)
    handler.add_collider(collider2, target2)
    handler.horizontal = True

    handler = loads(dumps(handler, -1))

    assert tuple(handler.in_patterns) == ()
    assert tuple(handler.again_patterns) == ("again pattern",)
    assert tuple(handler.out_patterns) == ()
    assert not handler.has_center()
    assert handler.horizontal
