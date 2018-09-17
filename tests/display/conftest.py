import pytest


@pytest.fixture(scope='session')
def graphics_pipe():
    from panda3d.core import GraphicsPipeSelection

    pipe = GraphicsPipeSelection.get_global_ptr().make_default_pipe()

    if pipe is None or not pipe.is_valid():
        pytest.skip("GraphicsPipe is invalid")

    yield pipe


@pytest.fixture(scope='session')
def graphics_engine():
    from panda3d.core import GraphicsEngine

    engine = GraphicsEngine.get_global_ptr()
    yield engine

    # This causes GraphicsEngine to also terminate the render threads.
    engine.remove_all_windows()


@pytest.fixture
def window(graphics_pipe, graphics_engine):
    from panda3d.core import GraphicsPipe, FrameBufferProperties, WindowProperties

    fbprops = FrameBufferProperties.get_default()
    winprops = WindowProperties.get_default()

    win = graphics_engine.make_output(
        graphics_pipe,
        'window',
        0,
        fbprops,
        winprops,
        GraphicsPipe.BF_require_window
    )
    graphics_engine.open_windows()

    if win is None:
        pytest.skip("GraphicsPipe cannot make windows")

    yield win

    if win is not None:
        graphics_engine.remove_window(win)


@pytest.fixture(scope='module')
def gsg(graphics_pipe, graphics_engine):
    "Returns a windowless GSG that can be used for offscreen rendering."
    from panda3d.core import GraphicsPipe, FrameBufferProperties, WindowProperties

    fbprops = FrameBufferProperties()
    fbprops.force_hardware = True

    buffer = graphics_engine.make_output(
        graphics_pipe,
        'buffer',
        0,
        fbprops,
        WindowProperties.size(32, 32),
        GraphicsPipe.BF_refuse_window
    )
    graphics_engine.open_windows()

    if buffer is None:
        pytest.skip("GraphicsPipe cannot make offscreen buffers")

    yield buffer.gsg

    if buffer is not None:
        graphics_engine.remove_window(buffer)
