from panda3d.core import Triangulator


def triangulate(vertices):
    t = Triangulator()
    for i, v in enumerate(vertices):
        t.add_vertex(v)
        t.add_polygon_vertex(i)

    t.triangulate()

    # Make sure that the result is consistent by starting each triangle with
    # the lowest index value.  That makes it easier to use predetermined values
    # in the test cases.
    result = set()

    for n in range(t.get_num_triangles()):
        # Switch to lowest matching index value in case of duplicates.
        v0 = vertices.index(vertices[t.get_triangle_v0(n)])
        v1 = vertices.index(vertices[t.get_triangle_v1(n)])
        v2 = vertices.index(vertices[t.get_triangle_v2(n)])
        if v1 < v0:
            v0, v1, v2 = v1, v2, v0
        if v1 < v0:
            v0, v1, v2 = v1, v2, v0
        result.add((v0, v1, v2))

    return result


def test_triangulator_degenerate():
    assert not triangulate([])
    assert not triangulate([(0, 0)])
    assert not triangulate([(0, 0), (0, 0)])
    assert not triangulate([(0, 0), (1, 0)])
    assert not triangulate([(0, 0), (0, 0), (0, 0)])
    assert not triangulate([(0, 0), (1, 0), (1, 0)])
    assert not triangulate([(1, 0), (1, 0), (1, 0)])
    assert not triangulate([(1, 0), (0, 0), (1, 0)])
    assert not triangulate([(0, 0), (0, 0), (0, 0), (0, 0)])


def test_triangulator_triangle():
    assert triangulate([(0, 0), (1, 0), (1, 1)]) == {(0, 1, 2)}


def test_triangulator_tail():
    # This triangle has a long "tail" where the polygon retraces its vertices.
    assert triangulate([
        (0, -1),
        (0, 1),
        (1, 0),
        (2, 0),
        (3, 1),
        (4, 0),
        (5, 0),
        (4, 0),
        (3, 1),
        (2, 0),
        (1, 0),
    ]) == {(0, 2, 1)}


def test_triangulator_hourglass():
    # Two triangles with touching tips, effectively.
    assert triangulate([
        (-1, 1),
        (-1, -1),
        (0, 0),
        (1, -1),
        (1, 1),
        (0, 0),
    ]) == {(0, 1, 2), (2, 3, 4)}
