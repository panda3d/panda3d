from collisions import *
from pytest import approx


def test_parabola_into_invsphere():
    invsphere = CollisionInvSphere(1, 1, 1, 5)
    parabola = CollisionParabola()
    parabola.set_t1(0)
    parabola.set_t2(2)

    # parabola starts from outside the sphere
    parabola.set_parabola(
            LParabola((1, 1, 1), (0, 0, 0), (7, 7, 7)))
    entry, np_from, np_into = make_collision(parabola, invsphere)
    assert (entry.get_surface_point(np_from) -
            invsphere.get_center()).length() == approx(invsphere.get_radius())

    # parabola starts on the sphere
    parabola.set_parabola(
            LParabola((1, 0, 1), (1, 0, 0), (1, 1, 6)))
    entry, np_from, np_into = make_collision(parabola, invsphere)
    assert entry.get_surface_point(np_from) == (1, 1, 6)

    # parabola starts from inside the sphere but doesn't collide
    parabola.set_parabola(
            LParabola((-1, -1, -1), (1, 1, 1), (1, 1, 1)))
    entry = make_collision(parabola, invsphere)[0]
    assert entry is None

    # parabola is inside the sphere and collides on an endpoint
    parabola.set_parabola(
            LParabola((1, 0, 0), (0, 0, 0), (2, 1, 1)))
    entry, np_from, np_into = make_collision(parabola, invsphere)
    assert entry.get_surface_point(np_from) == (6, 1, 1)

    # parabola starts from inside the sphere and collides on its projectile
    parabola.set_t2(3)
    assert parabola.get_parabola().calc_point(2) == (6, 1, 1)
    entry, np_from, np_into = make_collision(parabola, invsphere)
    assert entry.get_surface_point(np_from) == (6, 1, 1)

    parabola.set_parabola(
            LParabola((-1, 0, 0), (-1, 0, 0), (2, 1, 1)))
    assert parabola.get_parabola().calc_point(2) == (-4, 1, 1)
    entry, np_from, np_into = make_collision(parabola, invsphere)
    assert entry.get_surface_point(np_from) == (-4, 1, 1)
