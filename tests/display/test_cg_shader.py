import os

from panda3d import core


SHADERS_DIR = core.Filename.from_os_specific(os.path.dirname(__file__))


def run_cg_compile_check(gsg, shader_path, expect_fail=False):
    """Compile supplied Cg shader path and check for errors"""
    shader = core.Shader.load(shader_path, core.Shader.SL_Cg)
    # assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader is None
    else:
        assert shader is not None


def test_cg_compile_error(gsg):
    """Test getting compile errors from bad Cg shaders"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_bad.sha')
    run_cg_compile_check(gsg, shader_path, expect_fail=True)


def test_cg_from_file(gsg):
    """Test compiling Cg shaders from files"""
    shader_path = core.Filename(SHADERS_DIR, 'cg_simple.sha')
    run_cg_compile_check(gsg, shader_path)
