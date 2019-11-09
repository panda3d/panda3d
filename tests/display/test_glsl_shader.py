from panda3d import core
import os
import struct
import pytest
from _pytest.outcomes import Failed


SHADERS_DIR = core.Filename.from_os_specific(os.path.dirname(__file__))


# This is the template for the compute shader that is used by run_glsl_test.
# It defines an assert() macro that writes failures to a buffer, indexed by
# line number.
# The reset() function serves to prevent the _triggered variable from being
# optimized out in the case that the assertions are being optimized out.
GLSL_COMPUTE_TEMPLATE = """#version {version}
{extensions}

layout(local_size_x = 1, local_size_y = 1) in;

{preamble}

layout(r8ui) uniform writeonly uimageBuffer _triggered;

void _reset() {{
    imageStore(_triggered, 0, uvec4(0, 0, 0, 0));
    memoryBarrier();
}}

void _assert(bool cond, int line) {{
    if (!cond) {{
        imageStore(_triggered, line, uvec4(1));
    }}
}}

#define assert(cond) _assert(cond, __LINE__)

void main() {{
    _reset();
{body}
}}
"""


def run_glsl_test(gsg, body, preamble="", inputs={}, version=150, exts=set()):
    """ Runs a GLSL test on the given GSG.  The given body is executed in the
    main function and should call assert().  The preamble should contain all
    of the shader inputs. """

    if not gsg.supports_compute_shaders or not gsg.supports_glsl:
        pytest.skip("compute shaders not supported")

    if not gsg.supports_buffer_texture:
        pytest.skip("buffer textures not supported")

    exts = exts | {'GL_ARB_compute_shader', 'GL_ARB_shader_image_load_store'}
    missing_exts = sorted(ext for ext in exts if not gsg.has_extension(ext))
    if missing_exts:
        pytest.skip("missing extensions: " + ' '.join(missing_exts))

    extensions = ''
    for ext in exts:
        extensions += '#extension {ext} : require\n'.format(ext=ext)

    __tracebackhide__ = True

    preamble = preamble.strip()
    body = body.rstrip().lstrip('\n')
    code = GLSL_COMPUTE_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
    line_offset = code[:code.find(body)].count('\n') + 1
    shader = core.Shader.make_compute(core.Shader.SL_GLSL, code)
    assert shader, code

    # Create a buffer to hold the results of the assertion.  We use one byte
    # per line of shader code, so we can show which lines triggered.
    result = core.Texture("")
    result.set_clear_color((0, 0, 0, 0))
    result.setup_buffer_texture(code.count('\n'), core.Texture.T_unsigned_byte,
                                core.Texture.F_r8i, core.GeomEnums.UH_static)

    # Build up the shader inputs
    attrib = core.ShaderAttrib.make(shader)
    for name, value in inputs.items():
        attrib = attrib.set_shader_input(name, value)
    attrib = attrib.set_shader_input('_triggered', result)

    # Run the compute shader.
    engine = core.GraphicsEngine.get_global_ptr()
    try:
        engine.dispatch_compute((1, 1, 1), attrib, gsg)
    except AssertionError as exc:
        assert False, "Error executing compute shader:\n" + code

    # Download the texture to check whether the assertion triggered.
    assert engine.extract_texture_data(result, gsg)
    triggered = result.get_ram_image()
    if any(triggered):
        count = len(triggered) - triggered.count(0)
        lines = body.split('\n')
        formatted = ''
        for i, line in enumerate(lines):
            if triggered[i + line_offset]:
                formatted += '=>  ' + line + '\n'
            else:
                formatted += '    ' + line + '\n'
        pytest.fail("{0} GLSL assertions triggered:\n{1}".format(count, formatted))


def run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=False):
    """Compile supplied GLSL shader paths and check for errors"""
    shader = core.Shader.load(core.Shader.SL_GLSL, vert_path, frag_path)
    assert shader is not None

    shader.prepare_now(gsg.prepared_objects, gsg)
    assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader.get_error_flag()
    else:
        assert not shader.get_error_flag()


def test_glsl_test(gsg):
    "Test to make sure that the GLSL tests work correctly."

    run_glsl_test(gsg, "assert(true);")


