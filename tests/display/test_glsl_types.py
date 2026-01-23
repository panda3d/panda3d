from panda3d.core import Shader, ShaderType, Texture, CompilerOptions
import pytest


def compile_parameter(code):
    options = CompilerOptions()
    options.optimize = CompilerOptions.Optimize.NONE

    shader = Shader.make_compute(Shader.SL_GLSL, "#version 430\n" + code + "\nvoid main() {}\n", options)
    module = shader.get_module(Shader.Stage.COMPUTE)
    return module.parameters[0]


def compile_type(glsl_type, layout=None):
    if layout is None:
        param = compile_parameter(f"uniform {glsl_type} x;")
        return param.type
    else:
        param = compile_parameter(f"layout({layout}) buffer B {{ {glsl_type} x; }};")
        return param.type.contained_type.members[0].type


def compile_struct(members, layout=None):
    if layout is None:
        param = compile_parameter(f"uniform struct {{ {members} }} x;")
        return param.type
    else:
        param = compile_parameter(f"layout({layout}) buffer B {{ {members} }};")
        return param.type.contained_type


def test_glsl_scalar_singletons():
    #assert isinstance(ShaderType.VOID, ShaderType)
    assert isinstance(ShaderType.BOOL, ShaderType.Scalar)
    assert isinstance(ShaderType.INT, ShaderType.Scalar)
    assert isinstance(ShaderType.UINT, ShaderType.Scalar)
    assert isinstance(ShaderType.FLOAT, ShaderType.Scalar)
    assert isinstance(ShaderType.DOUBLE, ShaderType.Scalar)
    assert isinstance(ShaderType.SAMPLER, ShaderType)

    assert compile_type("float") == ShaderType.FLOAT
    assert compile_type("double") == ShaderType.DOUBLE
    assert compile_type("int") == ShaderType.INT
    assert compile_type("uint") == ShaderType.UINT
    assert compile_type("bool") == ShaderType.BOOL

    assert ShaderType.FLOAT != ShaderType.DOUBLE
    assert ShaderType.FLOAT != ShaderType.INT
    assert ShaderType.FLOAT != ShaderType.UINT
    assert ShaderType.FLOAT != ShaderType.BOOL
    assert ShaderType.INT != ShaderType.UINT


def test_glsl_type_equality():
    vec4_1 = compile_type("vec4")
    vec4_2 = compile_type("vec4")
    assert vec4_1 == vec4_2
    assert vec4_1.this == vec4_2.this

    mat4_1 = compile_type("mat4")
    mat4_2 = compile_type("mat4")
    assert mat4_1 == mat4_2
    assert mat4_1.this == mat4_2.this

    assert mat4_1 != vec4_1


def test_glsl_types_std140():
    # For std140 buffers, the compiler determines the layout.  For global
    # uniforms, Panda determines it.  So we check whether Panda did it right
    # by checking whether the types are identical.
    def check_type(type):
        assert compile_type(type) == compile_type(type, layout="std140")

        # Also check a struct where it's followed by a scalar, so we know that
        # the size is correct
        assert compile_struct(f"{type} a; float b;") == compile_struct(f"{type} a; float b;", layout="std140")

        # Also check a struct where it's following a scalar, so we know that
        # the alignment is correct
        assert compile_struct(f"float a; {type} b;") == compile_struct(f"float a; {type} b;", layout="std140")

    check_type("vec2")
    check_type("vec3")
    check_type("vec4")
    check_type("mat2")
    check_type("mat3")
    check_type("mat4")
    check_type("mat2x3")
    check_type("mat3x2")
    check_type("mat4x2")
    check_type("mat4x3")
    check_type("mat2x4")
    check_type("mat3x4")
    check_type("float[2]")
    check_type("vec2[2]")
    check_type("vec3[2]")
    check_type("vec4[2]")
    check_type("mat2[2]")
    check_type("mat3[2]")
    check_type("mat4[2]")
    check_type("dvec2")
    check_type("dvec3")
    check_type("dvec4")
    check_type("dmat2")
    check_type("dmat3")
    check_type("dmat4")
    check_type("dmat2x3")
    check_type("dmat3x2")
    check_type("dmat4x2")
    check_type("dmat4x3")
    check_type("dmat2x4")
    check_type("dmat3x4")
    check_type("double[2]")
    check_type("dvec2[2]")
    check_type("dvec3[2]")
    check_type("dvec4[2]")
    check_type("dmat2[2]")
    check_type("dmat3[2]")
    check_type("dmat4[2]")

    # Just to be sure check it's not actually giving equivalence for a type
    # that's known to have a different layout
    assert compile_type("mat2") != compile_type("mat2", layout="std430")
    assert compile_type("mat3x2") != compile_type("mat3x2", layout="std430")
    assert compile_type("mat4x2") != compile_type("mat4x2", layout="std430")
    assert compile_type("mat2[2]") != compile_type("mat2[2]", layout="std430")


