from panda3d import core

Shader = core.Shader
Stage = core.Shader.Stage


def compile_and_get_caps(stage, code):
    registry = core.ShaderCompilerRegistry.get_global_ptr()
    compiler = registry.get_compiler_for_language(Shader.SL_GLSL)
    stream = core.StringStream(code.encode('ascii'))
    module = compiler.compile_now(stage, stream, 'test-shader')
    assert module
    assert module.stage == stage

    # Mask out this bit, every shader should have it
    assert (module.used_capabilities & Shader.C_basic_shader) != 0
    return module.used_capabilities & ~Shader.C_basic_shader


def test_legacy_glsl_caps_basic_vertex():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 120

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == 0


def test_legacy_glsl_caps_basic_fragment():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 120

    void main() {
        gl_FragColor = vec4(0.0);
    }
    """) == 0


def test_legacy_glsl_caps_unified_model():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 130

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == Shader.C_unified_model

    # Optional extension doesn't trigger it
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 120

    #extension GL_EXT_gpu_shader4 : enable

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == 0

    # Required extension does
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 120

    #extension GL_EXT_gpu_shader4 : require

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == Shader.C_unified_model

    # Extension under #if doesn't either
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 120

    #if foobar
    #extension GL_EXT_gpu_shader4 : require
    #endif

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == 0


def test_glsl_caps_basic_vertex():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == 0


def test_glsl_caps_basic_fragment():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(1.0);
    }
    """) == 0


def test_glsl_caps_vertex_texture():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform sampler2D tex;

    void main() {
        gl_Position = textureLod(tex, vec2(0.0), 0);
    }
    """) == Shader.C_vertex_texture


def test_glsl_caps_point_coord():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(gl_PointCoord, 0.0, 1.0);
    }
    """) == Shader.C_point_coord


def test_glsl_caps_standard_derivatives():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(fwidth(gl_FragCoord.x), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_standard_derivatives


def test_glsl_caps_shadow_samplers():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    uniform sampler2DShadow a;

    void main() {
        gl_FragColor = vec4(texture(a, vec3(0.0, 0.0, 0.0)));
    }
    """) == Shader.C_shadow_samplers


def test_glsl_caps_non_square_matrices():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform mat3x4 a;

    void main() {
        gl_Position = a * vec3(1.0);
    }
    """) == Shader.C_non_square_matrices


def test_glsl_caps_unified_model_uint_uniform():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform uint a;

    void main() {
        gl_Position = vec4(float(a));
    }
    """) == Shader.C_unified_model


def test_glsl_caps_unified_model_int_varying():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    flat in int a;

    void main() {
        gl_FragColor = vec4(float(a), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_unified_model


def test_glsl_caps_unified_model_round():
    # DO NOT match unified model for regular round()
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform float a;

    void main() {
        gl_Position = vec4(round(a));
    }
    """) == 0

    # DO match it for roundEven()
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform float a;

    void main() {
        gl_Position = vec4(roundEven(a));
    }
    """) == Shader.C_unified_model


def test_glsl_caps_noperspective_interpolation():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    noperspective in vec4 a;

    void main() {
        gl_FragColor = a;
    }
    """) == Shader.C_noperspective_interpolation


def test_glsl_caps_texture_array():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    uniform sampler2DArray a;

    void main() {
        gl_FragColor = texture(a, vec3(0.0));
    }
    """) == Shader.C_texture_array


def test_glsl_caps_texture_lod():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    uniform sampler2D a;

    void main() {
        gl_FragColor = textureLod(a, vec2(0.0), 0);
    }
    """) == Shader.C_texture_lod


def test_glsl_caps_texture_query_size():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    uniform sampler2D a;

    void main() {
        gl_FragColor = vec4(vec2(textureSize(a, 0)), 0.0, 1.0);
    }
    """) == Shader.C_texture_query_size


def test_glsl_caps_sampler_cube_shadow():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    uniform samplerCubeShadow a;

    void main() {
        gl_FragColor = vec4(texture(a, vec4(0.0)), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_shadow_samplers | Shader.C_sampler_cube_shadow


def test_glsl_caps_vertex_id():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    void main() {
        gl_Position = vec4(float(gl_VertexID), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_vertex_id


def test_glsl_caps_draw_buffers():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    layout(location=0) out vec4 a;
    layout(location=1) out vec4 b;

    void main() {
        a = vec4(1.0, 0.0, 0.0, 1.0);
        b = vec4(0.0, 1.0, 0.0, 1.0);
    }
    """) == Shader.C_draw_buffers


