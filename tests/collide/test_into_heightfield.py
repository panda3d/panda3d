import pytest
from collisions import *
from panda3d.core import PNMImage

def test_sphere_into_heightfield():
    # Setup PNMImage
    img = PNMImage(512, 512, 1)
    img.set_gray_val(1, 1, 255)
    # Make CollisionHeightfield
    max_height = 10
    num_subdivisions = 1
    heightfield = CollisionHeightfield(img, max_height, num_subdivisions)
    # The coordinate (1, 1) on our heightfield image
    # maps to the coordinate (1, 510, Z) in 3D space
    sphere = CollisionSphere((1, 510, 11), 1)
    entry, np_from, np_into = make_collision(sphere, heightfield)
    assert entry.get_surface_point(np_from) == (1, 510, 10)
    assert entry.get_surface_normal(np_from) == (0, 0, 1)
    # Set the sphere higher so it is not colliding anymore
    sphere.set_center((1, 510, 11.1))
    entry = make_collision(sphere, heightfield)[0]
    assert entry is None
    # Set the max_height to re-collide with the sphere
    max_height = 10.1
    heightfield.set_max_height(max_height)
    entry, np_from, np_into = make_collision(sphere, heightfield)
    assert entry.get_surface_point(np_from) == (1, 510, 10.1)

    #with pytest.raises(AssertionError) as err:
    #    assert heightfield.set_num_subdivisions(-1) == err
    #    assert heightfield.set_num_subdivisions(11) == err

    # Use a greater number of subdivisions, should still work
    num_subdivisions = 10
    heightfield.set_num_subdivisions(num_subdivisions)
    entry, np_from, np_into = make_collision(sphere, heightfield)
    assert entry.get_surface_point(np_from) == (1, 510, 10.1)
    # Using 10 subdivisions is overkill for such a small heightfield,
    # CollisionHeightfield should've automatically decreased it
    assert heightfield.get_num_subdivisions() < num_subdivisions
    # Zero subdivisions should work too
    num_subdivisions = 0
    heightfield.set_num_subdivisions(num_subdivisions)
    entry, np_from, np_into = make_collision(sphere, heightfield)
    assert entry.get_surface_point(np_from) == (1, 510, 10.1)
    # Modify the heightfield, no longer colliding
    img.set_gray_val(1, 1, 254)
    heightfield.set_heightfield(img)
    entry = make_collision(sphere, heightfield)[0]
    assert entry is None


def test_ray_into_heightfield():
    # Setup heightfield
    img = PNMImage(127, 127, 1)
    img.fill_val(0)
    max_height = 10
    num_subdivisions = 1
    heightfield = CollisionHeightfield(img, max_height, num_subdivisions)
    # Make ray
    ray = CollisionRay((100, 100, 100), (-1, -1, -1))
    entry = make_collision(ray, heightfield)[0]
    assert entry is not None

    # Ray with only a z component in its direction
    ray.set_direction((0, 0, -5))
    entry, np_from, np_into = make_collision(ray, heightfield)
    assert entry.get_surface_point(np_from) == (100, 100, 0)

    # Set coordinate (54, 38) on heightfield to gray value of 255
    # Note that coordinate (54, 38) on the heightfield
    # maps to (54, 88, Z) in 3D space
    img.set_gray_val(54, 38, 255)
    heightfield.set_heightfield(img)

    ray.set_origin((54, 88, 10))
    entry, np_from, np_into = make_collision(ray, heightfield)
    assert entry.get_surface_point(np_from) == (54, 88, 10)


def test_box_into_heightfield():
    # Try using a large non-square heightfield image
    img = PNMImage(5023, 5130, 1)
    img.set_gray_val(1, 1, 255)
    # Make CollisionHeightfield
    max_height = 10
    num_subdivisions = 5
    heightfield = CollisionHeightfield(img, max_height, num_subdivisions)
    # Make box
    box = CollisionBox((1, 5128, 10), 1, 1, 1)
    entry = make_collision(box, heightfield)
    assert entry is not None
