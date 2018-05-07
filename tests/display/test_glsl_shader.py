from panda3d import core
import pytest
from _pytest.outcomes import Failed


# This is the template for the compute shader that is used by run_glsl_test.
# It defines an assert() macro that writes failures to a buffer, indexed by
# line number.
# The reset() function serves to prevent the _triggered variable from being
# optimized out in the case that the assertions are being optimized out.
GLSL_COMPUTE_TEMPLATE = """#version {version}

layout(local_size_x = 1, local_size_y = 1) in;

{preamble}

layout(r8ui) uniform writeonly uimageBuffer _triggered;

void _reset() {{
    imageStore(_triggered, 0, uvec4(0, 0, 0, 0));
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


def run_glsl_test(gsg, body, preamble="", inputs={}, version=430):
    """ Runs a GLSL test on the given GSG.  The given body is executed in the
    main function and should call assert().  The preamble should contain all
    of the shader inputs. """

    if not gsg.supports_compute_shaders or not gsg.supports_glsl:
        pytest.skip("compute shaders not supported")

    __tracebackhide__ = True

    preamble = preamble.strip()
    body = body.rstrip().lstrip('\n')
    code = GLSL_COMPUTE_TEMPLATE.format(version=version, preamble=preamble, body=body)
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
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2}), code


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
    run_glsl_test(gsg, code, preamble, {'tex1': tex1, 'tex2': tex2}), code


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
    run_glsl_test(gsg, code, preamble, {'buffer1': buffer1, 'buffer2': buffer2}), code


def test_glsl_int(gsg):
    inputs = dict(
        zero=0,
        intmax=0x7fffffff,
        intmin=-0x80000000,
    )
    preamble = """
    uniform int zero;
    uniform int intmax;
    uniform int intmin;
    """
    code = """
    assert(zero == 0);
    assert(intmax == 0x7fffffff);
    assert(intmin == -0x80000000);
    """
    run_glsl_test(gsg, code, preamble, inputs)


@pytest.mark.xfail
def test_glsl_uint(gsg):
    #TODO: fix passing uints greater than intmax
    inputs = dict(
        zero=0,
        intmax=0x7fffffff,
    )
    preamble = """
    uniform unsigned int zero;
    uniform unsigned int intmax;
    """
    code = """
    assert(zero == 0);
    assert(intmax == 0x7fffffff);
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
    run_glsl_test(gsg, code, preamble, {'pta': pta}), code


def test_glsl_pta_ivec4(gsg):
    pta = core.PTA_LVecBase4i(((0, 1, 2, 3), (4, 5, 6, 7)))

    preamble = """
    uniform ivec4 pta[2];
    """
    code = """
    assert(pta[0] == ivec4(0, 1, 2, 3));
    assert(pta[1] == ivec4(4, 5, 6, 7));
    """
    run_glsl_test(gsg, code, preamble, {'pta': pta}), code


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
    run_glsl_test(gsg, code, preamble, {'pta': pta}), code