def test_glsl_scalar_str():
    #assert str(ShaderType.VOID) == "void"
    assert str(ShaderType.BOOL) == "bool"
    assert str(ShaderType.INT) == "int"
    assert str(ShaderType.UINT) == "uint"
    assert str(ShaderType.FLOAT) == "float"
    assert str(ShaderType.DOUBLE) == "double"


def test_glsl_scalar_size_bytes():
    assert ShaderType.BOOL.size_bytes == 4
    assert ShaderType.INT.size_bytes == 4
    assert ShaderType.UINT.size_bytes == 4
    assert ShaderType.FLOAT.size_bytes == 4
    assert ShaderType.DOUBLE.size_bytes == 8


def test_glsl_scalar_align_bytes():
    assert ShaderType.BOOL.align_bytes == 4
    assert ShaderType.INT.align_bytes == 4
    assert ShaderType.UINT.align_bytes == 4
    assert ShaderType.FLOAT.align_bytes == 4
    assert ShaderType.DOUBLE.align_bytes == 8


def test_glsl_vector_float():
    vec2 = compile_type("vec2")
    assert isinstance(vec2, ShaderType.Vector)
    assert vec2.num_components == 2
    assert vec2.scalar_type == ShaderType.FLOAT
    assert vec2.size_bytes == 8
    assert vec2.align_bytes == 8

    vec3 = compile_type("vec3")
    assert isinstance(vec3, ShaderType.Vector)
    assert vec3.num_components == 3
    assert vec3.scalar_type == ShaderType.FLOAT
    assert vec3.size_bytes == 12
    assert vec3.align_bytes == 16

    vec4 = compile_type("vec4")
    assert isinstance(vec4, ShaderType.Vector)
    assert vec4.num_components == 4
    assert vec4.scalar_type == ShaderType.FLOAT
    assert vec4.size_bytes == 16
    assert vec4.align_bytes == 16


def test_glsl_vector_double():
    dvec2 = compile_type("dvec2")
    assert isinstance(dvec2, ShaderType.Vector)
    assert dvec2.num_components == 2
    assert dvec2.scalar_type == ShaderType.DOUBLE
    assert dvec2.size_bytes == 16
    assert dvec2.align_bytes == 16

    dvec3 = compile_type("dvec3")
    assert isinstance(dvec3, ShaderType.Vector)
    assert dvec3.num_components == 3
    assert dvec3.scalar_type == ShaderType.DOUBLE
    assert dvec3.size_bytes == 24
    assert dvec3.align_bytes == 32

    dvec4 = compile_type("dvec4")
    assert isinstance(dvec4, ShaderType.Vector)
    assert dvec4.num_components == 4
    assert dvec4.scalar_type == ShaderType.DOUBLE
    assert dvec4.size_bytes == 32
    assert dvec4.align_bytes == 32


def test_glsl_vector_int():
    ivec2 = compile_type("ivec2")
    assert isinstance(ivec2, ShaderType.Vector)
    assert ivec2.num_components == 2
    assert ivec2.scalar_type == ShaderType.INT
    assert ivec2.size_bytes == 8
    assert ivec2.align_bytes == 8

    ivec3 = compile_type("ivec3")
    assert isinstance(ivec3, ShaderType.Vector)
    assert ivec3.num_components == 3
    assert ivec3.scalar_type == ShaderType.INT
    assert ivec3.size_bytes == 12
    assert ivec3.align_bytes == 16

    ivec4 = compile_type("ivec4")
    assert isinstance(ivec4, ShaderType.Vector)
    assert ivec4.num_components == 4
    assert ivec4.scalar_type == ShaderType.INT
    assert ivec4.size_bytes == 16
    assert ivec4.align_bytes == 16


