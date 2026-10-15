from panda3d import core
import pytest


VERTEX = """#version 330
uniform mat4 transform;
in vec4 p3d_Vertex;
out vec2 uv;
void main() {
    gl_Position = transform * p3d_Vertex;
    uv = p3d_Vertex.xy;
}
"""
FRAGMENT = """#version 330
uniform vec4 color;
in vec2 uv;
out vec4 result;
void main() {
    result = color * vec4(uv, 0.0, 1.0);
}
"""
COMPUTE = """#version 430
layout(local_size_x = 2, local_size_y = 3, local_size_z = 1) in;
layout(rgba8) uniform writeonly image2D target;
void main() {
    imageStore(target, ivec2(gl_GlobalInvocationID.xy), vec4(1.0));
}
"""


@pytest.fixture(autouse=True, params=[False, True], ids=['spirv', 'legacy-glsl'])
def shader_pipeline(request):
    page = core.load_prc_file_data(
        'shader-bam-test',
        'glsl-force-legacy-pipeline ' + str(request.param).lower())
    core.ShaderPool.release_all_shaders()
    yield
    core.ShaderPool.release_all_shaders()
    core.unload_prc_file(page)


def check_shader(restored, original, stages):
    assert restored is not None
    assert restored != original
    assert restored.get_language() == original.get_language()
    for stage in core.Shader.Stage:
        module = restored.get_module(stage)
        if stage not in stages:
            assert module is None
            continue
        expected = original.get_module(stage)
        assert module is not None
        assert expected is not None
        assert module != expected
        assert module.get_type() == expected.get_type()
        assert module.stage == expected.stage
        assert module.used_capabilities == expected.used_capabilities
        assert bytes(module.code) == bytes(expected.code)
        for attribute in ('inputs', 'outputs', 'parameters', 'spec_constants'):
            actual_variables = getattr(module, attribute)
            expected_variables = getattr(expected, attribute)
            assert [(v.name, v.type, v.id, getattr(v, 'location', None))
                    for v in actual_variables] == [
                (v.name, v.type, v.id, getattr(v, 'location', None))
                for v in expected_variables
            ]


@pytest.mark.parametrize('compute', [False, True], ids=['graphics', 'compute'])
def test_shader_bam(tmp_path, compute):
    if compute:
        shader = core.Shader.make_compute(core.Shader.SL_GLSL, COMPUTE)
        stages = (core.Shader.Stage.COMPUTE,)
    else:
        shader = core.Shader.make(core.Shader.SL_GLSL,
                                  vertex=VERTEX, fragment=FRAGMENT)
        stages = (core.Shader.Stage.VERTEX, core.Shader.Stage.FRAGMENT)
    assert shader is not None

    filename = core.Filename.from_os_specific(str(tmp_path / 'shader.bam'))
    bam = core.BamFile()
    assert bam.open_write(filename)
    assert bam.write_object(shader)
    bam.close()

    assert bam.open_read(filename)
    restored = bam.read_object()
    assert bam.resolve()
    bam.close()
    check_shader(restored, shader, stages)


def test_shader_bam_without_source(vfs, ramdir, tmp_path):
    vertex_file = core.Filename(ramdir, 'shader.vert')
    fragment_file = core.Filename(ramdir, 'shader.frag')
    assert vfs.write_file(vertex_file, VERTEX.encode('ascii'), False)
    assert vfs.write_file(fragment_file, FRAGMENT.encode('ascii'), False)
    shader = core.Shader.load(core.Shader.SL_GLSL,
                              vertex=vertex_file, fragment=fragment_file)
    assert shader is not None

    filename = core.Filename.from_os_specific(str(tmp_path / 'shader.bam'))
    bam = core.BamFile()
    assert bam.open_write(filename)
    assert bam.write_object(shader)
    bam.close()

    assert vfs.delete_file(vertex_file)
    assert vfs.delete_file(fragment_file)
    core.ShaderPool.release_all_shaders()

    # Restoring the embedded modules must not depend on source files or the
    # shader pool, even when the shader was originally loaded from files.
    assert bam.open_read(filename)
    restored = bam.read_object()
    assert bam.resolve()
    bam.close()
    check_shader(restored, shader,
                 (core.Shader.Stage.VERTEX, core.Shader.Stage.FRAGMENT))


def test_shader_bam_scene(tmp_path):
    shader = core.Shader.make(core.Shader.SL_GLSL,
                              vertex=VERTEX, fragment=FRAGMENT)
    assert shader is not None
    root = core.NodePath('root')
    root.set_shader(shader, 7)
    root.set_shader_input('color', core.Vec4(0.25, 0.5, 0.75, 1), priority=3)
    root.set_shader_input('transform', core.Mat4.translate_mat(1, 2, 3))
    child = root.attach_new_node('child')
    child.set_shader(shader, 2)
    child.set_shader_input('source', root, priority=5)

    filename = core.Filename.from_os_specific(str(tmp_path / 'scene.bam'))
    assert root.write_bam_file(filename)
    bam = core.BamFile()
    assert bam.open_read(filename)
    restored = core.NodePath(bam.read_node())
    bam.close()

    attrib = restored.get_attrib(core.ShaderAttrib)
    assert attrib is not None
    assert attrib.get_shader_priority() == 7
    check_shader(attrib.shader, shader,
                 (core.Shader.Stage.VERTEX, core.Shader.Stage.FRAGMENT))
    assert attrib.has_shader_input('color')
    assert attrib.has_shader_input('transform')
    color = attrib.get_shader_input('color')
    assert color.get_vector() == core.Vec4(0.25, 0.5, 0.75, 1)
    assert color.get_priority() == 3
    matrix = core.Mat4()
    assert attrib.get_shader_input_matrix('transform', matrix) == \
        core.Mat4.translate_mat(1, 2, 3)
    restored_child = restored.find('child')
    child_attrib = restored_child.get_attrib(core.ShaderAttrib)
    assert child_attrib.shader == attrib.shader
    assert child_attrib.get_shader_priority() == 2
    assert child_attrib.has_shader_input('source')
    source = child_attrib.get_shader_input('source')
    assert source.get_priority() == 5
    assert source.get_nodepath() == restored
