from panda3d import core
import pytest

TEST_COLOR = core.LColor(1, 127/255.0, 0, 127/255.0)
TEST_COLOR_SCALE = core.LVecBase4(0.5, 0.5, 0.5, 0.5)
TEST_SCALED_COLOR = core.LColor(TEST_COLOR)
TEST_SCALED_COLOR.componentwise_mult(TEST_COLOR_SCALE)
FUZZ = 0.02


@pytest.fixture(scope='session', params=[False, True], ids=["shader:off", "shader:auto"])
def shader_attrib(request):
    """Returns two ShaderAttribs: one with auto shader, one without."""
    if request.param:
        return core.ShaderAttrib.make_default().set_shader_auto(True)
    else:
        return core.ShaderAttrib.make_off()


@pytest.fixture(scope='session', params=["mat:off", "mat:empty", "mat:amb", "mat:diff", "mat:both"])
def material_attrib(request):
    """Returns two MaterialAttribs: one with material, one without.  It
    shouldn't really matter what we set them to, since the tests in here do
    not use lighting, and therefore the material should be ignored."""

    if request.param == "mat:off":
        return core.MaterialAttrib.make_off()

    elif request.param == "mat:empty":
        return core.MaterialAttrib.make(core.Material())

    elif request.param == "mat:amb":
        mat = core.Material()
        mat.ambient = (0.1, 1, 0.5, 1)
        return core.MaterialAttrib.make(mat)

    elif request.param == "mat:diff":
        mat = core.Material()
        mat.diffuse = (0.1, 1, 0.5, 1)
        return core.MaterialAttrib.make(mat)

    elif request.param == "mat:both":
        mat = core.Material()
        mat.diffuse = (0.1, 1, 0.5, 1)
        mat.ambient = (0.1, 1, 0.5, 1)
        return core.MaterialAttrib.make(mat)


@pytest.fixture(scope='module', params=[False, True], ids=["srgb:off", "srgb:on"])
def color_region(request, graphics_pipe):
    """Creates and returns a DisplayRegion with a depth buffer."""

    engine = core.GraphicsEngine()
    engine.set_threading_model("")

    host_fbprops = core.FrameBufferProperties()
    host_fbprops.force_hardware = True

    host = engine.make_output(
        graphics_pipe,
        'host',
        0,
        host_fbprops,
        core.WindowProperties.size(32, 32),
        core.GraphicsPipe.BF_refuse_window,
    )
    engine.open_windows()

    if host is None:
        pytest.skip("GraphicsPipe cannot make offscreen buffers")

    fbprops = core.FrameBufferProperties()
    fbprops.force_hardware = True
    fbprops.set_rgba_bits(8, 8, 8, 8)
    fbprops.srgb_color = request.param

    buffer = engine.make_output(
        graphics_pipe,
        'buffer',
        0,
        fbprops,
        core.WindowProperties.size(32, 32),
        core.GraphicsPipe.BF_refuse_window,
        host.gsg,
        host
    )
    engine.open_windows()

    if buffer is None:
        pytest.skip("Cannot make color buffer")

    if fbprops.srgb_color != buffer.get_fb_properties().srgb_color:
        pytest.skip("Cannot make buffer with required srgb_color setting")

    buffer.set_clear_color_active(True)
    buffer.set_clear_color((0, 0, 0, 1))

    yield buffer.make_display_region()

    if buffer is not None:
        engine.remove_window(buffer)