def test_glsl_vector_uint():
    uvec2 = compile_type("uvec2")
    assert isinstance(uvec2, ShaderType.Vector)
    assert uvec2.num_components == 2
    assert uvec2.scalar_type == ShaderType.UINT
    assert uvec2.size_bytes == 8
    assert uvec2.align_bytes == 8

    uvec3 = compile_type("uvec3")
    assert isinstance(uvec3, ShaderType.Vector)
    assert uvec3.num_components == 3
    assert uvec3.scalar_type == ShaderType.UINT
    assert uvec3.size_bytes == 12
    assert uvec3.align_bytes == 16

    uvec4 = compile_type("uvec4")
    assert isinstance(uvec4, ShaderType.Vector)
    assert uvec4.num_components == 4
    assert uvec4.scalar_type == ShaderType.UINT
    assert uvec4.size_bytes == 16
    assert uvec4.align_bytes == 16


def test_glsl_vector_bool():
    bvec2 = compile_type("bvec2")
    assert isinstance(bvec2, ShaderType.Vector)
    assert bvec2.num_components == 2
    assert bvec2.scalar_type == ShaderType.BOOL
    assert bvec2.size_bytes == 8
    assert bvec2.align_bytes == 8

    bvec3 = compile_type("bvec3")
    assert isinstance(bvec3, ShaderType.Vector)
    assert bvec3.num_components == 3
    assert bvec3.scalar_type == ShaderType.BOOL
    assert bvec3.size_bytes == 12
    assert bvec3.align_bytes == 16

    bvec4 = compile_type("bvec4")
    assert isinstance(bvec4, ShaderType.Vector)
    assert bvec4.num_components == 4
    assert bvec4.scalar_type == ShaderType.BOOL
    assert bvec4.size_bytes == 16
    assert bvec4.align_bytes == 16


def test_glsl_vector_str():
    assert str(compile_type("vec2")) == "float2"
    assert str(compile_type("vec3")) == "float3"
    assert str(compile_type("vec4")) == "float4"
    assert str(compile_type("dvec2")) == "double2"
    assert str(compile_type("dvec3")) == "double3"
    assert str(compile_type("dvec4")) == "double4"
    assert str(compile_type("ivec2")) == "int2"
    assert str(compile_type("ivec3")) == "int3"
    assert str(compile_type("ivec4")) == "int4"
    assert str(compile_type("uvec2")) == "uint2"
    assert str(compile_type("uvec3")) == "uint3"
    assert str(compile_type("uvec4")) == "uint4"
    assert str(compile_type("bvec2")) == "bool2"
    assert str(compile_type("bvec3")) == "bool3"
    assert str(compile_type("bvec4")) == "bool4"


