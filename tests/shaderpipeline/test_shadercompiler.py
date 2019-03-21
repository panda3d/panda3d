from panda3d import core

GLSL_VERT_SHADER = ""

def test_shadercompiler_glsl(registry):
    compiler = registry.get_compiler_from_language(core.Shader.SL_GLSL)
    module = compiler.compile_now(core.ShaderModule.Stage.vertex, core.StringStream(GLSL_VERT_SHADER.encode('utf-8')))
    assert module.stage == core.ShaderModule.Stage.vertex
    assert module.get_ir() == GLSL_VERT_SHADER
