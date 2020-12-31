from direct.stdpy.pickle import dumps, loads


def test_collision_handler_event_pickle():
    from panda3d.core import CollisionHandlerEvent

    handler = CollisionHandlerEvent()
    handler.add_in_pattern("abcdefg")
    handler.add_in_pattern("test")
    handler.add_out_pattern("out pattern")
    handler.add_again_pattern("again pattern")
    handler.add_again_pattern("another again pattern")

    handler = loads(dumps(handler, -1))

    assert tuple(handler.in_patterns) == ("abcdefg", "test")
    assert tuple(handler.out_patterns) == ("out pattern",)
    assert tuple(handler.again_patterns) == ("again pattern", "another again pattern")
