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

    assert win is not None
    yield win

    if win is not None:
        graphics_engine.remove_window(win)
