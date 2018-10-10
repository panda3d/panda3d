from panda3d import core
import pytest
import tempfile

@pytest.fixture(scope='function')
def pool():
    "This fixture ensures the pool is properly emptied"
    pool = core.TexturePool
    pool.release_all_textures()
    yield pool
    pool.release_all_textures()


def write_image(filename, channels):
    img = core.PNMImage(1, 1, channels)
    img.set_xel_a(0, 0, (0.0, 0.25, 0.5, 0.75))
    assert img.write(filename)


@pytest.fixture(scope='session')
def image_rgb_path():
    "Generates an RGB image."

    file = tempfile.NamedTemporaryFile(suffix='-rgb.png')
    path = core.Filename.from_os_specific(file.name)
    path.make_true_case()
    write_image(path, 3)
    yield path
    file.close()


@pytest.fixture(scope='session')
def image_rgba_path():
    "Generates an RGBA image."

    file = tempfile.NamedTemporaryFile(suffix='-rgba.png')
    path = core.Filename.from_os_specific(file.name)
    path.make_true_case()
    write_image(path, 4)
    yield path
    file.close()


@pytest.fixture(scope='session')
def image_gray_path():
    "Generates a grayscale image."

    file = tempfile.NamedTemporaryFile(suffix='-gray.png')
    path = core.Filename.from_os_specific(file.name)
    path.make_true_case()
    write_image(path, 1)
    yield path
    file.close()


def test_load_texture_rgba(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 4


def test_load_texture_rgba4(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path, 4)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 4


def test_load_texture_rgba3(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path, 3)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 3


def test_load_texture_rgba2(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path, 2)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 2


def test_load_texture_rgba1(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path, 1)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 1


def test_load_texture_rgb(pool, image_rgb_path):
    tex = pool.load_texture(image_rgb_path)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 3


def test_load_texture_rgb4(pool, image_rgb_path):
    # Will not increase this
    tex = pool.load_texture(image_rgb_path, 4)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 3


def test_load_texture_rgb3(pool, image_rgb_path):
    tex = pool.load_texture(image_rgb_path, 3)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 3


def test_load_texture_rgb2(pool, image_rgb_path):
    # Cannot reduce this, since it would add an alpha channel
    tex = pool.load_texture(image_rgb_path, 2)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 3


def test_load_texture_rgb1(pool, image_rgb_path):
    tex = pool.load_texture(image_rgb_path, 1)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 1


def test_load_texture_rgba_alpha(pool, image_rgba_path, image_gray_path):
    tex = pool.load_texture(image_rgba_path, image_gray_path)
    assert tex.num_components == 4


def test_load_texture_rgba4_alpha(pool, image_rgba_path, image_gray_path):
    tex = pool.load_texture(image_rgba_path, image_gray_path, 4)
    assert tex.num_components == 4


def test_load_texture_rgba3_alpha(pool, image_rgba_path, image_gray_path):
    tex = pool.load_texture(image_rgba_path, image_gray_path, 3)
    assert tex.num_components == 4


def test_load_texture_rgba2_alpha(pool, image_rgba_path, image_gray_path):
    #FIXME: why is this not consistent with test_load_texture_rgb2_alpha?
    tex = pool.load_texture(image_rgba_path, image_gray_path, 2)
    assert tex.num_components == 2


def test_load_texture_rgba1_alpha(pool, image_rgba_path, image_gray_path):
    tex = pool.load_texture(image_rgba_path, image_gray_path, 1)
    assert tex.num_components == 2


def test_load_texture_rgb_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path, image_gray_path)
    assert tex.num_components == 4


def test_load_texture_rgb4_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path, image_gray_path, 4)
    assert tex.num_components == 4


def test_load_texture_rgb3_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path, image_gray_path, 3)
    assert tex.num_components == 4


def test_load_texture_rgb2_alpha(pool, image_rgb_path, image_gray_path):
    #FIXME: why is this not consistent with test_load_texture_rgba2_alpha?
    tex = pool.load_texture(image_rgb_path, image_gray_path, 2)
    assert tex.num_components == 4


def test_load_texture_rgb1_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path, image_gray_path, 1)
    assert tex.num_components == 2


def test_reload_texture_fewer_channels(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 4

    tex = pool.load_texture(image_rgba_path, 3)
    assert tex.num_components == 3


def test_reload_texture_more_channels(pool, image_rgba_path):
    tex = pool.load_texture(image_rgba_path, 3)
    assert pool.has_texture(image_rgba_path)
    assert tex.num_components == 3

    tex = pool.load_texture(image_rgba_path)
    assert tex.num_components == 4


def test_reload_texture_with_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path)
    assert pool.has_texture(image_rgb_path)
    assert tex.num_components == 3

    tex = pool.load_texture(image_rgb_path, image_gray_path)
    assert tex.num_components == 4


def test_reload_texture_without_alpha(pool, image_rgb_path, image_gray_path):
    tex = pool.load_texture(image_rgb_path, image_gray_path)
    assert tex.num_components == 4

    tex = pool.load_texture(image_rgb_path)
    assert tex.num_components == 3
