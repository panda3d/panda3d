import pytest


@pytest.fixture
def registry():
    from panda3d.core import ShaderCompilerRegistry
    return ShaderCompilerRegistry.get_global_ptr()


@pytest.fixture
def compiler_glsl(registry):
    from panda3d.core import Shader
    return registry.get_compiler_for_language(Shader.SL_GLSL)
