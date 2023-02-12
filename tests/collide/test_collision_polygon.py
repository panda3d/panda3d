from panda3d import core
import pytest


def test_collision_polygon_verify_not_enough_points():
    # Less than 3 points cannot create a polygon
    assert not core.CollisionPolygon.verify_points([])
    assert not core.CollisionPolygon.verify_points([core.LPoint3(1, 0, 0)])
    assert not core.CollisionPolygon.verify_points([core.LPoint3(1, 0, 0), core.LPoint3(0, 0, 1)])


def test_collision_polygon_verify_repeating_points():
    # Repeating points cannot create a polygon
    assert not core.CollisionPolygon.verify_points([core.LPoint3(1, 0, 0), core.LPoint3(1, 0, 0), core.LPoint3(0, 0, 1)])
    assert not core.CollisionPolygon.verify_points([core.LPoint3(3, 6, 1), core.LPoint3(1, 3, 5), core.LPoint3(9, 1, 2), core.LPoint3(1, 3, 5)])


def test_collision_polygon_verify_colinear_points():
    # Colinear points cannot create a polygon
    assert not core.CollisionPolygon.verify_points([core.LPoint3(1, 2, 3), core.LPoint3(2, 3, 4), core.LPoint3(3, 4, 5)])
    assert not core.CollisionPolygon.verify_points([core.LPoint3(2, 1, 1), core.LPoint3(3, 2, 1), core.LPoint3(4, 3, 1)])


def test_collision_polygon_verify_points():
    # Those should be regular, non-colinear points
    assert core.CollisionPolygon.verify_points([core.LPoint3(1, 0, 0), core.LPoint3(0, 1, 0), core.LPoint3(0, 0, 1)])
    assert core.CollisionPolygon.verify_points([core.LPoint3(10, 2, 8), core.LPoint3(7, 1, 3), core.LPoint3(5, 9, 6)])
    assert core.CollisionPolygon.verify_points([core.LPoint3(3, -8, -7), core.LPoint3(9, 10, 8), core.LPoint3(7, 0, 10), core.LPoint3(-6, -2, 3)])
    assert core.CollisionPolygon.verify_points([core.LPoint3(-1, -3, -5), core.LPoint3(10, 3, -10), core.LPoint3(-10, 10, -4), core.LPoint3(0, 1, -4), core.LPoint3(-9, -2, 0)])

    with pytest.raises(TypeError):
        core.CollisionPolygon.verify_points([core.LPoint3(0, 0, 0), None])


def test_collision_polygon_setup_points():
    # Create empty collision polygon
    polygon = core.CollisionPolygon(core.LVecBase3(0, 0, 0), core.LVecBase3(0, 0, 0), core.LVecBase3(0, 0, 0))
    assert not polygon.is_valid()

    # Test our setup method against a few test cases
    for points in [
        [core.LPoint3(-1, -3, -5), core.LPoint3(10, 3, -10), core.LPoint3(-10, 10, -4), core.LPoint3(0, 1, -4), core.LPoint3(-9, -2, 0)],
        [core.LPoint3(3, -8, -7), core.LPoint3(9, 10, 8), core.LPoint3(7, 0, 10), core.LPoint3(-6, -2, 3)],
        [core.LPoint3(1, 0, 0), core.LPoint3(0, 1, 0), core.LPoint3(0, 0, 1)],
        [core.LPoint3(10, 2, 8), core.LPoint3(7, 1, 3), core.LPoint3(5, 9, 6)]
    ]:
        polygon.setup_points(points)
        assert polygon.is_valid()
        assert polygon.get_num_points() == len(points)

    with pytest.raises(TypeError):
        polygon.setup_points([core.LPoint3(0, 0, 0), None, 1])

    with pytest.raises(ValueError):
        polygon.setup_points([core.LPoint3(0, 0, 0)])