def test_glsl_matrix_float():
    mat2 = compile_type("mat2")
    assert isinstance(mat2, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat2.num_rows == 2
    assert mat2.num_columns == 2
    assert mat2.align_bytes == 16
    assert mat2.size_bytes == 32

    mat3 = compile_type("mat3")
    assert isinstance(mat3, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat3.num_rows == 3
    assert mat3.num_columns == 3
    assert mat3.align_bytes == 16
    assert mat3.size_bytes == 48

    mat4 = compile_type("mat4")
    assert isinstance(mat4, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat4.num_rows == 4
    assert mat4.num_columns == 4
    assert mat4.align_bytes == 16
    assert mat4.size_bytes == 64

    mat3x2 = compile_type("mat3x2")
    assert isinstance(mat3x2, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat3x2.num_rows == 3
    assert mat3x2.num_columns == 2
    assert mat3x2.align_bytes == 16
    assert mat3x2.size_bytes == 48

    mat4x2 = compile_type("mat4x2")
    assert isinstance(mat4x2, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat4x2.num_rows == 4
    assert mat4x2.num_columns == 2
    assert mat4x2.align_bytes == 16
    assert mat4x2.size_bytes == 64


def test_glsl_matrix_float_std430():
    mat2 = compile_type("mat2", layout="std430")
    assert isinstance(mat2, ShaderType.Matrix)
    assert mat2.scalar_type == ShaderType.FLOAT
    assert mat2.num_rows == 2
    assert mat2.num_columns == 2
    assert mat2.align_bytes == 8
    assert mat2.size_bytes == 16

    mat3 = compile_type("mat3", layout="std430")
    assert isinstance(mat3, ShaderType.Matrix)
    assert mat3.scalar_type == ShaderType.FLOAT
    assert mat3.num_rows == 3
    assert mat3.num_columns == 3
    assert mat3.align_bytes == 16
    assert mat3.size_bytes == 48

    mat4 = compile_type("mat4", layout="std430")
    assert isinstance(mat4, ShaderType.Matrix)
    assert mat4.scalar_type == ShaderType.FLOAT
    assert mat4.num_rows == 4
    assert mat4.num_columns == 4
    assert mat4.align_bytes == 16
    assert mat4.size_bytes == 64

    mat2x3 = compile_type("mat2x3", layout="std430")
    assert isinstance(mat2x3, ShaderType.Matrix)
    assert mat2x3.scalar_type == ShaderType.FLOAT
    assert mat2x3.num_rows == 2
    assert mat2x3.num_columns == 3
    assert mat2x3.align_bytes == 16
    assert mat2x3.size_bytes == 32

    mat3x2 = compile_type("mat3x2", layout="std430")
    assert isinstance(mat3x2, ShaderType.Matrix)
    assert mat3x2.scalar_type == ShaderType.FLOAT
    assert mat3x2.num_rows == 3
    assert mat3x2.num_columns == 2
    assert mat3x2.align_bytes == 8
    assert mat3x2.size_bytes == 24

    mat2x4 = compile_type("mat2x4", layout="std430")
    assert isinstance(mat2x4, ShaderType.Matrix)
    assert mat2x4.scalar_type == ShaderType.FLOAT
    assert mat2x4.align_bytes == 16
    assert mat2x4.size_bytes == 32

    mat4x2 = compile_type("mat4x2", layout="std430")
    assert isinstance(mat4x2, ShaderType.Matrix)
    assert mat4x2.scalar_type == ShaderType.FLOAT
    assert mat4x2.align_bytes == 8
    assert mat4x2.size_bytes == 32

    mat3x4 = compile_type("mat3x4", layout="std430")
    assert isinstance(mat3x4, ShaderType.Matrix)
    assert mat3x4.scalar_type == ShaderType.FLOAT
    assert mat3x4.align_bytes == 16
    assert mat3x4.size_bytes == 48

    mat4x3 = compile_type("mat4x3", layout="std430")
    assert isinstance(mat4x3, ShaderType.Matrix)
    assert mat4x3.scalar_type == ShaderType.FLOAT
    assert mat4x3.align_bytes == 16
    assert mat4x3.size_bytes == 64


def test_glsl_matrix_double_std430():
    dmat2 = compile_type("dmat2")
    assert isinstance(dmat2, ShaderType.Matrix)
    assert dmat2.scalar_type == ShaderType.DOUBLE
    assert dmat2.num_rows == 2
    assert dmat2.num_columns == 2
    assert dmat2.align_bytes == 16
    assert dmat2.size_bytes == 32

    dmat3 = compile_type("dmat3")
    assert isinstance(dmat3, ShaderType.Matrix)
    assert dmat3.scalar_type == ShaderType.DOUBLE
    assert dmat3.num_rows == 3
    assert dmat3.num_columns == 3
    assert dmat3.align_bytes == 32
    assert dmat3.size_bytes == 96

    dmat4 = compile_type("dmat4")
    assert isinstance(dmat4, ShaderType.Matrix)
    assert dmat4.scalar_type == ShaderType.DOUBLE
    assert dmat4.num_rows == 4
    assert dmat4.num_columns == 4
    assert dmat4.align_bytes == 32
    assert dmat4.size_bytes == 128


def test_glsl_matrix_str():
    assert str(compile_type("mat2")) == "float2x2"
    assert str(compile_type("mat3")) == "float3x3"
    assert str(compile_type("mat4")) == "float4x4"
    assert str(compile_type("mat2x3")) == "float2x3"
    assert str(compile_type("mat3x2")) == "float3x2"
    assert str(compile_type("dmat2")) == "double2x2"
    assert str(compile_type("dmat3")) == "double3x3"
    assert str(compile_type("dmat4")) == "double4x4"


def test_glsl_array_float():
    arr = compile_type("float[4]")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == ShaderType.FLOAT
    assert arr.num_elements == 4
    assert arr.stride_bytes == 16  # rounded to vec4
    assert arr.size_bytes == 64
    assert arr.align_bytes == 16


def test_glsl_array_vec2():
    arr = compile_type("vec2[3]")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == compile_type("vec2")
    assert arr.num_elements == 3
    assert arr.stride_bytes == 16  # rounded to vec4
    assert arr.size_bytes == 48
    assert arr.align_bytes == 16


def test_glsl_array_vec3():
    arr = compile_type("vec3[3]")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == compile_type("vec3")
    assert arr.num_elements == 3
    assert arr.stride_bytes == 16  # rounded to vec4
    assert arr.size_bytes == 48
    assert arr.align_bytes == 16


def test_glsl_array_vec4():
    arr = compile_type("vec4[3]")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == compile_type("vec4")
    assert arr.num_elements == 3
    assert arr.stride_bytes == 16
    assert arr.size_bytes == 48
    assert arr.align_bytes == 16


def test_glsl_array_float_std430():
    arr = compile_type("float[3]", layout="std430")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == ShaderType.FLOAT
    assert arr.num_elements == 3
    assert arr.stride_bytes == 4
    assert arr.size_bytes == 12
    assert arr.align_bytes == 4


def test_glsl_array_vec2_std430():
    arr = compile_type("vec2[3]", layout="std430")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == compile_type("vec2")
    assert arr.num_elements == 3
    assert arr.stride_bytes == 8
    assert arr.size_bytes == 24
    assert arr.align_bytes == 8


def test_glsl_array_vec3_std430():
    arr = compile_type("vec3[2]", layout="std430")
    assert isinstance(arr, ShaderType.Array)
    assert arr.element_type == compile_type("vec3")
    assert arr.num_elements == 2
    assert arr.stride_bytes == 16  # vec3 size 12 rounded to align 16
    assert arr.size_bytes == 32
    assert arr.align_bytes == 16


def test_glsl_array_mat2_std430():
    arr = compile_type("mat2[3]", layout="std430")
    assert isinstance(arr.element_type, ShaderType.Matrix)
    assert arr.element_type.num_rows == 2
    assert arr.element_type.num_columns == 2
    assert arr.num_elements == 3
    assert arr.stride_bytes == 16
    assert arr.size_bytes == 48
    assert arr.align_bytes == 8


def test_glsl_array_mat3_std430():
    arr = compile_type("mat3[3]", layout="std430")
    assert isinstance(arr.element_type, ShaderType.Matrix)
    assert arr.element_type.num_rows == 3
    assert arr.element_type.num_columns == 3
    assert arr.num_elements == 3
    assert arr.stride_bytes == 16 * 3
    assert arr.size_bytes == 144
    assert arr.align_bytes == 16


def test_glsl_array_mat4_std430():
    arr = compile_type("mat4[3]", layout="std430")
    assert isinstance(arr.element_type, ShaderType.Matrix)
    assert arr.element_type.num_rows == 4
    assert arr.element_type.num_columns == 4
    assert arr.num_elements == 3
    assert arr.stride_bytes == 64
    assert arr.size_bytes == 192
    assert arr.align_bytes == 16


def test_glsl_array_str():
    assert str(compile_type("float[4]")) == "float[4]"
    assert str(compile_type("vec3[2]")) == "float3[2]"
    assert str(compile_type("mat4[3]")) == "float4x4[3]"


def test_glsl_struct_float_array():
    struct = compile_struct("float a; float b[3]; float c;")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # array aligned to vec4
    assert struct.members[2].offset == 64  # 16 + 3*16
    assert struct.align_bytes == 16
    assert struct.size_bytes == 80


def test_glsl_struct_vec2_array():
    struct = compile_struct("float a; vec2 b[2]; float c;")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # aligned to vec4
    assert struct.members[2].offset == 48  # 16 + 2*16
    assert struct.align_bytes == 16
    assert struct.size_bytes == 64


def test_glsl_struct_mat2():
    # mat2 in std140: columns have vec4 stride, so size = 32, align = 16
    struct = compile_struct("float a; mat2 b; float c;")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # aligned to vec4
    assert struct.members[2].offset == 48  # 16 + 32
    assert struct.align_bytes == 16
    assert struct.size_bytes == 64


def test_glsl_struct_double():
    struct = compile_struct("float a; double b; float c;")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8   # double aligned to 8
    assert struct.members[2].offset == 16
    assert struct.align_bytes == 8
    assert struct.size_bytes == 24


def test_glsl_struct_dvec3():
    struct = compile_struct("float a; dvec3 b; float c;")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 32  # dvec3 aligned to 32
    assert struct.members[2].offset == 56
    assert struct.align_bytes == 32
    assert struct.size_bytes == 64


def test_glsl_struct_floats_std430():
    struct = compile_struct("float a; float b; float c;", layout="std430")
    assert isinstance(struct, ShaderType.Struct)
    assert len(struct.members) == 3
    assert struct.members[0].name == "a"
    assert struct.members[0].type == ShaderType.FLOAT
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 4
    assert struct.members[2].offset == 8
    assert struct.align_bytes == 4
    assert struct.size_bytes == 12


def test_glsl_struct_vec2_std430():
    struct = compile_struct("float a; vec2 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8   # vec2 aligned to 8
    assert struct.members[2].offset == 16
    assert struct.align_bytes == 8
    assert struct.size_bytes == 24  # 20 rounded to align 8


def test_glsl_struct_vec3_std430():
    struct = compile_struct("float a; vec3 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # vec3 aligned to 16
    assert struct.members[2].offset == 28
    assert struct.align_bytes == 16
    assert struct.size_bytes == 32


def test_glsl_struct_mixed_scalars_std430():
    struct = compile_struct("int a; float b; uint c; bool d;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 4
    assert struct.members[2].offset == 8
    assert struct.members[3].offset == 12
    assert struct.align_bytes == 4
    assert struct.size_bytes == 16


def test_glsl_struct_double_std430():
    struct = compile_struct("float a; double b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8   # double aligned to 8
    assert struct.members[2].offset == 16
    assert struct.align_bytes == 8
    assert struct.size_bytes == 24


def test_glsl_struct_dvec3_std430():
    struct = compile_struct("float a; dvec3 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 32  # dvec3 aligned to 32
    assert struct.members[2].offset == 56
    assert struct.align_bytes == 32
    assert struct.size_bytes == 64


def test_glsl_struct_mat4_std430():
    struct = compile_struct("float a; mat4 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # mat4 aligned to 16
    assert struct.members[2].offset == 80
    assert struct.align_bytes == 16
    assert struct.size_bytes == 96


def test_glsl_struct_mat2_std430():
    # mat2 in std430: align 8, size 16
    struct = compile_struct("float a; mat2 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8   # mat2 aligned to 8
    assert struct.members[2].offset == 24  # 8 + 16
    assert struct.align_bytes == 8
    assert struct.size_bytes == 32


def test_glsl_struct_mat3_std430():
    struct = compile_struct("float a; mat3 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # mat3 aligned to 16
    assert struct.members[2].offset == 64  # 16 + 48
    assert struct.align_bytes == 16
    assert struct.size_bytes == 80


def test_glsl_struct_array_std430():
    struct = compile_struct("float a; float b[3]; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 4
    assert struct.members[2].offset == 16  # 4 + 3*4
    assert struct.align_bytes == 4
    assert struct.size_bytes == 20


def test_glsl_struct_vec3_array_std430():
    struct = compile_struct("float a; vec3 b[2]; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # vec3 array aligned to 16
    assert struct.members[2].offset == 48  # 16 + 2*16
    assert struct.align_bytes == 16
    assert struct.size_bytes == 64


def test_glsl_struct_nested():
    param = compile_parameter("""
    struct Inner { float x; float y; };
    struct Outer { float a; Inner b; float c; };
    uniform Outer x;
    """)
    struct = param.type
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # struct aligned to vec4
    assert struct.members[2].offset == 32  # inner size 8 rounded to 16


def test_glsl_struct_nested_std430():
    param = compile_parameter("""
    struct Inner { float x; vec3 y; };
    layout(std430) buffer B { Inner a; float b; };
    """)
    struct = param.type.contained_type
    inner = struct.members[0].type
    assert isinstance(inner, ShaderType.Struct)
    assert inner.members[0].offset == 0
    assert inner.members[1].offset == 16
    assert inner.align_bytes == 16
    assert inner.size_bytes == 32  # 28 rounded to 16
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 32


def test_glsl_packing_vec3_scalar():
    struct = compile_struct("vec3 a; float b;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 12  # scalar fits in vec3 padding
    assert struct.size_bytes == 16


def test_glsl_packing_vec3_scalar_std430():
    struct = compile_struct("vec3 a; float b;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 12  # scalar fits in vec3 padding
    assert struct.size_bytes == 16


def test_glsl_packing_vec3_vec2_std430():
    struct = compile_struct("vec3 a; vec2 b;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16  # vec2 needs align 8, 12 isn't divisible
    assert struct.align_bytes == 16
    assert struct.size_bytes == 32


def test_glsl_packing_multiple_vec3_std430():
    struct = compile_struct("vec3 a; vec3 b; vec3 c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 16
    assert struct.members[2].offset == 32
    assert struct.size_bytes == 48  # 44 rounded to 16


def test_glsl_packing_interleaved_std430():
    struct = compile_struct("float a; vec2 b; float c; vec3 d; float e;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8
    assert struct.members[2].offset == 16
    assert struct.members[3].offset == 32
    assert struct.members[4].offset == 44
    assert struct.size_bytes == 48


def test_glsl_packing_mat3x2_std430():
    # mat3x2: 3 columns of vec2, stride 8, align 8
    struct = compile_struct("float a; mat3x2 b; float c;", layout="std430")
    assert struct.members[0].offset == 0
    assert struct.members[1].offset == 8   # aligned to 8
    assert struct.members[2].offset == 32  # 8 + 24
    assert struct.align_bytes == 8
    assert struct.size_bytes == 40  # 36 rounded to 8


def test_glsl_sampled_image_1d():
    tex_type = compile_type("sampler1D")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_1d_texture
    assert tex_type.sampled_type == ShaderType.FLOAT
    assert tex_type.shadow == False


def test_glsl_sampled_image_2d():
    tex_type = compile_type("sampler2D")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_2d_texture
    assert tex_type.sampled_type == ShaderType.FLOAT
    assert tex_type.shadow == False


def test_glsl_sampled_image_3d():
    tex_type = compile_type("sampler3D")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_3d_texture


def test_glsl_sampled_image_cube():
    tex_type = compile_type("samplerCube")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_cube_map


def test_glsl_sampled_image_2d_array():
    tex_type = compile_type("sampler2DArray")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_2d_texture_array


def test_glsl_sampled_image_cube_array():
    tex_type = compile_type("samplerCubeArray")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_cube_map_array


def test_glsl_sampled_image_buffer():
    tex_type = compile_type("samplerBuffer")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.texture_type == Texture.TT_buffer_texture


def test_glsl_sampled_image_shadow():
    tex_type = compile_type("sampler2DShadow")
    assert isinstance(tex_type, ShaderType.SampledImage)
    assert tex_type.shadow == True


def test_glsl_sampled_image_int():
    tex_type = compile_type("isampler2D")
    assert tex_type.sampled_type == ShaderType.INT


def test_glsl_sampled_image_uint():
    tex_type = compile_type("usampler2D")
    assert tex_type.sampled_type == ShaderType.UINT


def test_glsl_sampled_image_str():
    assert str(compile_type("sampler2D")) == "sampler2D"
    assert str(compile_type("sampler3D")) == "sampler3D"
    assert str(compile_type("samplerCube")) == "samplerCube"
    assert str(compile_type("sampler2DShadow")) == "sampler2DShadow"
    assert str(compile_type("isampler2D")) == "isampler2D"
    assert str(compile_type("usampler2D")) == "usampler2D"


def test_glsl_sampled_image_size_bytes():
    tex_type = compile_type("sampler2D")
    assert tex_type.size_bytes == 0


def test_glsl_image_2d_readonly():
    param = compile_parameter("layout(rgba32f) uniform readonly image2D img;")
    img_type = param.type
    assert isinstance(img_type, ShaderType.Image)
    assert img_type.texture_type == Texture.TT_2d_texture
    assert img_type.sampled_type == ShaderType.FLOAT
    assert img_type.access == ShaderType.Access.READ_ONLY
    assert img_type.writable == False


def test_glsl_image_2d_writeonly():
    param = compile_parameter("layout(rgba32f) uniform writeonly image2D img;")
    img_type = param.type
    assert img_type.access == ShaderType.Access.WRITE_ONLY
    assert img_type.writable == True


def test_glsl_image_2d_readwrite():
    param = compile_parameter("layout(rgba32f) uniform image2D img;")
    img_type = param.type
    assert img_type.access == ShaderType.Access.READ_WRITE
    assert img_type.writable == True


def test_glsl_image_int():
    param = compile_parameter("layout(rgba32i) uniform iimage2D img;")
    img_type = param.type
    assert img_type.sampled_type == ShaderType.INT


def test_glsl_image_uint():
    param = compile_parameter("layout(rgba32ui) uniform uimage2D img;")
    img_type = param.type
    assert img_type.sampled_type == ShaderType.UINT


def test_glsl_image_buffer():
    param = compile_parameter("layout(rgba32f) uniform imageBuffer img;")
    img_type = param.type
    assert img_type.texture_type == Texture.TT_buffer_texture


def test_glsl_image_str():
    assert str(compile_parameter("layout(rgba32f) uniform image2D img;").type) == "image2D"
    assert str(compile_parameter("layout(rgba32f) uniform image3D img;").type) == "image3D"
    assert str(compile_parameter("layout(rgba32i) uniform iimage2D img;").type) == "iimage2D"


def test_glsl_storage_buffer_basic():
    param = compile_parameter("layout(std430) buffer B { float data[]; };")
    ssbo_type = param.type
    assert isinstance(ssbo_type, ShaderType.StorageBuffer)
    assert ssbo_type.size_bytes == 0
    assert ssbo_type.access == ShaderType.Access.READ_WRITE


def test_glsl_storage_buffer_readonly():
    param = compile_parameter("layout(std430) readonly buffer B { float data[]; };")
    ssbo_type = param.type
    assert ssbo_type.access == ShaderType.Access.READ_ONLY


def test_glsl_storage_buffer_writeonly():
    param = compile_parameter("layout(std430) writeonly buffer B { float data[]; };")
    ssbo_type = param.type
    assert ssbo_type.access == ShaderType.Access.WRITE_ONLY


def test_glsl_storage_buffer_contained_type():
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
    contained = param.type.contained_type
    assert isinstance(contained, ShaderType.Struct)
    assert len(contained.members) == 1
    assert contained.members[0].name == "things"
    arr = contained.members[0].type
    assert isinstance(arr, ShaderType.Array)
    assert arr.num_elements == 64
    element = arr.element_type
    assert element.members[0].offset == 0
    assert element.members[1].offset == 12
    assert element.size_bytes == 16
    assert arr.size_bytes == 1024


def test_glsl_storage_buffer_str():
    param = compile_parameter("layout(std430) buffer B { float data[]; };")
    assert "buffer" in str(param.type).lower() or "B" in str(param.type)


def test_glsl_sampler_singleton():
    assert ShaderType.SAMPLER is not None
    assert str(ShaderType.SAMPLER) == "sampler"
