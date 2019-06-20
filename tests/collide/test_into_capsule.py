from collisions import *

def test_box_into_capsule():
    capsule = CollisionCapsule((1, 0, 0), (-1, 0, 0), .5)

    # colliding just on the capsule's cylinder
    box = CollisionBox((0, 1, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is not None

    # no longer colliding
    box = CollisionBox((0, 1.1, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is None

    # box is inside the capsule's cylinder
    box = CollisionBox((0, .8, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is not None

    # colliding with the first endcap
    box = CollisionBox((2, 0, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is not None

    # almost colliding with first endcap
    box = CollisionBox((2.01, 0, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is None

    # colliding with the second endcap
    box = CollisionBox((-2, 0, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is not None

    # almost colliding with second endcap
    box = CollisionBox((-2.01, 0, 0), .5, .5, .5)
    entry = make_collision(box, capsule)[0]
    assert entry is None