def test_glsl_test_fail(gsg):
    "Same as above, but making sure that the failure case works correctly."

    with pytest.raises(Failed):
        run_glsl_test(gsg, "assert(false);")


def test_glsl_sampler(gsg):
    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8)
    tex1.set_clear_color((0, 2 / 255.0, 1, 1))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_rgba32)
    tex2.set_clear_color((1.0, 2.0, -3.14, 0.0))

    preamble = """
    uniform sampler1D tex1;
    uniform sampler2D tex2;
    """
    code = """
    assert(texelFetch(tex1, 0, 0) == vec4(0, 2 / 255.0, 1, 1));
    assert(texelFetch(tex2, ivec2(0, 0), 0) == vec4(1.0, 2.0, -3.14, 0.0));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2})


def test_glsl_image(gsg):
    tex1 = core.Texture("")
    tex1.setup_1d_texture(1, core.Texture.T_unsigned_byte, core.Texture.F_rgba8)
    tex1.set_clear_color((0, 2 / 255.0, 1, 1))

    tex2 = core.Texture("")
    tex2.setup_2d_texture(1, 1, core.Texture.T_float, core.Texture.F_rgba32)
    tex2.set_clear_color((1.0, 2.0, -3.14, 0.0))

    preamble = """
    layout(rgba8) uniform image1D tex1;
    layout(rgba32f) uniform image2D tex2;
    """
    code = """
    assert(imageLoad(tex1, 0) == vec4(0, 2 / 255.0, 1, 1));
    assert(imageLoad(tex2, ivec2(0, 0)) == vec4(1.0, 2.0, -3.14, 0.0));
    """
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2})


def test_glsl_ssbo(gsg):
    from struct import pack
    num1 = pack('<i', 1234567)
    num2 = pack('<i', -1234567)
    buffer1 = core.ShaderBuffer("buffer1", num1, core.GeomEnums.UH_static)
    buffer2 = core.ShaderBuffer("buffer2", num2, core.GeomEnums.UH_static)

    preamble = """
    layout(std430, binding=0) buffer buffer1 {
        int value1;
    };
    layout(std430, binding=1) buffer buffer2 {
        int value2;
    };
    """
    code = """
    assert(value1 == 1234567);
    assert(value2 == -1234567);
    """
    run_glsl_test(gsg, code, preamble, {'buffer1': buffer1, 'buffer2': buffer2},
                  exts={'GL_ARB_shader_storage_buffer_object',
                        'GL_ARB_uniform_buffer_object',
                        'GL_ARB_shading_language_420pack'})


def test_glsl_int(gsg):
    inputs = dict(
        zero=0,
        intmax=0x7fffffff,
        intmin=-0x7fffffff,
    )
    preamble = """
    uniform int zero;
    uniform int intmax;
    uniform int intmin;
    """
    code = """
    assert(zero == 0);
    assert(intmax == 0x7fffffff);
    assert(intmin == -0x7fffffff);
    """
    run_glsl_test(gsg, code, preamble, inputs)


def test_glsl_uint(gsg):
    #TODO: fix passing uints greater than intmax
    inputs = dict(
        zero=0,
        intmax=0x7fffffff,
    )
    preamble = """
    uniform uint zero;
    uniform uint intmax;
    """
    code = """
    assert(zero == 0u);
    assert(intmax == 0x7fffffffu);
    """
    run_glsl_test(gsg, code, preamble, inputs)


def test_glsl_bool(gsg):
    flags = dict(
        flag1=False,
        flag2=0,
        flag3=0.0,
        flag4=True,
        flag5=1,
        flag6=3,
    )
    preamble = """
    uniform bool flag1;
    uniform bool flag2;
    uniform bool flag3;
    uniform bool flag4;
    uniform bool flag5;
    uniform bool flag6;
    """
    code = """
    assert(!flag1);
    assert(!flag2);
    assert(!flag3);
    assert(flag4);
    assert(flag5);
    assert(flag6);
    """
    run_glsl_test(gsg, code, preamble, flags)


def test_glsl_pta_int(gsg):
    pta = core.PTA_int((0, 1, 2, 3))

    preamble = """
    uniform int pta[4];
    """
    code = """
    assert(pta[0] == 0);
    assert(pta[1] == 1);
    assert(pta[2] == 2);
    assert(pta[3] == 3);
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


