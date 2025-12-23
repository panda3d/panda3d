from panda3d.core import Shader, ShaderType, CompilerOptions


def compile_parameter(code):
    options = CompilerOptions()
    options.optimize = CompilerOptions.Optimize.NONE

    shader = Shader.make_compute(Shader.SL_GLSL, "#version 430\n" + code + "\nvoid main() {}\n", options)
    module = shader.get_module(Shader.Stage.COMPUTE)
    return module.parameters[0]


def test_glsl_reflect_ssbo():
    param = compile_parameter("""
    struct Thing {
      vec3 pos;
      uint idx;
    };

    layout(std430) buffer dataBuffer {
      Thing things[64];
    };
    """)

    assert isinstance(param.type, ShaderType.StorageBuffer)

    # An SSBO is an opaque reference, does not occupy a size
    assert param.type.size_bytes == 0

    # The contained type does have a size, following std430 rules
    assert param.type.contained_type.size_bytes == 16 * 64
