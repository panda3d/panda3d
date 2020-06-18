from panda3d import core


def test_shadercompiler_glsl_empty(compiler_glsl):
    assert not compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(b''),
    )


def test_shadercompiler_glsl_simple(compiler_glsl):
    code = """
    #version 330

    void main() {
    }
    """

    module = compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )
    assert module
    assert module.stage == core.ShaderModule.Stage.vertex


def test_shadercompiler_glsl_invalid_version(compiler_glsl):
    code = """
    #version 123

    void main() {
    }
    """

    assert not compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl150_bitcast(compiler_glsl):
    code = """
    #version 150

    in int a;
    out float b;

    void main() {
        b = floatBitsToInt(a);
    }
    """

    assert not compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl150_bitcast_extension1(compiler_glsl):
    code = """
    #version 150
    #extension GL_ARB_shader_bit_encoding : enable

    in int a;
    out float b;

    void main() {
        b = floatBitsToInt(a);
    }
    """

    assert compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl150_bitcast_extension2(compiler_glsl):
    code = """
    #version 150
    #extension GL_ARB_gpu_shader5 : enable

    in int a;
    out float b;

    void main() {
        b = floatBitsToInt(a);
    }
    """

    assert compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl330_bitcast(compiler_glsl):
    code = """
    #version 330

    in int a;
    out float b;

    void main() {
        b = floatBitsToInt(a);
    }
    """

    assert compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl150_explicit_attrib_location(compiler_glsl):
    code = """
    #version 150

    layout(location=2) in vec4 a;

    void main() {
        gl_Position = a;
    }
    """

    assert not compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )


def test_shadercompiler_glsl150_explicit_attrib_location_extension(compiler_glsl):
    code = """
    #version 150
    #extension GL_ARB_explicit_attrib_location : enable

    layout(location=2) in vec4 a;

    void main() {
        gl_Position = a;
    }
    """

    module = compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )
    assert module
    assert len(module.inputs) == 1
    assert module.inputs[0].name.get_name() == "a"
    assert module.inputs[0].location == 2


def test_shadercompiler_glsl330_explicit_attrib_location(compiler_glsl):
    code = """
    #version 330

    layout(location=2) in vec4 a;

    void main() {
        gl_Position = a;
    }
    """

    module = compiler_glsl.compile_now(
        core.ShaderModule.Stage.vertex,
        core.StringStream(code.encode('ascii')),
    )
    assert module
    assert len(module.inputs) == 1
    assert module.inputs[0].name.get_name() == "a"
    assert module.inputs[0].location == 2
