from panda3d import core
import pytest
import tempfile


def write_image(filename, channels):
    img = core.PNMImage(1, 1, channels)
    img.set_xel_a(0, 0, (0.0, 0.25, 0.5, 0.75))
    assert img.write(filename)


def yield_image(suffix, channels):
    file = tempfile.NamedTemporaryFile(suffix=suffix)
    path = core.Filename.from_os_specific(file.name)
    path.make_true_case()
    write_image(path, channels)
    yield path
    file.close()


def register_filter(pool, tex_filter):
    assert pool.get_num_filters() == 0
    assert pool.register_filter(tex_filter)
    assert pool.get_num_filters() == 1


def yield_registered_filter(filter_type):
    tex_filter = filter_type()
    yield tex_filter

    p = core.TexturePool.get_global_ptr()

    if p.is_filter_registered(tex_filter):
        p.unregister_filter(tex_filter)


@pytest.fixture(scope='function')
def pool():
    "This fixture ensures the pool is properly emptied"
    pool = core.TexturePool.get_global_ptr()
    pool.release_all_textures()
    yield pool
    pool.release_all_textures()


@pytest.fixture(scope='session')
def image_gray_path():
    "Generates a grayscale image."
    yield from yield_image('.bw', channels=1)


@pytest.fixture(scope='session')
def image_rgb_path():
    "Generates an RGB image."
    yield from yield_image('.rgb', channels=3)


@pytest.fixture(scope='session')
def image_rgba_path():
    "Generates an RGBA image."
    yield from yield_image('.rgba', channels=4)


@pytest.fixture(scope='function')
def pre_filter():
    "Creates a texture pool preload filter."
    class PreLoadTextureFilter(object):

        def pre_load(self, orig_filename, orig_alpha_filename,
                     primary_file_num_channels, alpha_file_channel,
                     read_mipmaps, options):
            return core.Texture('preloaded')

    yield from yield_registered_filter(PreLoadTextureFilter)


@pytest.fixture(scope='function')
def post_filter():
    "Creates a texture pool postload filter."
    class PostLoadTextureFilter(object):

        def post_load(self, tex):
            tex.set_name('postloaded')
            return tex

    yield from yield_registered_filter(PostLoadTextureFilter)


@pytest.fixture(scope='function')
def mix_filter():
    "Creates a texture pool mix filter."
    class MixTextureFilter(object):

        def pre_load(self, orig_filename, orig_alpha_filename,
                     primary_file_num_channels, alpha_file_channel,
                     read_mipmaps, options):
            return core.Texture('preloaded')

        def post_load(self, tex):
            tex.set_name(tex.get_name() + '-postloaded')
            return tex

    yield from yield_registered_filter(MixTextureFilter)


@pytest.fixture(scope='function')
def invalid_filter():
    "Creates an invalid texture filter."
    class InvalidTextureFilter(object):
        pass

    tex_filter = InvalidTextureFilter()
    yield tex_filter


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


def test_reuse_texture(pool, image_rgba_path):
    tex1 = pool.load_texture(image_rgba_path)
    tex2 = pool.load_texture(image_rgba_path)
    assert tex1 == tex2


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
    tex1 = pool.load_texture(image_rgb_path, image_gray_path)
    assert tex1.num_components == 4

    tex2 = pool.load_texture(image_rgb_path)
    assert tex2.num_components == 3

    assert tex1.num_components == 4
    assert tex1 != tex2


def test_reload_texture_different_sampler(pool, image_rgb_path):
    sampler = core.SamplerState()
    sampler.wrap_u = core.Texture.WM_clamp
    tex1 = pool.load_texture(image_rgb_path, 0, False, core.LoaderOptions(), sampler)
    assert tex1.wrap_u == core.Texture.WM_clamp

    sampler = core.SamplerState()
    sampler.wrap_u = core.Texture.WM_repeat
    tex2 = pool.load_texture(image_rgb_path, 0, False, core.LoaderOptions(), sampler)
    assert tex2.wrap_u == core.Texture.WM_repeat

    assert tex1.wrap_u == core.Texture.WM_clamp
    assert tex1 != tex2


def test_reload_texture_with_force_srgb(pool, image_rgb_path):
    tex1 = pool.load_texture(image_rgb_path)
    assert tex1.format == core.Texture.F_rgb

    options = core.LoaderOptions()
    options.set_texture_flags(options.get_texture_flags() | core.LoaderOptions.TF_force_srgb)
    tex2 = pool.load_texture(image_rgb_path, 0, False, options)
    assert tex2.format == core.Texture.F_srgb

    assert tex1.format == core.Texture.F_rgb
    assert tex1 != tex2