def test_glsl_caps_clip_distance():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    void main() {
        gl_Position = vec4(0.0);
        gl_ClipDistance[0] = 0.0;
    }
    """) == Shader.C_clip_distance


def test_glsl_caps_instance_id():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    void main() {
        gl_Position = vec4(float(gl_InstanceID), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_instance_id


def test_glsl_caps_geometry_shader():
    assert compile_and_get_caps(Stage.GEOMETRY, """
    #version 330

    layout(triangles) in;
    layout(triangle_strip, max_vertices=3) out;

    void main() {
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
    """) == Shader.C_geometry_shader


def test_glsl_caps_primitive_id():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    void main() {
        gl_FragColor = vec4(float(gl_PrimitiveID), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_primitive_id


def test_glsl_caps_bit_encoding():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 330

    uniform uint a;

    void main() {
        gl_Position = vec4(uintBitsToFloat(a));
    }
    """) == Shader.C_unified_model | Shader.C_bit_encoding


def test_glsl_caps_texture_query_lod():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform sampler2D a;

    void main() {
        gl_FragColor = vec4(textureQueryLod(a, vec2(0.0)), 0.0, 1.0);
    }
    """) == Shader.C_texture_query_lod


def test_glsl_caps_texture_gather_red():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform sampler2D a;

    void main() {
        gl_FragColor = textureGather(a, vec2(0.0)) + textureGather(a, vec2(0.0), 0);
    }
    """) == Shader.C_texture_gather_red


def test_glsl_caps_texture_gather_any():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform sampler2D a;

    void main() {
        gl_FragColor = textureGather(a, vec2(0.0), 1);
    }
    """) == Shader.C_texture_gather_any


def test_glsl_caps_extended_arithmetic():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform uint a;
    uniform uint b;

    void main() {
        uint c;
        uint d = uaddCarry(a, b, c);
        gl_FragColor = vec4(a, b, c, d);
    }
    """) == Shader.C_unified_model | Shader.C_extended_arithmetic


def test_glsl_caps_double():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 400

    uniform double a;

    void main() {
        gl_Position = vec4(float(a), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_double


def test_glsl_caps_cube_map_array():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform samplerCubeArray a;

    void main() {
        gl_FragColor = texture(a, vec4(0.0));
    }
    """) == Shader.C_texture_array | Shader.C_cube_map_array


def test_glsl_caps_geometry_shader_instancing():
    assert compile_and_get_caps(Stage.GEOMETRY, """
    #version 400

    layout(triangles, invocations=2) in;
    layout(triangle_strip, max_vertices=3) out;

    void main() {
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
    """) == Shader.C_geometry_shader | Shader.C_geometry_shader_instancing


def test_glsl_caps_tessellation_shader():
    assert compile_and_get_caps(Stage.TESS_CONTROL, """
    #version 400 core

    layout (vertices = 3) out;

    void main() {
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
        gl_TessLevelInner[0] = 1;
    }
    """) == Shader.C_tessellation_shader

    assert compile_and_get_caps(Stage.TESS_EVALUATION, """
    #version 400 core

    layout(triangles, equal_spacing, ccw) in;

    void main() {
        gl_Position = vec4(0.0);
    }
    """) == Shader.C_tessellation_shader


def test_glsl_caps_sample_variables():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    #extension GL_ARB_sample_shading : require

    void main() {
        gl_FragColor = vec4(float(gl_SampleID), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_sample_variables


def test_glsl_caps_multisample_interpolation():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    sample in vec4 a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = a;
    }
    """) == Shader.C_multisample_interpolation


def test_glsl_caps_dynamic_indexing():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    uniform int a;
    uniform sampler2D b[3];

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = texture(b[a], vec2(0.0));
    }
    """) == Shader.C_dynamic_indexing

    # NOT dynamic indexing
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    const int a = 2;
    uniform sampler2D b[3];

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = texture(b[a], vec2(0.0));
    }
    """) == 0

    # Such a simple loop MUST be unrolled!
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    #define COUNT 3
    struct LightSource {
      sampler2D shadowMap;
    };
    uniform LightSource source[COUNT];

    out vec4 p3d_FragColor;

    void main() {
        vec4 result = vec4(0);
        for (int i = 0; i < COUNT; ++i) {
            result += texture(source[i].shadowMap, vec2(0.0));
        }
        p3d_FragColor = result;
    }
    """) == 0

    # This one need not be unrolled.
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 400

    #define COUNT 3
    struct LightSource {
      sampler2D shadowMap;
    };
    uniform LightSource source[COUNT];

    out vec4 p3d_FragColor;

    void main() {
        vec4 result = vec4(0);
        for (int i = 0; i < COUNT; ++i) {
            result += texture(source[i].shadowMap, vec2(0.0));
        }
        p3d_FragColor = result;
    }
    """) == Shader.C_dynamic_indexing

    # This one need not be unrolled either.
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 330

    #extension GL_ARB_gpu_shader5 : require

    #define COUNT 3
    struct LightSource {
      sampler2D shadowMap;
    };
    uniform LightSource source[COUNT];

    out vec4 p3d_FragColor;

    void main() {
        vec4 result = vec4(0);
        for (int i = 0; i < COUNT; ++i) {
            result += texture(source[i].shadowMap, vec2(0.0));
        }
        p3d_FragColor = result;
    }
    """) == Shader.C_dynamic_indexing


def test_glsl_caps_atomic_counters():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 420

    uniform atomic_uint a;

    void main() {
        gl_Position = vec4(float(atomicCounter(a)));
    }
    """) == Shader.C_unified_model | Shader.C_atomic_counters


