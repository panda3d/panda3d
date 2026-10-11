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


@pytest.fixture(scope='session')
def light_attrib():
    light = core.AmbientLight('amb')
    light.color = (1, 1, 1, 1)
    light_attrib = core.LightAttrib.make()
    light_attrib = light_attrib.add_on_light(core.NodePath(light))

    return light_attrib


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
        core.WindowProperties.size(8, 8),
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
        core.WindowProperties.size(8, 8),
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


def render_color_pixel(region, state, vertex_color=None, clear_color=(0, 0, 0, 1)):
    """Renders a fragment using the specified render settings, and returns the
    resulting color value."""

    # Skip auto-shader tests if we don't support Cg shaders.
    if not region.window.gsg.supports_basic_shaders:
        sattr = state.get_attrib(core.ShaderAttrib)
        if sattr and sattr.auto_shader():
            pytest.skip("Cannot test auto-shader without Cg shader support")

    # Set up the scene with a blank triangle rendering at specified distance.
    scene = core.NodePath("root")
    scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))

    camera = scene.attach_new_node(core.Camera("camera"))
    camera.node().get_lens(0).set_near_far(1, 3)
    camera.node().set_cull_bounds(core.OmniBoundingVolume())

    if vertex_color is not None:
        format = core.GeomVertexFormat.get_v3cp()
    else:
        format = core.GeomVertexFormat.get_v3()

    vdata = core.GeomVertexData("triangle", format, core.Geom.UH_static)
    vdata.unclean_set_num_rows(3)

    vertex = core.GeomVertexWriter(vdata, "vertex")
    vertex.set_data3(core.Vec3.rfu(-1, 0, 1))
    vertex.set_data3(core.Vec3.rfu(-1, 0, -3))
    vertex.set_data3(core.Vec3.rfu(3, 0, 1))

    if vertex_color is not None:
        color = core.GeomVertexWriter(vdata, "color")
        color.set_data4(vertex_color)
        color.set_data4(vertex_color)
        color.set_data4(vertex_color)

    strip = core.GeomTriangles(core.Geom.UH_static)
    strip.set_shade_model(core.Geom.SM_uniform)
    strip.add_next_vertices(3)

    geom = core.Geom(vdata)
    geom.add_primitive(strip)

    gnode = core.GeomNode("triangle")
    gnode.add_geom(geom, state)
    triangle = scene.attach_new_node(gnode)
    triangle.set_pos(0, 2, 0)

    region.active = True
    region.camera = camera

    region.window.set_clear_color(clear_color)
    try:
        color_texture = core.Texture("color")
        region.window.add_render_texture(color_texture,
                                         core.GraphicsOutput.RTM_copy_ram,
                                         core.GraphicsOutput.RTP_color)

        region.window.engine.render_frame()
    finally:
        region.window.clear_render_textures()
        region.window.set_clear_color((0, 0, 0, 1))

    col = core.LColor()
    color_texture.peek().lookup(col, 0.5, 0.5)
    return col


def test_color_write_mask(color_region):
    state = core.RenderState.make(
        core.ColorWriteAttrib.make(core.ColorWriteAttrib.C_green),
    )
    result = render_color_pixel(color_region, state)
    assert result == (0, 1, 0, 1)


OP_NAMES = ['zero', 'one',
    'incoming_color', 'one_minus_incoming_color',
    'fbuffer_color', 'one_minus_fbuffer_color',
    'incoming_alpha', 'one_minus_incoming_alpha',
    'fbuffer_alpha', 'one_minus_fbuffer_alpha',
    'constant_color', 'one_minus_constant_color',
    'constant_alpha', 'one_minus_constant_alpha'
]
@pytest.mark.parametrize('op_name_a', OP_NAMES)
@pytest.mark.parametrize('op_name_b', OP_NAMES)
def test_color_blend_add(color_region, op_name_a, op_name_b):
    fbuffer = core.LColor(0.2, 0.4, 0.6, 0.8)
    incoming = core.LColor(0.3, 0.5, 0.7, 0.1)
    const = core.LColor(0.0, 1.0, 0.5, 0.25)

    op_a = getattr(core.ColorBlendAttrib, 'O_' + op_name_a)
    op_b = getattr(core.ColorBlendAttrib, 'O_' + op_name_b)
    state = core.RenderState.make(
        core.ColorAttrib.make_flat(incoming),
        core.ColorBlendAttrib.make(core.ColorBlendAttrib.M_add, op_a, op_b, const),
    )

    # Calculate what it should be.
    def calc_op(op):
        if op == core.ColorBlendAttrib.O_zero:
            return core.LColor(0.0)
        if op == core.ColorBlendAttrib.O_one:
            return core.LColor(1.0)
        if op == core.ColorBlendAttrib.O_incoming_color:
            return incoming
        if op == core.ColorBlendAttrib.O_one_minus_incoming_color:
            return core.LColor(1.0) - incoming
        if op == core.ColorBlendAttrib.O_fbuffer_color:
            return fbuffer
        if op == core.ColorBlendAttrib.O_one_minus_fbuffer_color:
            return core.LColor(1.0) - fbuffer
        if op == core.ColorBlendAttrib.O_incoming_alpha:
            return core.LColor(incoming.w)
        if op == core.ColorBlendAttrib.O_one_minus_incoming_alpha:
            return core.LColor(1.0 - incoming.w)
        if op == core.ColorBlendAttrib.O_fbuffer_alpha:
            return core.LColor(fbuffer.w)
        if op == core.ColorBlendAttrib.O_one_minus_fbuffer_alpha:
            return core.LColor(1.0 - fbuffer.w)
        if op == core.ColorBlendAttrib.O_constant_color:
            return const
        if op == core.ColorBlendAttrib.O_one_minus_constant_color:
            return core.LColor(1.0) - const
        if op == core.ColorBlendAttrib.O_constant_alpha:
            return core.LColor(const.w)
        if op == core.ColorBlendAttrib.O_one_minus_constant_alpha:
            return core.LColor(1.0 - const.w)

    term_a = core.LColor(incoming)
    term_a.componentwise_mult(calc_op(op_a))
    term_b = core.LColor(fbuffer)
    term_b.componentwise_mult(calc_op(op_b))
    expected = term_a + term_b
    expected = expected.fmax(core.LColor(0)).fmin(core.LColor(1))

    result = render_color_pixel(color_region, state, clear_color=fbuffer)
    assert result.almost_equal(expected, FUZZ)