def test_reload_texture_with_format(pool, image_rgb_path):
    tex1 = pool.load_texture(image_rgb_path)
    assert tex1.format == core.Texture.F_rgb

    options = core.LoaderOptions()
    options.set_texture_format(core.Texture.F_rgb5)
    tex2 = pool.load_texture(image_rgb_path, 0, False, options)
    assert tex2.format == core.Texture.F_rgb5

    assert tex1.format == core.Texture.F_rgb
    assert tex1 != tex2


def test_empty_texture_filters(pool):
    assert pool.get_num_filters() == 0


def test_register_pre_texture_filter(pool, pre_filter):
    register_filter(pool, pre_filter)


def test_register_post_texture_filter(pool, post_filter):
    register_filter(pool, post_filter)


def test_register_mix_texture_filter(pool, mix_filter):
    register_filter(pool, mix_filter)


def test_register_invalid_texture_filter(pool, invalid_filter):
    assert pool.get_num_filters() == 0

    with pytest.raises(TypeError):
        pool.register_filter(invalid_filter)

    assert pool.get_num_filters() == 0


def test_register_null_texture_filter(pool):
    assert pool.get_num_filters() == 0

    with pytest.raises(TypeError):
        pool.register_filter(None)

    assert pool.get_num_filters() == 0


def test_register_all_texture_filters(pool, pre_filter, post_filter, mix_filter):
    assert pool.get_num_filters() == 0
    assert pool.register_filter(pre_filter)
    assert pool.register_filter(post_filter)
    assert pool.register_filter(mix_filter)
    assert pool.get_num_filters() == 3


def test_unregister_texture_filter(pool, mix_filter):
    register_filter(pool, mix_filter)
    assert pool.unregister_filter(mix_filter)
    assert pool.get_num_filters() == 0


def test_clear_texture_filters(pool, pre_filter, post_filter):
    assert pool.get_num_filters() == 0
    assert pool.register_filter(pre_filter)
    assert pool.register_filter(post_filter)
    assert pool.get_num_filters() == 2

    pool.clear_filters()
    assert pool.get_num_filters() == 0


def test_double_register_texture_filter(pool, mix_filter):
    register_filter(pool, mix_filter)
    assert not pool.register_filter(mix_filter)
    assert pool.get_num_filters() == 1


def test_double_unregister_texture_filter(pool, mix_filter):
    register_filter(pool, mix_filter)
    assert pool.unregister_filter(mix_filter)
    assert not pool.unregister_filter(mix_filter)
    assert pool.get_num_filters() == 0


def test_is_texture_filter_registered(pool, pre_filter, mix_filter):
    assert not pool.is_filter_registered(mix_filter)
    assert pool.register_filter(mix_filter)
    assert pool.is_filter_registered(mix_filter)
    assert not pool.is_filter_registered(pre_filter)


def test_get_texture_filter(pool, pre_filter):
    assert not pool.get_filter(0)

    assert pool.register_filter(pre_filter)
    tex_filter = pool.get_filter(0)
    assert isinstance(tex_filter, core.TexturePoolFilter)

    assert not pool.get_filter(1)


def test_texture_pre_filter(pool, pre_filter):
    register_filter(pool, pre_filter)

    texture = pool.load_texture('nonexistent')
    assert isinstance(texture, core.Texture)
    assert texture.get_name() == 'preloaded'


def test_texture_post_filter(pool, post_filter, image_rgb_path):
    register_filter(pool, post_filter)

    texture = pool.load_texture(image_rgb_path, 3)
    assert isinstance(texture, core.Texture)
    assert texture.get_name() == 'postloaded'


def test_texture_mix_filter(pool, mix_filter):
    register_filter(pool, mix_filter)

    texture = pool.load_texture('nonexistent')
    assert isinstance(texture, core.Texture)
    assert texture.get_name() == 'preloaded-postloaded'


def test_no_texture_filter_option(pool, pre_filter, image_rgb_path):
    register_filter(pool, pre_filter)

    texture = pool.load_texture(image_rgb_path, 3, False, core.LoaderOptions(0, core.LoaderOptions.TF_no_filters))
    assert isinstance(texture, core.Texture)
    assert texture.get_name() != 'preloaded'