def test_glsl_caps_image_load_store():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 420

    layout(rgba8) uniform readonly image2D a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = imageLoad(a, ivec2(0, 0));
    }
    """) == Shader.C_image_load_store


def test_glsl_caps_image_buffer():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 420

    layout(rgba8) uniform readonly imageBuffer a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = imageLoad(a, 0);
    }
    """) == Shader.C_image_load_store | Shader.C_texture_buffer


def test_glsl_caps_image_atomic():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 420

    layout(r32i) uniform iimage1D a;

    out vec4 p3d_FragColor;

    void main() {
        imageAtomicExchange(a, 0, 0);
        p3d_FragColor = vec4(0.0);
    }
    """) == Shader.C_image_load_store | Shader.C_image_atomic


def test_glsl_caps_image_query_size():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 420

    #extension GL_ARB_shader_image_size : require

    uniform writeonly image2D a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(vec2(imageSize(a)), 0.0, 1.0);
    }
    """) == Shader.C_image_query_size


def test_glsl_caps_texture_query_levels():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 430

    uniform sampler2D a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(float(textureQueryLevels(a)), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_texture_query_levels


def test_glsl_caps_storage_buffer():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 430

    buffer A {
        vec4 pos;
    } a;

    void main() {
        gl_Position = a.pos;
    }
    """) == Shader.C_storage_buffer

    # Don't trigger for a regular UBO
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 430

    uniform A {
        vec4 pos;
    } a;

    void main() {
        gl_Position = a.pos;
    }
    """) == 0


def test_glsl_caps_compute_shader():
    assert compile_and_get_caps(Stage.COMPUTE, """
    #version 430

    layout(local_size_x=16, local_size_y=16) in;

    void main() {
    }
    """) == Shader.C_compute_shader


def test_glsl_caps_enhanced_layouts():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 450

    layout(location=0, component=1) out float a;

    void main() {
        gl_Position = vec4(0.0);
        a = 0.0;
    }
    """) == Shader.C_enhanced_layouts


def test_glsl_caps_cull_distance():
    assert compile_and_get_caps(Stage.VERTEX, """
    #version 450

    void main() {
        gl_Position = vec4(0.0);
        gl_CullDistance[0] = 0.0;
    }
    """) == Shader.C_cull_distance


def test_glsl_caps_derivative_control():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 450

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(fwidthFine(gl_FragCoord.x), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_standard_derivatives | Shader.C_derivative_control


def test_glsl_caps_texture_query_samples():
    assert compile_and_get_caps(Stage.FRAGMENT, """
    #version 450

    uniform sampler2DMS a;

    out vec4 p3d_FragColor;

    void main() {
        p3d_FragColor = vec4(float(textureSamples(a)), 0.0, 0.0, 1.0);
    }
    """) == Shader.C_texture_query_samples
