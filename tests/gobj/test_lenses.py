from panda3d.core import PerspectiveLens, Point3, Point2, CS_zup_right


def test_perspectivelens_extrude():
    lens = PerspectiveLens()
    lens.set_fov(90, 90)
    lens.set_near_far(0.5, 100)

    near = Point3()
    far = Point3()

    assert lens.extrude((0, 0), near, far)
    assert near.almost_equal((0, 0.5, 0), 0.001)
    assert far.almost_equal((0, 100, 0), 0.1)

    assert lens.extrude((-1, -1), near, far)
    assert near.almost_equal((-0.5, 0.5, -0.5), 0.001)
    assert far.almost_equal((-100, 100, -100), 0.1)

    assert lens.extrude((1, 0), near, far)
    assert near.almost_equal((0.5, 0.5, 0), 0.001)
    assert far.almost_equal((100, 100, 0), 0.1)


def test_perspectivelens_extrude_depth():
    lens = PerspectiveLens()
    lens.set_fov(90, 90)
    lens.set_near_far(0.5, 100)

    point = Point3()

    assert lens.extrude_depth((0, 0, -1), point)
    assert point.almost_equal((0, 0.5, 0), 0.001)

    assert lens.extrude_depth((0, 0, 1), point)
    assert point.almost_equal((0, 100, 0), 0.001)

    assert lens.extrude_depth((-1, -1, -1), point)
    assert point.almost_equal((-0.5, 0.5, -0.5), 0.001)

    assert lens.extrude_depth((-1, -1, 1), point)
    assert point.almost_equal((-100, 100, -100), 0.1)

    assert lens.extrude_depth((1, 0, -1), point)
    assert point.almost_equal((0.5, 0.5, 0), 0.001)

    assert lens.extrude_depth((1, 0, 1), point)
    assert point.almost_equal((100, 100, 0), 0.1)


def test_perspectivelens_project():
    lens = PerspectiveLens()
    lens.set_fov(90, 90)
    lens.set_near_far(0.5, 100)

    point = Point2()

    assert not lens.project((0, 0, 0), point)
    assert not lens.project((-1, 0.5, 0), point)

    assert lens.project((0, 0.5, 0), point)
    assert point.almost_equal((0, 0), 0.001)

    assert lens.project((0, 100, 0), point)
    assert point.almost_equal((0, 0), 0.001)

    assert lens.project((-0.5, 0.5, -0.5), point)
    assert point.almost_equal((-1, -1), 0.001)

    assert lens.project((-100, 100, -100), point)
    assert point.almost_equal((-1, -1), 0.001)

    assert lens.project((0.5, 0.5, 0), point)
    assert point.almost_equal((1, 0), 0.001)

    assert lens.project((100, 100, 0), point)
    assert point.almost_equal((1, 0), 0.001)


def test_perspectivelens_far_inf():
    lens = PerspectiveLens()
    lens.set_fov(90, 90)
    lens.set_near_far(2, float("inf"))
    lens.coordinate_system = CS_zup_right

    mat = lens.get_projection_mat()
    assert mat[1][2] == 1
    assert mat[3][2] == -4


def test_perspectivelens_near_inf():
    lens = PerspectiveLens()
    lens.set_fov(90, 90)
    lens.set_near_far(float("inf"), 2)
    lens.coordinate_system = CS_zup_right

    mat = lens.get_projection_mat()
    assert mat[1][2] == -1
    assert mat[3][2] == 4
