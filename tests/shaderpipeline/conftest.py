import pytest

@pytest.fixture
def registry():
    from panda3d.core import ShaderCompilerRegistry
    return ShaderCompilerRegistry.get_global_ptr()

