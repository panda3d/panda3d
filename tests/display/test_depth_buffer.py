from panda3d import core
import pytest


@pytest.fixture(scope='module', params=[32, 24, 16])
def depth_region(request, graphics_pipe):
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
    fbprops.depth_bits = request.param

    if fbprops.depth_bits >= 32:
        fbprops.float_depth = True

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
        pytest.skip("Cannot make depth buffer")

    if buffer.get_fb_properties().depth_bits != request.param:
        pytest.skip("Could not make buffer with desired bit count")

    yield buffer.make_display_region()

    if buffer is not None:
        engine.remove_window(buffer)


def render_depth_pixel(region, distance, near, far, clear=None, write=True):
    """Renders a fragment at the specified distance using the specified render
    settings, and returns the resulting depth value."""

    # Set up the scene with a blank card rendering at specified distance.
    scene = core.NodePath("root")
    scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))
    scene.set_depth_write(write)

    camera = scene.attach_new_node(core.Camera("camera"))
    camera.node().get_lens(0).set_near_far(near, far)
    camera.node().set_cull_bounds(core.OmniBoundingVolume())

    if distance is not None:
        cm = core.CardMaker("card")
        cm.set_frame(-1, 1, -1, 1)
        card = scene.attach_new_node(cm.generate())
        card.set_pos(0, distance, 0)
        card.set_scale(60)

    region.active = True
    region.camera = camera

    if clear is not None:
        region.set_clear_depth_active(True)
        region.set_clear_depth(clear)

    depth_texture = core.Texture("depth")
    region.window.add_render_texture(depth_texture,
                                     core.GraphicsOutput.RTM_copy_ram,
                                     core.GraphicsOutput.RTP_depth)

    region.window.engine.render_frame()
    region.window.clear_render_textures()

    col = core.LColor()
    depth_texture.peek().lookup(col, 0.5, 0.5)
    return col[0]


def test_depth_clear(depth_region):
    assert 1.0 == render_depth_pixel(depth_region, None, near=1, far=10, clear=1.0)
    assert 0.0 == render_depth_pixel(depth_region, None, near=1, far=10, clear=0.0)


def test_depth_write(depth_region):
    assert 1.0 == render_depth_pixel(depth_region, 5.0, near=1, far=10, clear=1.0, write=False)
    assert 0.99 > render_depth_pixel(depth_region, 5.0, near=1, far=10, clear=1.0, write=True)


def test_depth_far_inf(depth_region):
    inf = float("inf")
    assert 0.99 > render_depth_pixel(depth_region, 10.0, near=1, far=inf, clear=1.0)


def test_depth_near_inf(depth_region):
    inf = float("inf")
    assert 0.01 < render_depth_pixel(depth_region, 10.0, near=inf, far=1, clear=0.0)


def test_depth_clipping(depth_region):
    # Get the actual depth resulting from the clear value.
    clr = render_depth_pixel(depth_region, None, near=1, far=10, clear=0.5)

    # We try rendering something at various distances to make sure that the
    # resulting depth value matches our expectations.

    # Too close; read clear value.
    assert clr == render_depth_pixel(depth_region, 0.999, near=1, far=10, clear=0.5)

    # Too far; read clear value.
    assert clr == render_depth_pixel(depth_region, 10.01, near=1, far=10, clear=0.5)

    # Just close enough; read a value close to 0.0.
    assert 0.01 > render_depth_pixel(depth_region, 1.001, near=1, far=10, clear=0.5)

    # Just far enough; read 1.0.
    assert 0.99 < render_depth_pixel(depth_region, 9.999, near=1, far=10, clear=0.5)


def test_inverted_depth_clipping(depth_region):
    # Get the actual depth resulting from the clear value.
    clr = render_depth_pixel(depth_region, None, near=1, far=10, clear=0.5)

    # Too close; read clear value.
    assert clr == render_depth_pixel(depth_region, 0.999, near=10, far=1, clear=0.5)

    # Too far; read clear value.
    assert clr == render_depth_pixel(depth_region, 10.01, near=10, far=1, clear=0.5)

    # Just close enough; read a value close to 1.0.
    assert 0.99 < render_depth_pixel(depth_region, 1.001, near=10, far=1, clear=0.5)

    # Just far enough; read a value close to 0.0.
    assert 0.01 > render_depth_pixel(depth_region, 9.999, near=10, far=1, clear=0.5)