def render_color_pixel(region, state, vertex_color=None):
    """Renders a fragment using the specified render settings, and returns the
    resulting color value."""

    # Skip auto-shader tests if we don't support Cg shaders.
    if not region.window.gsg.supports_basic_shaders:
        sattr = state.get_attrib(core.ShaderAttrib)
        if sattr and sattr.auto_shader():
            pytest.skip("Cannot test auto-shader without Cg shader support")

    # Set up the scene with a blank card rendering at specified distance.
    scene = core.NodePath("root")
    scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))

    camera = scene.attach_new_node(core.Camera("camera"))
    camera.node().get_lens(0).set_near_far(1, 3)
    camera.node().set_cull_bounds(core.OmniBoundingVolume())

    if vertex_color is not None:
        format = core.GeomVertexFormat.get_v3cp()
    else:
        format = core.GeomVertexFormat.get_v3()

    vdata = core.GeomVertexData("card", format, core.Geom.UH_static)
    vdata.unclean_set_num_rows(4)

    vertex = core.GeomVertexWriter(vdata, "vertex")
    vertex.set_data3(core.Vec3.rfu(-1, 0, 1))
    vertex.set_data3(core.Vec3.rfu(-1, 0, -1))
    vertex.set_data3(core.Vec3.rfu(1, 0, 1))
    vertex.set_data3(core.Vec3.rfu(1, 0, -1))

    if vertex_color is not None:
        color = core.GeomVertexWriter(vdata, "color")
        color.set_data4(vertex_color)
        color.set_data4(vertex_color)
        color.set_data4(vertex_color)
        color.set_data4(vertex_color)

    strip = core.GeomTristrips(core.Geom.UH_static)
    strip.set_shade_model(core.Geom.SM_uniform)
    strip.add_next_vertices(4)
    strip.close_primitive()

    geom = core.Geom(vdata)
    geom.add_primitive(strip)

    gnode = core.GeomNode("card")
    gnode.add_geom(geom, state)
    card = scene.attach_new_node(gnode)
    card.set_pos(0, 2, 0)
    card.set_scale(60)

    region.active = True
    region.camera = camera

    color_texture = core.Texture("color")
    region.window.add_render_texture(color_texture,
                                     core.GraphicsOutput.RTM_copy_ram,
                                     core.GraphicsOutput.RTP_color)

    region.window.engine.render_frame()
    region.window.clear_render_textures()

    col = core.LColor()
    color_texture.peek().lookup(col, 0.5, 0.5)
    return col


def test_color_write_mask(color_region):
    state = core.RenderState.make(
        core.ColorWriteAttrib.make(core.ColorWriteAttrib.C_green),
    )
    result = render_color_pixel(color_region, state)
    assert result == (0, 1, 0, 1)


def test_color_empty(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result == (1, 1, 1, 1)


def test_color_off(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_off(),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result == (1, 1, 1, 1)


def test_color_flat(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_flat(TEST_COLOR),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result.almost_equal(TEST_COLOR, FUZZ)


def test_color_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_vertex(),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result.almost_equal(TEST_COLOR, FUZZ)


def test_color_empty_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result.almost_equal(TEST_COLOR, FUZZ)


def test_color_off_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_off(),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result == (1, 1, 1, 1)


def test_scaled_color_empty(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result == (1, 1, 1, 1)


def test_scaled_color_off(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_off(),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result == (1, 1, 1, 1)


def test_scaled_color_flat(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_flat(TEST_COLOR),
        core.ColorScaleAttrib.make(TEST_COLOR_SCALE),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result.almost_equal(TEST_SCALED_COLOR, FUZZ)


def test_scaled_color_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_vertex(),
        core.ColorScaleAttrib.make(TEST_COLOR_SCALE),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result.almost_equal(TEST_SCALED_COLOR, FUZZ)


def test_scaled_color_empty_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorScaleAttrib.make(TEST_COLOR_SCALE),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result.almost_equal(TEST_SCALED_COLOR, FUZZ)


def test_scaled_color_off_vertex(color_region, shader_attrib, material_attrib):
    state = core.RenderState.make(
        core.ColorAttrib.make_off(),
        core.ColorScaleAttrib.make(TEST_COLOR_SCALE),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=TEST_COLOR)
    assert result.almost_equal(TEST_COLOR_SCALE, FUZZ)