@pytest.mark.parametrize('rgb_mode', ['min', 'max'])
@pytest.mark.parametrize('alpha_mode', ['min', 'max'])
def test_color_blend_min_max(color_region, rgb_mode, alpha_mode):
    fbuffer = core.LColor(0.2, 0.5, 0.6, 0.8)
    incoming = core.LColor(0.3, 0.4, 0.7, 0.1)

    # Note that operands are ignored for M_min and M_max
    state = core.RenderState.make(
        core.ColorAttrib.make_flat(incoming),
        core.ColorBlendAttrib.make(
            getattr(core.ColorBlendAttrib, 'M_' + rgb_mode),
            core.ColorBlendAttrib.O_one,
            core.ColorBlendAttrib.O_one,
            getattr(core.ColorBlendAttrib, 'M_' + alpha_mode),
            core.ColorBlendAttrib.O_one,
            core.ColorBlendAttrib.O_one,
        ),
    )

    if rgb_mode == 'min':
        expected = fbuffer.fmin(incoming)
    elif rgb_mode == 'max':
        expected = fbuffer.fmax(incoming)

    if rgb_mode != alpha_mode:
        if alpha_mode == 'min':
            expected.w = min(fbuffer.w, incoming.w)
        elif alpha_mode == 'max':
            expected.w = max(fbuffer.w, incoming.w)

    result = render_color_pixel(color_region, state, clear_color=fbuffer)
    assert result.almost_equal(expected, FUZZ)


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


def test_color_transparency(color_region, shader_attrib, light_attrib):
    mat = core.Material()
    mat.diffuse = (1, 1, 1, 0.75)
    material_attrib = core.MaterialAttrib.make(mat)

    state = core.RenderState.make(
        core.TransparencyAttrib.make(core.TransparencyAttrib.M_alpha),
        light_attrib,
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result.x == pytest.approx(0.75, 0.1)
    assert result.w == pytest.approx(1.0, 0.1)


def test_color_transparency_flat(color_region, shader_attrib, light_attrib):
    mat = core.Material()
    mat.diffuse = (1, 1, 1, 0.75)
    material_attrib = core.MaterialAttrib.make(mat)

    state = core.RenderState.make(
        core.TransparencyAttrib.make(core.TransparencyAttrib.M_alpha),
        core.ColorAttrib.make_flat(TEST_COLOR),
        light_attrib,
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result.x == pytest.approx(0.75, 0.1)
    assert result.w == pytest.approx(1.0, 0.1)


def test_color_transparency_vertex(color_region, shader_attrib, light_attrib):
    mat = core.Material()
    mat.diffuse = (1, 1, 1, 0.75)
    material_attrib = core.MaterialAttrib.make(mat)

    state = core.RenderState.make(
        core.TransparencyAttrib.make(core.TransparencyAttrib.M_alpha),
        core.ColorAttrib.make_vertex(),
        light_attrib,
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state, vertex_color=(1, 1, 1, 0.5))
    assert result.x == pytest.approx(0.75, 0.1)
    assert result.w == pytest.approx(1.0, 0.1)


def test_color_transparency_no_light(color_region, shader_attrib):
    mat = core.Material()
    mat.diffuse = (1, 1, 1, 0.75)
    material_attrib = core.MaterialAttrib.make(mat)

    state = core.RenderState.make(
        core.TransparencyAttrib.make(core.TransparencyAttrib.M_alpha),
        shader_attrib,
        material_attrib,
    )
    result = render_color_pixel(color_region, state)
    assert result.x == pytest.approx(1.0, 0.1)
    assert result.w == pytest.approx(1.0, 0.1)


def test_texture_occlusion(color_region):
    shader_attrib = core.ShaderAttrib.make_default().set_shader_auto(True)

    tex = core.Texture("occlusion")
    tex.set_clear_color((0.5, 1.0, 1.0, 1.0))
    stage = core.TextureStage("occlusion")
    stage.set_mode(core.TextureStage.M_occlusion)
    texture_attrib = core.TextureAttrib.make().add_on_stage(stage, tex)

    mat = core.Material()
    mat.diffuse = (0, 1, 0, 1)
    mat.ambient = (0, 0, 1, 1)
    material_attrib = core.MaterialAttrib.make(mat)

    alight = core.AmbientLight("ambient")
    alight.set_color((0, 0, 1, 1))
    light_attrib = core.LightAttrib.make().add_on_light(core.NodePath(alight))

    state = core.RenderState.make(
        shader_attrib,
        material_attrib,
        texture_attrib,
        light_attrib
    )
    result = render_color_pixel(color_region, state)
    assert result.z == pytest.approx(0.5, 0.05)
