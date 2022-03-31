# Testing that all variants of CollisionLine
# cannot be used as "into" objects
from collisions import *


def test_sphere_into_line():
    entry = make_collision(CollisionSphere(0, 0, 0, 3), CollisionLine(0, 0, 0, 1, 0, 0))[0]
    assert entry is None


def test_sphere_into_ray():
    entry = make_collision(CollisionSphere(0, 0, 0, 3), CollisionRay(0, 0, 0, 3, 3, 3))[0]
    assert entry is None


def test_sphere_into_segment():
    entry = make_collision(CollisionSphere(0, 0, 0, 3), CollisionSegment(0, 0, 0, 3, 3, 3))[0]
    assert entry is None


def test_sphere_into_parabola():
    parabola = LParabola((1, 0, 0), (0, 1, 0), (0, 0, 1))
    entry = make_collision(CollisionSphere(0, 0, 0, 3), CollisionParabola(parabola, 1, 2))[0]
    assert entry is None