def test_glsl_pta_ivec4(gsg):
    pta = core.PTA_LVecBase4i(((0, 1, 2, 3), (4, 5, 6, 7)))

    preamble = """
    uniform ivec4 pta[2];
    """
    code = """
    assert(pta[0] == ivec4(0, 1, 2, 3));
    assert(pta[1] == ivec4(4, 5, 6, 7));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


def test_glsl_pta_mat4(gsg):
    pta = core.PTA_LMatrix4f((
        (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
        (16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31),
    ))

    preamble = """
    uniform mat4 pta[2];
    """
    code = """
    assert(pta[0][0] == vec4(0, 1, 2, 3));
    assert(pta[0][1] == vec4(4, 5, 6, 7));
    assert(pta[0][2] == vec4(8, 9, 10, 11));
    assert(pta[0][3] == vec4(12, 13, 14, 15));
    assert(pta[1][0] == vec4(16, 17, 18, 19));
    assert(pta[1][1] == vec4(20, 21, 22, 23));
    assert(pta[1][2] == vec4(24, 25, 26, 27));
    assert(pta[1][3] == vec4(28, 29, 30, 31));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta})


def test_glsl_param_vec4(gsg):
    param = core.ParamVecBase4((0, 1, 2, 3))

    preamble = """
    uniform vec4 param;
    """
    code = """
    assert(param.x == 0.0);
    assert(param.y == 1.0);
    assert(param.z == 2.0);
    assert(param.w == 3.0);
    """
    run_glsl_test(gsg, code, preamble, {'param': param})


def test_glsl_param_ivec4(gsg):
    param = core.ParamVecBase4i((0, 1, 2, 3))

    preamble = """
    uniform ivec4 param;
    """
    code = """
    assert(param.x == 0);
    assert(param.y == 1);
    assert(param.z == 2);
    assert(param.w == 3);
    """
    run_glsl_test(gsg, code, preamble, {'param': param})


def test_glsl_write_extract_image_buffer(gsg):
    # Tests that we can write to a buffer texture on the GPU, and then extract
    # the data on the CPU.  We test two textures since there was in the past a
    # where it would only work correctly for one texture.
    tex1 = core.Texture("tex1")
    tex1.set_clear_color(0)
    tex1.setup_buffer_texture(1, core.Texture.T_unsigned_int, core.Texture.F_r32i,
                              core.GeomEnums.UH_static)
    tex2 = core.Texture("tex2")
    tex2.set_clear_color(0)
    tex2.setup_buffer_texture(1, core.Texture.T_int, core.Texture.F_r32i,
                              core.GeomEnums.UH_static)

    preamble = """
    layout(r32ui) uniform uimageBuffer tex1;
    layout(r32i) uniform iimageBuffer tex2;
    """
    code = """
    assert(imageLoad(tex1, 0).r == 0u);
    assert(imageLoad(tex2, 0).r == 0);
    imageStore(tex1, 0, uvec4(123));
    imageStore(tex2, 0, ivec4(-456));
    memoryBarrier();
    assert(imageLoad(tex1, 0).r == 123u);
    assert(imageLoad(tex2, 0).r == -456);
    """

    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2})

    engine = core.GraphicsEngine.get_global_ptr()
    assert engine.extract_texture_data(tex1, gsg)
    assert engine.extract_texture_data(tex2, gsg)

    assert struct.unpack('I', tex1.get_ram_image()) == (123,)
    assert struct.unpack('i', tex2.get_ram_image()) == (-456,)


def test_glsl_compile_error(gsg):
    """Test getting compile errors from bad shaders"""
    vert_path = core.Filename(SHADERS_DIR, 'glsl_bad.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path, expect_fail=True)


def test_glsl_from_file(gsg):
    """Test compiling GLSL shaders from files"""
    vert_path = core.Filename(SHADERS_DIR, 'glsl_simple.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)


def test_glsl_includes(gsg):
    """Test preprocessing includes in GLSL shaders"""
    vert_path = core.Filename(SHADERS_DIR, 'glsl_include.vert')
    frag_path = core.Filename(SHADERS_DIR, 'glsl_simple.frag')
    run_glsl_compile_check(gsg, vert_path, frag_path)
