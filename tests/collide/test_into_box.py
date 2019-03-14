from collisions import *


def test_sphere_into_box():
    sphere = make_sphere("sphere", 0, 0, 0, 3)
    box = make_box("box", (0, 0, 0), 2, 3, 4)

    node1 = sphere[1]
    node2 = box[1]

    entry, np, np2 = make_collision(node1, node2)
    assert entry is not None
    assert entry.get_from() == sphere[0]
    assert entry.get_into() == box[0]
