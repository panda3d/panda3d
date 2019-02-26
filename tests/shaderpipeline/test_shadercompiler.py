from panda3d import core

GLSL_VERT_SHADER = """
"""

def test_shadercompiler_glsl(registry):
    compiler = registry.get_compiler_from_language(core.Shader.SL_GLSL)
    module = compiler.compile_now(core.Shader.ST_vertex, GLSL_VERT_SHADER)
    assert module.get_shader_type() == core.Shader.ST_vertex
    assert module.get_ir() == GLSL_VERT_SHADER
